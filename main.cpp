#include <chrono>
#include <iostream>
#include <thread>

#include <api/peer_connection_interface.h>
#include <api/create_peerconnection_factory.h>
#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
// #include <api/video_codecs/builtin_video_decoder_factory.h>
// #include <api/video_codecs/builtin_video_encoder_factory.h>
#include <api/video_codecs/video_decoder_factory_template.h>
#include <api/video_codecs/video_decoder_factory_template_dav1d_adapter.h>
#include <api/video_codecs/video_decoder_factory_template_libvpx_vp8_adapter.h>
#include <api/video_codecs/video_decoder_factory_template_libvpx_vp9_adapter.h>
#include <api/video_codecs/video_decoder_factory_template_open_h264_adapter.h>
#include <api/video_codecs/video_encoder_factory_template.h>
#include <api/video_codecs/video_encoder_factory_template_libaom_av1_adapter.h>
#include <api/video_codecs/video_encoder_factory_template_libvpx_vp8_adapter.h>
#include <api/video_codecs/video_encoder_factory_template_libvpx_vp9_adapter.h>
#include <api/video_codecs/video_encoder_factory_template_open_h264_adapter.h>

#include "local_signaling.h"
#include "sdp_observer.h"
#include "simple_peer_connection_observer.h"

#include "rtc_base/ssl_adapter.h"

