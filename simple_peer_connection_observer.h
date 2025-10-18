#pragma once

#include <api/peer_connection_interface.h>

#include "data_channel_observer.h"

class SimplePeerConnectionObserver : public webrtc::PeerConnectionObserver {
public:
    explicit SimplePeerConnectionObserver(const std::string& name)
        : name_(name), data_observer_(std::make_unique<DataChannelObserver>(name)) {}

    // PeerConnectionObserver implementation
    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {
        std::cout << "[" << name_ << "] Signaling state: " << SignalingStateToString(new_state) << std::endl;
    }

    void OnAddTrack(webrtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                    const std::vector<webrtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override {
        std::cout << "[" << name_ << "] Track added" << std::endl;
    }

    void OnRemoveTrack(webrtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {
        std::cout << "[" << name_ << "] Track removed" << std::endl;
    }

    void OnDataChannel(webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {
        std::cout << "[" << name_ << "] Data channel received: " << channel->label() << std::endl;
        data_observer_->SetDataChannel(channel);
    }

    void OnRenegotiationNeeded() override {
        std::cout << "[" << name_ << "] Renegotiation needed" << std::endl;
    }

    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
        std::cout << "[" << name_ << "] ICE connection state: " << IceConnectionStateToString(new_state) << std::endl;
        if (new_state == webrtc::PeerConnectionInterface::kIceConnectionConnected) {
            ice_connected_ = true;
        }
    }

    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {
        std::cout << "[" << name_ << "] ICE gathering state: " << IceGatheringStateToString(new_state) << std::endl;
        if (new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete) {
            ice_gathering_complete_ = true;
        }
    }

    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
        std::string sdp;
        candidate->ToString(&sdp);
        std::cout << "[" << name_ << "] ICE candidate: " << candidate->sdp_mid()
                  << " " << candidate->sdp_mline_index() << std::endl;

        // Store candidate for exchange
        webrtc::SdpParseError error;
        std::unique_ptr<webrtc::IceCandidateInterface> ice_candidate(
            webrtc::CreateIceCandidate(candidate->sdp_mid(),
                                      candidate->sdp_mline_index(),
                                      sdp,
                                      &error)
        );

        if (ice_candidate) {
            ice_candidates_.push_back(std::move(ice_candidate));
        } else {
            std::cerr << "[" << name_ << "] Failed to create ICE candidate: "
                      << error.description << std::endl;
        }
    }

    void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override {
        std::cout << "[" << name_ << "] Connection state: " << ConnectionStateToString(new_state) << std::endl;
        if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kConnected) {
            peer_connected_ = true;
        }
    }

    // Getters for connection state
    bool IsIceConnected() const { return ice_connected_.load(); }
    bool IsIceGatheringComplete() const { return ice_gathering_complete_.load(); }
    bool IsPeerConnected() const { return peer_connected_.load(); }
    bool HasReceivedMessage() const { return data_observer_->HasReceivedMessage(); }

    // Access to ICE candidates
    const std::vector<std::unique_ptr<webrtc::IceCandidateInterface>>& GetIceCandidates() const {
        return ice_candidates_;
    }

    void ClearIceCandidates() { ice_candidates_.clear(); }

    // Access to data channel observer
    DataChannelObserver* GetDataObserver() { return data_observer_.get(); }

private:
    std::string SignalingStateToString(webrtc::PeerConnectionInterface::SignalingState state) {
        switch (state) {
            case webrtc::PeerConnectionInterface::kStable: return "Stable";
            case webrtc::PeerConnectionInterface::kHaveLocalOffer: return "HaveLocalOffer";
            case webrtc::PeerConnectionInterface::kHaveLocalPrAnswer: return "HaveLocalPrAnswer";
            case webrtc::PeerConnectionInterface::kHaveRemoteOffer: return "HaveRemoteOffer";
            case webrtc::PeerConnectionInterface::kHaveRemotePrAnswer: return "HaveRemotePrAnswer";
            case webrtc::PeerConnectionInterface::kClosed: return "Closed";
            default: return "Unknown";
        }
    }

    std::string IceConnectionStateToString(webrtc::PeerConnectionInterface::IceConnectionState state) {
        switch (state) {
            case webrtc::PeerConnectionInterface::kIceConnectionNew: return "New";
            case webrtc::PeerConnectionInterface::kIceConnectionChecking: return "Checking";
            case webrtc::PeerConnectionInterface::kIceConnectionConnected: return "Connected";
            case webrtc::PeerConnectionInterface::kIceConnectionCompleted: return "Completed";
            case webrtc::PeerConnectionInterface::kIceConnectionFailed: return "Failed";
            case webrtc::PeerConnectionInterface::kIceConnectionDisconnected: return "Disconnected";
            case webrtc::PeerConnectionInterface::kIceConnectionClosed: return "Closed";
            default: return "Unknown";
        }
    }

    std::string IceGatheringStateToString(webrtc::PeerConnectionInterface::IceGatheringState state) {
        switch (state) {
            case webrtc::PeerConnectionInterface::kIceGatheringNew: return "New";
            case webrtc::PeerConnectionInterface::kIceGatheringGathering: return "Gathering";
            case webrtc::PeerConnectionInterface::kIceGatheringComplete: return "Complete";
            default: return "Unknown";
        }
    }

    std::string ConnectionStateToString(webrtc::PeerConnectionInterface::PeerConnectionState state) {
        switch (state) {
            case webrtc::PeerConnectionInterface::PeerConnectionState::kNew: return "New";
            case webrtc::PeerConnectionInterface::PeerConnectionState::kConnecting: return "Connecting";
            case webrtc::PeerConnectionInterface::PeerConnectionState::kConnected: return "Connected";
            case webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected: return "Disconnected";
            case webrtc::PeerConnectionInterface::PeerConnectionState::kFailed: return "Failed";
            case webrtc::PeerConnectionInterface::PeerConnectionState::kClosed: return "Closed";
            default: return "Unknown";
        }
    }

    std::string name_;
    std::unique_ptr<DataChannelObserver> data_observer_;
    std::vector<std::unique_ptr<webrtc::IceCandidateInterface>> ice_candidates_;

    std::atomic<bool> ice_connected_{false};
    std::atomic<bool> ice_gathering_complete_{false};
    std::atomic<bool> peer_connected_{false};
};
