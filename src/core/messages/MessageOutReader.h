#pragma once
#include "MessageOutBuffer.h"
#include <chrono>
#include <crow.h>
#include <iostream>
#include <thread>

class MessageOutReader {
  public:
    explicit MessageOutReader(MessageOutBuffer &sender, crow::websocket::connection *wsConn, std::condition_variable &cv)
        : messageOutBuffer(sender), wsConnection(wsConn), stopFlag(false), isRunning(false), sharedCV(cv) {}

    // Set the WebSocket connection (for use in onopen and onclose)
    void setConnection(crow::websocket::connection *wsConn) {
        wsConnection = wsConn;
    }

    // Start the consumer thread if not already running
    void start() {
        if (!isRunning.load(std::memory_order_acquire)) {
            stopFlag.store(false, std::memory_order_release);
            consumerThread = std::thread([this] {
                isRunning.store(true, std::memory_order_release);
                consumeMessages();
            });
        }
    }

    // Stop the consumer thread and join it
    void stop() {
        stopFlag.store(true, std::memory_order_release);
        if (consumerThread.joinable()) {
            sharedCV.notify_one(); // Wake up the thread if it's sleeping
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
            std::unique_lock<std::mutex> lock(sharedMutex);
            // Wait until notified by the MessageOutBuffer
            sharedCV.wait(lock, [this] { return messageOutBuffer.checkMoreMessages() || stopFlag.load(std::memory_order_acquire); });

            // If the stop flag is set, break out of the loop
            if (stopFlag.load(std::memory_order_acquire)) {
                break;
            }

            while (messageOutBuffer.checkMoreMessages()) {
                auto message = messageOutBuffer.pop(); // Non-blocking pop
                if (message && wsConnection) {
                    // If we have a message, send it over the WebSocket
                    std::string msg = "rackId: " + std::to_string(message->rackId) +
                                      ", paramName: " + std::string(message->paramName) +
                                      ", paramValue: " + std::to_string(message->paramValue);
                    wsConnection->send_text(msg);
                }
            }
        }

        std::cout << "Message consumer thread exiting." << std::endl;
    }

    MessageOutBuffer &messageOutBuffer;
    crow::websocket::connection *wsConnection;
    std::thread consumerThread;
    std::atomic<bool> stopFlag;
    std::atomic<bool> isRunning;
    std::mutex sharedMutex;            // Protects access to wsConnection
    std::condition_variable &sharedCV; // Reference to shared condition variable
};
