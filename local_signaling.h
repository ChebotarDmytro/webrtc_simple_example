#pragma once
#include <thread>
#include <api/peer_connection_interface.h>
#include <api/scoped_refptr.h>

#include "sdp_observer.h"
#include "simple_peer_connection_observer.h"


// Simple local signaling (simulates signaling server)
class LocalSignaling {
public:
    static void ExchangeSDPs(webrtc::scoped_refptr<webrtc::PeerConnectionInterface> pc1,
                             webrtc::scoped_refptr<webrtc::PeerConnectionInterface> pc2,
                             std::unique_ptr<webrtc::SessionDescriptionInterface> offer,
                             webrtc::scoped_refptr<CreateSDPObserver> answer_observer) {

        // Set remote description (offer) on pc2
        webrtc::scoped_refptr<SetSDPObserver> set_observer = SetSDPObserver::Create();
        pc2->SetRemoteDescription(set_observer.get(), offer.release());

        // Wait for set to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Create answer on pc2
        pc2->CreateAnswer(answer_observer.get(), webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }

    static void ExchangeICECandidates(webrtc::scoped_refptr<webrtc::PeerConnectionInterface> pc1,
                                      webrtc::scoped_refptr<webrtc::PeerConnectionInterface> pc2,
                                      SimplePeerConnectionObserver* observer1,
                                      SimplePeerConnectionObserver* observer2) {

        // Exchange ICE candidates from pc1 to pc2
        for (const auto& candidate : observer1->GetIceCandidates()) {
            pc2->AddIceCandidate(candidate.get());
        }

        // Exchange ICE candidates from pc2 to pc1
        for (const auto& candidate : observer2->GetIceCandidates()) {
            pc1->AddIceCandidate(candidate.get());
        }

        observer1->ClearIceCandidates();
        observer2->ClearIceCandidates();
    }
};