int main(int argc, char *argv[]) {
    // Initialize SSL
    webrtc::InitializeSSL();

    // Create threads
    std::unique_ptr<webrtc::Thread> network_thread = webrtc::Thread::CreateWithSocketServer();
    std::unique_ptr<webrtc::Thread> worker_thread = webrtc::Thread::Create();
    std::unique_ptr<webrtc::Thread> signaling_thread = webrtc::Thread::Create();

    network_thread->Start();
    worker_thread->Start();
    signaling_thread->Start();

    // Create PeerConnection factory
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory =
        webrtc::CreatePeerConnectionFactory(
            network_thread.get(),
            worker_thread.get(),
            signaling_thread.get(),
            nullptr, // audio device module
            webrtc::CreateBuiltinAudioEncoderFactory(),
            webrtc::CreateBuiltinAudioDecoderFactory(),
            // https://issues.webrtc.org/issues/42223784#comment26
            // webrtc::CreateBuiltinVideoEncoderFactory(),
            // webrtc::CreateBuiltinVideoDecoderFactory(),
            std::make_unique<webrtc::VideoEncoderFactoryTemplate<
              webrtc::LibvpxVp8EncoderTemplateAdapter,
              webrtc::LibvpxVp9EncoderTemplateAdapter,
              webrtc::OpenH264EncoderTemplateAdapter,
              webrtc::LibaomAv1EncoderTemplateAdapter>>(),
            std::make_unique<webrtc::VideoDecoderFactoryTemplate<
              webrtc::LibvpxVp8DecoderTemplateAdapter,
              webrtc::LibvpxVp9DecoderTemplateAdapter,
              webrtc::OpenH264DecoderTemplateAdapter,
              webrtc::Dav1dDecoderTemplateAdapter>>(),
            nullptr, // audio mixer
            nullptr  // audio processing
        );

    if (!pc_factory) {
        std::cerr << "Failed to create PeerConnectionFactory!" << std::endl;
        return -1;
    }

    std::cout << "PeerConnectionFactory created successfully" << std::endl;

    // Configure PeerConnections
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    webrtc::PeerConnectionInterface::IceServer stun_server;
    stun_server.uri = "stun:stun.l.google.com:19302";
    config.servers.push_back(stun_server);

    // Create observers
    auto observer1 = std::make_unique<SimplePeerConnectionObserver>("Peer1");
    auto observer2 = std::make_unique<SimplePeerConnectionObserver>("Peer2");

    // Create PeerConnections using CreatePeerConnectionOrError
    webrtc::PeerConnectionDependencies pc1_dependencies(observer1.get());
    webrtc::PeerConnectionDependencies pc2_dependencies(observer2.get());

    auto pc1_result = pc_factory->CreatePeerConnectionOrError(config, std::move(pc1_dependencies));
    if (!pc1_result.ok()) {
        std::cerr << "Failed to create PeerConnection 1: " << pc1_result.error().message() << std::endl;
        return -1;
    }

    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> pc1 = pc1_result.MoveValue();

    auto pc2_result = pc_factory->CreatePeerConnectionOrError(config, std::move(pc2_dependencies));
    if (!pc2_result.ok()) {
        std::cerr << "Failed to create PeerConnection 2: " << pc2_result.error().message() << std::endl;
        return -1;
    }
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> pc2 = pc2_result.MoveValue();

    std::cout << "PeerConnections created successfully" << std::endl;

    // Create data channel on pc1
    webrtc::DataChannelInit dc_config;
    dc_config.ordered = true;

    auto data_channel_result = pc1->CreateDataChannelOrError("hello_channel", &dc_config);
    if (!data_channel_result.ok()) {
        std::cerr << "Failed to create data channel: " << data_channel_result.error().message() << std::endl;
        return -1;
    }

    webrtc::scoped_refptr<webrtc::DataChannelInterface> data_channel = data_channel_result.MoveValue();
    observer1->GetDataObserver()->SetDataChannel(data_channel);
    std::cout << "Data channel created: " << data_channel->label() << std::endl;

    // Create offer
    webrtc::scoped_refptr<CreateSDPObserver> offer_observer = CreateSDPObserver::Create();
    pc1->CreateOffer(offer_observer.get(), webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

    std::cout << "Creating offer..." << std::endl;

    // Wait for offer creation
    int timeout = 50; // 5 seconds
    while (!offer_observer->IsSuccessful() && timeout-- > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!offer_observer->IsSuccessful()) {
        std::cerr << "Failed to create offer!" << std::endl;
        return -1;
    }

    // Get the created offer and clone it immediately
    auto offer = offer_observer->TakeCreatedSDP();
    std::string offer_sdp;
    offer->ToString(&offer_sdp);

    // Create two separate copies of the SDP
    auto offer_for_pc1 = webrtc::CreateSessionDescription(offer->GetType(), offer_sdp);
    auto offer_for_pc2 = webrtc::CreateSessionDescription(offer->GetType(), offer_sdp);

    // Set local description on pc1
    webrtc::scoped_refptr<SetSDPObserver> set_local_observer = SetSDPObserver::Create();
    pc1->SetLocalDescription(set_local_observer.get(), offer_for_pc1.release());

    // Exchange SDPs and create answer
    webrtc::scoped_refptr<CreateSDPObserver> answer_observer = CreateSDPObserver::Create();

    LocalSignaling::ExchangeSDPs(pc1, pc2, std::move(offer_for_pc2), answer_observer);

    std::cout << "Exchanging offer and creating answer..." << std::endl;

    // Wait for answer creation
    timeout = 50;
    while (!answer_observer->IsSuccessful() && timeout-- > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!answer_observer->IsSuccessful()) {
        std::cerr << "Failed to create answer!" << std::endl;
        return -1;
    }

    auto answer = answer_observer->TakeCreatedSDP();
    std::string answer_sdp;
    answer->ToString(&answer_sdp);

    // Create two separate copies of the answer SDP
    auto answer_for_pc2 = webrtc::CreateSessionDescription(answer->GetType(), answer_sdp);
    auto answer_for_pc1 = webrtc::CreateSessionDescription(answer->GetType(), answer_sdp);

    // Set local description on pc2
    webrtc::scoped_refptr<SetSDPObserver> set_local2_observer = SetSDPObserver::Create();
    pc2->SetLocalDescription(set_local2_observer.get(), answer_for_pc2.release());

    // Set remote description (answer) on pc1
    webrtc::scoped_refptr<SetSDPObserver> set_remote_observer = SetSDPObserver::Create();
    pc1->SetRemoteDescription(set_remote_observer.get(), answer_for_pc1.release());

    std::cout << "SDP exchange completed" << std::endl;

    // Wait for ICE gathering and exchange candidates
    std::cout << "Waiting for ICE gathering..." << std::endl;
    timeout = 100; // 10 seconds
    while ((!observer1->IsIceGatheringComplete() || !observer2->IsIceGatheringComplete()) && timeout-- > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Periodically exchange ICE candidates
        if (timeout % 10 == 0) {
            LocalSignaling::ExchangeICECandidates(pc1, pc2, observer1.get(), observer2.get());
        }
    }

    // Final ICE candidate exchange
    LocalSignaling::ExchangeICECandidates(pc1, pc2, observer1.get(), observer2.get());

    std::cout << "Waiting for connection establishment..." << std::endl;

    // Wait for connection establishment
    timeout = 100; // 10 seconds
    while ((!observer1->IsPeerConnected() || !observer2->IsPeerConnected()) && timeout-- > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (observer1->IsPeerConnected() && observer2->IsPeerConnected()) {
        std::cout << "✅ WebRTC connection established successfully!" << std::endl;

        // Wait for data channel messages
        std::cout << "Waiting for data channel messages..." << std::endl;
        timeout = 50; // 5 seconds
        while ((!observer1->HasReceivedMessage() || !observer2->HasReceivedMessage()) && timeout-- > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (observer1->HasReceivedMessage() && observer2->HasReceivedMessage()) {
            std::cout << "✅ Data channel communication successful!" << std::endl;
        } else {
            std::cout << "⚠️  Data channel communication partially successful" << std::endl;
        }

    } else {
        std::cout << "❌ Failed to establish WebRTC connection" << std::endl;
    }

    std::cout << "\nWebRTC Hello World completed!" << std::endl;
    std::cout << "Connection summary:" << std::endl;
    std::cout << "- Peer1 connected: " << (observer1->IsPeerConnected() ? "Yes" : "No") << std::endl;
    std::cout << "- Peer2 connected: " << (observer2->IsPeerConnected() ? "Yes" : "No") << std::endl;
    std::cout << "- Messages exchanged: " << (observer1->HasReceivedMessage() && observer2->HasReceivedMessage() ? "Yes" : "Partial/No") << std::endl;

    // Cleanup
    data_channel = nullptr;
    pc1 = nullptr;
    pc2 = nullptr;
    pc_factory = nullptr;

    webrtc::CleanupSSL();
    return 0;
}
