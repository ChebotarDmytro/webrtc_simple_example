#pragma once

#include <iostream>
#include <string>
#include <api/data_channel_interface.h>

class DataChannelObserver : public webrtc::DataChannelObserver {
public:
    explicit DataChannelObserver(const std::string& label) : label_(label) {}

    void OnStateChange() override {
        if (data_channel_) {
            webrtc::DataChannelInterface::DataState state = data_channel_->state();
            std::cout << "[" << label_ << "] Data channel state: ";

            switch (state) {
                case webrtc::DataChannelInterface::kConnecting:
                    std::cout << "Connecting" << std::endl;
                    break;
                case webrtc::DataChannelInterface::kOpen:
                    std::cout << "Open" << std::endl;
                    SendHelloMessage();
                    break;
                case webrtc::DataChannelInterface::kClosing:
                    std::cout << "Closing" << std::endl;
                    break;
                case webrtc::DataChannelInterface::kClosed:
                    std::cout << "Closed" << std::endl;
                    break;
            }
        }
    }

    void OnMessage(const webrtc::DataBuffer& buffer) override {
        std::string message(buffer.data.data<char>(), buffer.data.size());
        std::cout << "[" << label_ << "] Received: " << message << std::endl;
        message_received_ = true;
    }

    void SetDataChannel(webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
        data_channel_ = channel;
        if (data_channel_) {
            data_channel_->RegisterObserver(this);
        }
    }

    bool HasReceivedMessage() const { return message_received_.load(); }

private:
    void SendHelloMessage() {
        if (data_channel_ && data_channel_->state() == webrtc::DataChannelInterface::kOpen) {
            std::string msg = "Hello from " + label_ + "!";
            webrtc::DataBuffer buffer(msg);

            if (data_channel_->Send(buffer)) {
                std::cout << "[" << label_ << "] Sent: " << msg << std::endl;
            } else {
                std::cout << "[" << label_ << "] Failed to send message" << std::endl;
            }
        }
    }

    std::string label_;
    webrtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
    std::atomic<bool> message_received_{false};
};
