#include "MainLoop.hpp"
#include <algorithm>
#include <utility>

MainLoop::~MainLoop() {
    if (running.load() && worker_thread.joinable()) {
        stop();
        worker_thread.join();
    }
}

void MainLoop::set_target_dt(float dt) {
    target_dt = dt;
}


void MainLoop::set_target_fixed_dt(float dt) {
    target_fixed_dt = dt;
}

void MainLoop::listen_update(std::function<void(float dt)> cb) {
    std::lock_guard<std::mutex> lock(update_mutex);
    pending_update.push_back(std::move(cb));
}

void MainLoop::listen_fixed_update(std::function<void(float dt)> cb) {
    std::lock_guard<std::mutex> lock(update_mutex);
    pending_fixed_update.push_back(std::move(cb));
}

void MainLoop::start() {
    if (running.exchange(true)) return;

    worker_thread = std::thread(&MainLoop::loop, this);
}

void MainLoop::stop() {
    if (!running.exchange(false)) return;

    if (worker_thread.joinable()) {
        worker_thread.join();
    }

    {
        std::lock_guard<std::mutex> lock(update_mutex);
        pending_update.clear();
        pending_fixed_update.clear();
    }
}

void MainLoop::loop() {
    auto last_time = std::chrono::high_resolution_clock::now();
    float accumulator = 0.0f;

    while (running.load()) {
        const auto now = std::chrono::high_resolution_clock::now();
        const float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        accumulator += dt;

        {
            std::lock_guard<std::mutex> lock(update_mutex);
            if (!pending_update.empty()) {
                update_callback.insert(
                    update_callback.end(),
                    std::make_move_iterator(pending_update.begin()),
                    std::make_move_iterator(pending_update.end())
                );
                pending_update.clear();
            }

            if (!pending_fixed_update.empty()) {
                fixed_update_callback.insert(
                    fixed_update_callback.end(),
                    std::make_move_iterator(pending_fixed_update.begin()),
                    std::make_move_iterator(pending_fixed_update.end())
                );
                pending_fixed_update.clear();
            }
        }

        while (accumulator >= target_fixed_dt) {
            for (auto& cb : fixed_update_callback) {
                cb(target_fixed_dt);
            }
            accumulator -= target_fixed_dt;
        }

        for (auto& cb : update_callback) {
            cb(dt);
        }

        if (target_dt > 0.0f) {
            const float elapsed = dt;
            const float sleep_time = target_dt - elapsed;
            if (sleep_time > 0.0f) {
                std::this_thread::sleep_for(std::chrono::duration<float>(sleep_time));
            }
        }
    }
}