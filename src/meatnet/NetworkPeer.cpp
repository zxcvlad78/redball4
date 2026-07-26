#include "NetworkPeer.hpp"

namespace MeatNet {

void NetworkPeer::PollCallbacks() {
    if (m_pInterface) {
        m_pInterface->RunCallbacks();
    }
}

void NetworkPeer::Log(LogLevel level, const char* msg) {
    if (m_logCallback) {
        m_logCallback(level, msg);
    } else {
        printf("[Peer] %s\n", msg);
    }
}

} // namespace MeatNet