#pragma once

#include <iostream>
#include <windows.h>
#include <vector>
#include "vec.h"

namespace comm
{
    struct box
    {
        vec2 xy, size;
        DWORD col;
    };

    HANDLE pipe;
    std::vector<box> last_boxes;

    void init() {
        LPCSTR pipeName = "\\\\.\\pipe\\SmAnalytics";

        // Wait for the pipe to be available
        while (!WaitNamedPipeA(pipeName, NMPWAIT_WAIT_FOREVER)) {
            std::cout << "Waiting for server..." << std::endl;
            Sleep(1000);
        }

        // Connect to the named pipe
        pipe = CreateFileA(
            pipeName,                // Pipe name
            GENERIC_READ,            // Read-only access
            0,                       // No sharing
            NULL,                    // Default security attributes
            OPEN_EXISTING,           // Opens existing pipe
            0,                       // Default attributes
            NULL);                   // No template file

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to connect to pipe. Error: " << GetLastError() << std::endl;
            return;
        }



    }

    std::vector<box> read_draw_data()
    {
        size_t size;
        DWORD bytesRead;
        ReadFile(pipe, &size, sizeof(size), &bytesRead, NULL);

        if (size == 0) {
            return last_boxes;
        }

        std::vector<box> vecs(size);
        ReadFile(pipe, vecs.data(), sizeof(box) * size, &bytesRead, NULL);

        last_boxes = vecs;
        return vecs;
    }
}