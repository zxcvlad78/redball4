#pragma once
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

class MainLoop {
private:
    std::atomic<bool> running{false};

    std::vector<std::function<void(float dt)>> update_callback;
    std::vector<std::function<void(float dt)>> fixed_update_callback;

    std::vector<std::function<void(float dt)>> pending_update;
    std::vector<std::function<void(float dt)>> pending_fixed_update;
    std::mutex update_mutex;

    float target_dt = 1 / 60.f;
    float target_fixed_dt = 1 / 60.f;

    std::thread worker_thread;

    void loop();

public:
    MainLoop() = default;
    ~MainLoop();

    bool is_running() { return running; }

    void listen_update(std::function<void(float dt)> cb);
    void listen_fixed_update(std::function<void(float dt)> cb);

    void start();
    void stop();

    void set_target_dt(float dt);
    void set_target_fixed_dt(float dt);
};

extern MainLoop mainloop;