#pragma once

#include <iostream>
#include <api/peer_connection_interface.h>

class CreateSDPObserver : public webrtc::CreateSessionDescriptionObserver {
public:
    static webrtc::scoped_refptr<CreateSDPObserver> Create() {
        return webrtc::make_ref_counted<CreateSDPObserver>();
    }

    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
        std::cout << "SDP creation successful: " << desc->type() << std::endl;
        created_sdp_.reset(desc);
        success_ = true;
    }

    void OnFailure(webrtc::RTCError error) override {
        std::cout << "SDP creation failed: " << error.message() << std::endl;
        success_ = false;
    }

    std::unique_ptr<webrtc::SessionDescriptionInterface> TakeCreatedSDP() {
        return std::move(created_sdp_);
    }

    bool IsSuccessful() const { return success_; }

protected:
    CreateSDPObserver() = default;
    ~CreateSDPObserver() override = default;

private:
    std::unique_ptr<webrtc::SessionDescriptionInterface> created_sdp_;
    bool success_ = false;
};

class SetSDPObserver : public webrtc::SetSessionDescriptionObserver {
public:
    static webrtc::scoped_refptr<SetSDPObserver> Create() {
        return webrtc::make_ref_counted<SetSDPObserver>();
    }

    void OnSuccess() override {
        std::cout << "SDP set successfully" << std::endl;
        success_ = true;
    }

    void OnFailure(webrtc::RTCError error) override {
        std::cout << "SDP set failed: " << error.message() << std::endl;
        success_ = false;
    }

    bool IsSuccessful() const { return success_; }

protected:
    SetSDPObserver() = default;
    ~SetSDPObserver() override = default;

private:
    bool success_ = false;
};
