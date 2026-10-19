#include "pocketqube_fsm.h"
#include "config.h"

namespace cavsat {

PocketQubeOutputs PocketQubeFsm::update(uint32_t sincePowerMs) {
    PocketQubeOutputs out;
    if (!p_.panelsFired && sincePowerMs >= PANEL_DEPLOY_DELAY_MS) {
        out.firePanels = true;
        p_.panelsFired = true;
    }
    return out;
}

bool PocketQubeFsm::requestBoom() {
    if (p_.boomFired) return false;
    p_.boomFired = true;
    return true;
}

} // namespace cavsat
