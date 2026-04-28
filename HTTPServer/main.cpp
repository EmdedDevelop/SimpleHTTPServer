// Built: 28 April 2026 13:59:37
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include "server_funcs.h"
#include "FileInfo.h"
#include "main.h"

#pragma comment(lib, "ws2_32.lib")


static HTTP_Server http_server;
static std::mutex cout_mutex;



void safe_print(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << message << std::endl;
}



int main() 
{
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    GetExeFileInfo(exePath);

    unsigned error_code = http_server.ServerInitialization();
    if (error_code != NO_ERROR_CODE)
    {
        std::cout << "Failed with error code " << error_code << std::endl;
        return 1;
    }

    http_server.RequestHandling();

    return 0;
}