#include <Windows.h>
#include "communication.h"
#include "overlay.h"

void main_thread()
{
    comm::init();
    Sleep(1000);
    HANDLE h = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)overlay::main_thread, FindWindow(NULL, "Counter-Strike 2"), 0, 0);
    if (h)
        CloseHandle(h);

    return;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE h = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)main_thread, NULL, NULL, NULL);
        if (h)
            CloseHandle(h);
    }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

