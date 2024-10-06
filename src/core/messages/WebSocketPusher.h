#pragma once
#include "MessageSender.h"
#include <crow.h>
#include <iostream>
#include <thread>

// this is not the queue. It's the thread reading the queue.

class WebSocketPusher {
  public:
    explicit WebSocketPusher(MessageSender &sender, crow::websocket::connection *wsConn)
        : messageSender(sender), wsConnection(wsConn), stopFlag(false), isRunning(false) {}

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
            consumerThread.join();
        }
        isRunning.store(false, std::memory_order_release);
    }

    ~WebSocketPusher() {
        stop();
    }

  private:
    void consumeMessages() {
        std::cout << "Thread consuming messages started." << std::endl;

        while (!stopFlag.load(std::memory_order_acquire)) {
            messageSender.setSleeping(true);

            auto message = messageSender.pop(); // Blocking wait on the queue

            messageSender.setSleeping(false); // Wake up regardless of message or no message

            if (message && wsConnection) {
                std::string msg = "rackId: " + std::to_string(message->rackId) +
                                  ", paramName: " + std::string(message->paramName) +
                                  ", paramValue: " + std::to_string(message->paramValue);
                wsConnection->send_text(msg);
            }
        }
    }

    MessageSender &messageSender;
    crow::websocket::connection *wsConnection;
    std::thread consumerThread;
    std::atomic<bool> stopFlag;
    std::atomic<bool> isRunning;
};
