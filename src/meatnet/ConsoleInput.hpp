#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <queue>

namespace MeatNet {

class ConsoleInput {
public:
    ConsoleInput();
    ~ConsoleInput();

    void Start();
    void Stop();
    bool GetNextLine(std::string& outLine);

private:
    void WorkerThread();

    std::thread* m_thread = nullptr;
    std::mutex m_mutex;
    std::queue<std::string> m_queue;
    bool m_stop = false;
};

} // namespace MeatNet