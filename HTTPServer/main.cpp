#include <string>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")



std::string getHttpResponse(const std::string& path, unsigned req_number) {
    if (path == "/") {
        std::string response = 
            "HTTP/1.1 200 OK\r\n" 
            "Content-Type: text/plain\r\n" 
            "Content-Length: " + std::to_string(15 + std::to_string(req_number).length()) + " \r\n"
            "\r\n" 
            "Hello, World! #" + std::to_string(req_number) + "\r\n";
        return response;
            
    }
    else if (path == "/favicon.ico") {
        // Возвращаем 404 для favicon.ico, чтобы браузер не пытался снова
        return
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
    }
    else {
        // Для любых других путей
        return
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
    }
}



int main() {
    unsigned req_number = 1;

    std::cout << "Starting HTTP Server..." << std::endl;

    // TODO: Здесь будет код сервера

    // Инициализация Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Создание сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Принимать соединения с любых интерфейсов
    serverAddr.sin_port = htons(8080);         // Порт 8080

    // Привязка сокета к адресу
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Начинаем прослушивать порт
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;


    while (true) {
        // Принимаем входящее подключение
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        // Выводим информацию о подключении (опционально)
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "Client connected from: " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // Чтение запроса от клиента
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';  // Добавляем нуль-терминатор
            std::cout << "Received request:\n" << buffer << std::endl;
        }



        // Извлекаем путь из запроса
        std::string request(buffer);
        std::string path = "/";

        // Ищем "GET " в запросе
        size_t getPos = request.find("GET ");
        if (getPos != std::string::npos) {
            size_t spacePos = request.find(' ', getPos + 4);
            if (spacePos != std::string::npos) {
                path = request.substr(getPos + 4, spacePos - (getPos + 4));
            }
        }

        std::cout << "Requested path: " << path << std::endl;

        // Получаем соответствующий ответ
        std::string response = getHttpResponse(path, req_number);
        req_number++;

        // Отправляем ответ
        int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        }

        // Закрываем соединение с клиентом
        closesocket(clientSocket);
    }



    return 0;
}