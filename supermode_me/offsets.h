/*
 * Created using https://github.com/a2x/cs2-dumper
 * Sat, 18 Nov 2023 01:51:41 +0000
 */

#pragma once

#include <cstddef>

namespace client_dll { // client.dll
    constexpr std::ptrdiff_t dwEntityList = 0x17B5200;
    constexpr std::ptrdiff_t dwForceAttack = 0x16B5510;
    constexpr std::ptrdiff_t dwForceAttack2 = 0x16B55A0;
    constexpr std::ptrdiff_t dwForceBackward = 0x16B57E0;
    constexpr std::ptrdiff_t dwForceCrouch = 0x16B5AB0;
    constexpr std::ptrdiff_t dwForceForward = 0x16B5750;
    constexpr std::ptrdiff_t dwForceJump = 0x16B5A20;
    constexpr std::ptrdiff_t dwForceLeft = 0x16B5870;
    constexpr std::ptrdiff_t dwForceRight = 0x16B5900;
    constexpr std::ptrdiff_t dwGameEntitySystem = 0x18E08E0;
    constexpr std::ptrdiff_t dwGameEntitySystem_getHighestEntityIndex = 0x1510;
    constexpr std::ptrdiff_t dwGameRules = 0x1810EB0;
    constexpr std::ptrdiff_t dwGlobalVars = 0x16B1670;
    constexpr std::ptrdiff_t dwGlowManager = 0x1810ED8;
    constexpr std::ptrdiff_t dwInterfaceLinkList = 0x190E578;
    constexpr std::ptrdiff_t dwLocalPlayerController = 0x1804518;
    constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x16BC5B8;
    constexpr std::ptrdiff_t dwPlantedC4 = 0x1818478;
    constexpr std::ptrdiff_t dwPrediction = 0x16BC480;
    constexpr std::ptrdiff_t dwSensitivity = 0x1812468;
    constexpr std::ptrdiff_t dwSensitivity_sensitivity = 0x40;
    constexpr std::ptrdiff_t dwViewAngles = 0x18744C0;
    constexpr std::ptrdiff_t dwViewMatrix = 0x1813840;
    constexpr std::ptrdiff_t dwViewRender = 0x18140C0;
}

namespace engine2_dll { // engine2.dll
    constexpr std::ptrdiff_t dwBuildNumber = 0x48B514;
    constexpr std::ptrdiff_t dwNetworkGameClient = 0x48AAC0;
    constexpr std::ptrdiff_t dwNetworkGameClient_getLocalPlayer = 0xF0;
    constexpr std::ptrdiff_t dwNetworkGameClient_maxClients = 0x250;
    constexpr std::ptrdiff_t dwNetworkGameClient_signOnState = 0x240;
    constexpr std::ptrdiff_t dwWindowHeight = 0x541D8C;
    constexpr std::ptrdiff_t dwWindowWidth = 0x541D88;
}

namespace inputsystem_dll { // inputsystem.dll
    constexpr std::ptrdiff_t dwInputSystem = 0x35760;
}

namespace player {
    constexpr std::ptrdiff_t m_iHealth = 0x32C;
    constexpr std::ptrdiff_t m_vOldOrigin = 0x1224;
    constexpr std::ptrdiff_t m_hPlayerPawn = 0x7EC;

    constexpr std::ptrdiff_t m_pGameSceneNode = 0x310;
    constexpr std::ptrdiff_t m_modelState = 0x160;
    constexpr std::ptrdiff_t m_vecOrigin = 0x80;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3BF;
}