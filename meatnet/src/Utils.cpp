#include "meatnet/Utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

namespace MeatNet {

static LogCallback g_logCallback = nullptr;
static bool g_bInitialized = false;
static ISteamNetworkingSockets* g_pSockets = nullptr;

static void NukeProcess(int rc) {
#ifdef _WIN32
    ExitProcess(rc);
#else
    (void)rc;
    kill(getpid(), SIGKILL);
#endif
}

void InternalDebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg) {
    LogLevel level = static_cast<LogLevel>(eType);
    if (g_logCallback) {
        g_logCallback(level, pszMsg);
    } else {
        printf("[%s] %s\n",
               eType == k_ESteamNetworkingSocketsDebugOutputType_Bug ? "FATAL" :
               eType == k_ESteamNetworkingSocketsDebugOutputType_Error ? "ERROR" :
               eType == k_ESteamNetworkingSocketsDebugOutputType_Warning ? "WARN" :
               eType == k_ESteamNetworkingSocketsDebugOutputType_Msg ? "INFO" : "DEBUG",
               pszMsg);
        fflush(stdout);
    }

    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
        fflush(stderr);
        NukeProcess(1);
    }
}

bool InitNetwork() {
    if (g_bInitialized)
        return true;

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
        fprintf(stderr, "GameNetworkingSockets_Init failed: %s\n", errMsg);
        return false;
    }
#else
    SteamDatagram_SetAppID(570); // Set app id
    SteamDatagram_SetUniverse(false, k_EUniverseDev);

    SteamDatagramErrMsg errMsg;
    if (!SteamDatagramClient_Init(errMsg)) {
        fprintf(stderr, "SteamDatagramClient_Init failed: %s\n", errMsg);
        return false;
    }

    SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
#endif

    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, InternalDebugOutput);

    g_pSockets = SteamNetworkingSockets();
    g_bInitialized = true;
    return true;
}

void ShutdownNetwork() {
    if (!g_bInitialized)
        return;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
    GameNetworkingSockets_Kill();
#else
    SteamDatagramClient_Kill();
#endif

    g_pSockets = nullptr;
    g_bInitialized = false;
}

void SetLogCallback(LogCallback cb) {
    g_logCallback = cb;
}

ISteamNetworkingSockets* GetSocketsInterface() {
    return g_pSockets;
}

} // namespace MeatNet