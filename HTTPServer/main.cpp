#include <string>
#include <iostream>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
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