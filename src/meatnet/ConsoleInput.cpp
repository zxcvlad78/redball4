#include "ConsoleInput.hpp"
#include <cstdio>
#include <cctype>
#include <algorithm>

namespace MeatNet {

static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

ConsoleInput::ConsoleInput() = default;
ConsoleInput::~ConsoleInput() { Stop(); }

void ConsoleInput::Start() {
    if (m_thread)
        return;
    m_stop = false;
    m_thread = new std::thread(&ConsoleInput::WorkerThread, this);
}

void ConsoleInput::Stop() {
    m_stop = true;
    if (m_thread) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_queue.empty())
        m_queue.pop();
}

bool ConsoleInput::GetNextLine(std::string& outLine) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        return false;
    outLine = m_queue.front();
    m_queue.pop();
    ltrim(outLine);
    rtrim(outLine);
    return !outLine.empty();
}

void ConsoleInput::WorkerThread() {
    while (!m_stop) {
        char buffer[4096];
        if (fgets(buffer, sizeof(buffer), stdin)) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n')
                line.pop_back();
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(line);
        } else {
            break;
        }
    }
}

} // namespace MeatNet