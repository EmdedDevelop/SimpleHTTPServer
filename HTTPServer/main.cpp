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

#pragma comment(lib, "ws2_32.lib")


HTTP_Server http_server;


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