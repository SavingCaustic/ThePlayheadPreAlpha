#pragma once
#include "core/messages/MessageOutQueue.h"
#include <chrono>
#include <crow.h>
#include <iostream>
#include <thread>

class MessageOutReader {
  public:
    explicit MessageOutReader(MessageOutQueue &sender, crow::websocket::connection *wsConn)
        : messageOutQueue(sender), wsConnection(wsConn), stopFlag(false), isRunning(false) {}

    void setConnection(crow::websocket::connection *wsConn) {
        wsConnection = wsConn;
    }

    void start() {
        if (!isRunning.load(std::memory_order_acquire)) {
            stopFlag.store(false, std::memory_order_release);
            consumerThread = std::thread([this] {
                isRunning.store(true, std::memory_order_release);
                consumeMessages();
            });
        }
    }

    void stop() {
        stopFlag.store(true, std::memory_order_release);
        // Notify the condition variable to wake the consumer thread
        messageOutQueue.cv.notify_all();
        if (consumerThread.joinable()) {
            consumerThread.join();
        }
        isRunning.store(false, std::memory_order_release);
    }

    ~MessageOutReader() {
        stop();
    }

  private:
    void consumeMessages() {
        std::cout << "Thread consuming messages started." << std::endl;

        while (!stopFlag.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(messageOutQueue.mtx); // Lock buffer's mutex

            // Wait until either a message is available or stopFlag is set
            messageOutQueue.cv.wait(lock, [this] {
                return messageOutQueue.checkMoreMessages() || stopFlag.load(std::memory_order_acquire);
            });

            if (stopFlag.load(std::memory_order_acquire)) {
                break;
            }

            while (messageOutQueue.checkMoreMessages()) {
                auto message = messageOutQueue.pop();
                if (message && wsConnection) {
                    // Send message if WebSocket is still open
                    std::string msg = "rackId: " + std::to_string(message->rackId) +
                                      ", paramName: " + std::string(message->paramName) +
                                      ", paramValue: " + std::to_string(message->paramValue);
                    wsConnection->send_text(msg);
                }
            }
        }

        std::cout << "Message consumer thread exiting." << std::endl;
    }

    MessageOutQueue &messageOutQueue;
    crow::websocket::connection *wsConnection;
    std::thread consumerThread;
    std::atomic<bool> stopFlag;
    std::atomic<bool> isRunning;
};
