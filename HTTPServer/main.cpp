// Built: 24 April 2026 15:20:47
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
    unsigned error_code = http_server.ServerInitialization();
    if (error_code != NO_ERROR_CODE)
    {
        std::cout << "Failed with error code " << error_code << std::endl;
        return 1;
    }

    http_server.RequestHandling();

    return 0;
}