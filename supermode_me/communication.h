#include <iostream>
#include <windows.h>
#include "vec.h"

namespace comm
{
    struct box
    {
        vec2 xy, size;
        DWORD col;
    };

    HANDLE pipe;
    std::vector<box> boxes;

    void init() {
        LPCSTR pipeName = "\\\\.\\pipe\\SmAnalytics";

        // Create a named pipe
        pipe = CreateNamedPipeA(
            pipeName,                // Pipe name
            PIPE_ACCESS_OUTBOUND,    // Write-only access
            PIPE_TYPE_BYTE,          // Byte-type pipe
            1,                       // Number of instances
            0,                       // Default buffer size
            0,                       // Default buffer size
            0,                       // Default timeout
            NULL);                   // Default security attributes

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to create pipe. Error: " << GetLastError() << std::endl;
            return;
        }

        // Wait for the client to connect
        BOOL connected = false;
        while (!connected)
        {
            std::cout << "waiting for overlay to load..." << std::endl;

            connected = ConnectNamedPipe(pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
            if (!connected) {
                std::cerr << "Failed to connect to pipe. Error: " << GetLastError() << std::endl;
                CloseHandle(pipe);
            }

            Sleep(1000);
        }
    }

    void write_draw_data()
    {
        size_t size = boxes.size();
        DWORD bytesWritten;
        WriteFile(pipe, &size, sizeof(size), &bytesWritten, NULL);
        WriteFile(pipe, boxes.data(), sizeof(box) * size, &bytesWritten, NULL);
    }
}