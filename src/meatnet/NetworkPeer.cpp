#include "NetworkPeer.hpp"

namespace MeatNet {

void NetworkPeer::PollCallbacks() {
    if (m_pInterface) {
        m_pInterface->RunCallbacks();
    }
}

} //namespace MeatNet