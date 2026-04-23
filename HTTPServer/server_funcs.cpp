#include <string>
#include <iostream>
#include <ws2tcpip.h>
#include <algorithm>
#include <map>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <sstream>
#include "server_funcs.h"
#include "html_templates.h"
#include "html_funcs.h"
#include "main.h"



std::string HTTP_Server::produceHtmlResponse(const uint8_t browser_number, const std::string& os,
    const std::string& device, const std::string& language) {

    // Определяем эмодзи для устройств
    std::string deviceEmoji;
    if (device == "Mobile Phone") deviceEmoji = "📱";
    else if (device == "Tablet") deviceEmoji = "📟";
    else if (device == "Desktop") deviceEmoji = "💻";
    else if (device == "Bot/Crawler") deviceEmoji = "🤖";
    else deviceEmoji = "❓";

    // Определяем цвет в зависимости от браузера
    std::string browserColor;

    if (browser_names[browser_number].find("Chrome") != std::string::npos) browserColor = "#4285F4"; // Chrome blue
    else if (browser_names[browser_number].find("Firefox") != std::string::npos) browserColor = "#FF7139"; // Firefox orange
    else if (browser_names[browser_number].find("Edge") != std::string::npos) browserColor = "#0078D7"; // Edge blue
    else if (browser_names[browser_number].find("Yandex") != std::string::npos) browserColor = "#FF0000"; // Yandex red
    else if (browser_names[browser_number].find("Opera") != std::string::npos) browserColor = "#FF1B2D"; // Opera red
    else browserColor = "#6C757D"; // Default gray

    // Локализация (простые примеры)
    std::string title, welcome, browserText, osText, deviceText, requestText;

    if (language.substr(0, 2) == "ru") {
        title = "Сервер на C++";
        welcome = "Добро пожаловать на наш сервер!";
        browserText = "Браузер:";
        osText = "Операционная система:";
        deviceText = "Тип устройства:";
        requestText = "Номер запроса:";
    }
    else {
        title = "C++ HTTP Server";
        welcome = "Welcome to our C++ HTTP Server!";
        browserText = "Browser:";
        osText = "Operating System:";
        deviceText = "Device Type:";
        requestText = "Request #:";
    }

    // Создаём карту замен
    std::map<std::string, std::string> replacements = {
        {"{lang}", language.substr(0, 2)},
        {"{title}", title},
        {"{welcome}", welcome},
        {"{browser_color}", browserColor},
        {"{browser_label}", browserText},
        {"{browser}", browser_names[browser_number]},
        {"{os_label}", osText},
        {"{os}", os},
        {"{device_label}", deviceText},
        {"{device_emoji}", deviceEmoji},
        {"{device}", device},
        {"{request_label}", requestText},
        {"{request_number}", std::to_string(req_browser_number[browser_number]++)},
        {"{port_number}", std::to_string(port_num)},
        {"{thread_info}", thread_info},
    };

    // Заменяем плейсхолдеры в шаблоне
    return replacePlaceholders(HtmlTemplates::HTML_TEMPLATE, replacements);

}


std::string HTTP_Server::produceHttpResponse(const std::string& path, const std::string& userAgent,
     const std::string& language) {

    if (path == "/") {
        uint8_t browser_number = detectBrowser(userAgent);
        std::string os = detectOS(userAgent);
        std::string device = detectDeviceType(userAgent);

        // Получаем красивый HTML
        std::string htmlContent = produceHtmlResponse(browser_number, os, device, language);

        // Формируем HTTP-ответ с HTML
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: " + std::to_string(htmlContent.length()) + "\r\n"
            "\r\n" +
            htmlContent;

        return response;
    }
    else if (path == "/favicon.ico") {
        // Можно вернуть реальную иконку, но пока 404
        return
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
    }
    else if (path == "/stats") {
        // TODO: Страница статистики
        return
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<h1>Statistics (coming soon)</h1>";
    }
    else {
        // Красивая 404 страница
        std::string html404 = R"(
<!DOCTYPE html>
<html>
<head><title>404 Not Found</title></head>
<body style='text-align: center; padding: 50px;'>
    <h1>Error 404 - Page Not Found</h1>
    <p>The requested URL )" + path + R"( was not found on this server.</p>
    <a href='/'> Back to Home</a>
</body>
</html>
)";

        return
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(html404.length()) + "\r\n"
            "\r\n" +
            html404;
    }
}



int HTTP_Server::WinSockInit()
{
    // Инициализация Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    return 0;
}


int HTTP_Server::SocketCreate()
{
    // Создание сокета
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    return 0;
}


void HTTP_Server::ServerAddrInit()
{
    // Настройка адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Принимать соединения с любых интерфейсов
    serverAddr.sin_port = htons(port_num);         // Порт 80
}


int HTTP_Server::BindingSocket2Addr() const
{
    // Привязка сокета к адресу
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    return 0;
}

int HTTP_Server::StartListenPort() const
{
    // Начинаем прослушивать порт
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << port_num << std::endl;

    return 0;
}


bool HTTP_Server::safeReadClientRequest(const SOCKET clientSocket, int &bytesRead, char* clientIP, const unsigned req_number)
{    
    std::stringstream ss;

    fd_set readfds = {};
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);

    timeval tv = {};
    tv.tv_sec = 0;
    tv.tv_usec = 200000; // 200 мс

    int selectResult = select(0, &readfds, nullptr, nullptr, &tv);

    if (selectResult == 0) {
        // Таймаут - клиент не прислал данные
        ss << "[" << req_number << "] Client timeout [half-open connection]" << std::endl;
        safe_print(ss.str()); 
        return true;
    }

    if (selectResult == SOCKET_ERROR) {
        ss << "[" << req_number << "] Select error: " << WSAGetLastError() << std::endl;
        safe_print(ss.str());
        return true;
    }

    // Читаем запрос
    bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
        ss << "[" << req_number << "] Read error or client closed" << std::endl;
        safe_print(ss.str());
        return true;
    }

    return false;
}




void HTTP_Server::handleClient(SOCKET clientSocket) {

    // Получаем информацию о клиенте
    sockaddr_in clientAddr = {};
    std::stringstream ss;

    int clientAddrSize = sizeof(clientAddr);
    getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrSize);

    char clientIP[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN) == nullptr) {
        strcpy_s(clientIP, "unknown");
    }

    // Генерируем уникальный ID для этого запроса
    static std::atomic<unsigned> globalReqNumber{ 0 };
    unsigned req_number = globalReqNumber++;

    ss << "[" << req_number << "] Thread "
        << std::this_thread::get_id()
        << " handling client: "
        << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
    safe_print(ss.str());


    int bytesRead;
    if (safeReadClientRequest(clientSocket, bytesRead, clientIP, req_number))
    {
        closesocket(clientSocket);
        return;
    }

    buffer[bytesRead] = '\0';
    std::string request(buffer, bytesRead);

    // Проверяем, что запрос не пустой
    if (request.empty()) {
        ss << "[" << req_number << "] Empty request" << std::endl;
        safe_print(ss.str());
        closesocket(clientSocket);
        return;
    }

    // Извлекаем путь из запроса
    std::string path = extractPath(request);
    if (path.empty()) {
        std::string errorResponse = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(clientSocket, errorResponse.c_str(),
            static_cast<int>(errorResponse.length()), 0);
        closesocket(clientSocket);
        return;
    }

    // Извлекаем User-Agent и язык
    std::string userAgent = getUserAgent(request);
    std::string language = getAcceptLanguage(request);

    // Создаем HTML-ответ
    std::string response;
    try {
        response = produceHttpResponse(path, userAgent, language);
    }
    catch (const std::exception& e) {
        ss << "[" << req_number << "] Error: " << e.what() << std::endl;
        safe_print(ss.str());
        response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    }

    // Отправляем ответ
    int bytesSent = send(clientSocket, response.c_str(),
        static_cast<int>(response.length()), 0);

    if (bytesSent == SOCKET_ERROR) {
        ss << "[" << req_number << "] Send failed: " << WSAGetLastError() << std::endl;
        safe_print(ss.str());
    }
    else {
        ss << "[" << req_number << "] Response sent ("
            << bytesSent << " bytes)" << std::endl;
        safe_print(ss.str());
    }

    // Закрываем соединение
    closesocket(clientSocket);
    ss << "[" << req_number << "] Connection closed" << std::endl;
    safe_print(ss.str());
}




void HTTP_Server::RequestHandling()
{
    std :: stringstream ss;
    // Создаём пул из 4 потоков (можно настроить)
    ThreadPool pool(MAX_THREADS);

    if (MAX_THREADS == 1)
        thread_info = "Single-threaded";
    else
        thread_info = "Multi-threaded (" + std::to_string(MAX_THREADS) + ")";

    std::cout << "Thread pool started with " << MAX_THREADS << " workers" << std::endl;
    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;

    // Статистика
    std::atomic<unsigned> totalConnections{ 0 };

    while (true) {
        // Принимаем входящее подключение
        sockaddr_in clientAddr = {};
        int clientAddrSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        totalConnections++;

        // Выводим информацию о подключении
        char clientIP[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN) == nullptr) {
            strcpy_s(clientIP, "unknown");
        }

        ss << "[Main] Accepted connection #" << totalConnections.load()
            << " from " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
        safe_print(ss.str());

        // Передаём обработку в пул потоков
        pool.enqueue([this, clientSocket]() {
            handleClient(clientSocket);
            });

        // Периодически выводим статистику
        if (totalConnections.load() % 10 == 0) {
            ss << "[Main] Total connections: " << totalConnections.load() << std::endl;
            safe_print(ss.str());
        }
    }
}


int HTTP_Server::ServerInitialization()
{

    std::cout << "Starting HTTP Server..." << std::endl;

    if (WinSockInit())
        return WIN_SOCKS_INIT_FAILED;

    if (SocketCreate())
        return SOCKET_CREATE_FAILED;

    ServerAddrInit();

    if (BindingSocket2Addr())
        return BINDING_SOCKET_FAILED;

    if (StartListenPort())
        return START_LISTEN_FAILED;

    return NO_ERROR_CODE;
}




uint8_t HTTP_Server::detectBrowser(const std::string& userAgent) {
    std::string ua = userAgent;
    std::transform(ua.begin(), ua.end(), ua.begin(), ::tolower); // Приводим к нижнему регистру

    if (ua.find("yabrowser") != std::string::npos || ua.find("yandex") != std::string::npos) {
        return NAME_YANDEX; 
    }
    else if (ua.find("opr") != std::string::npos || ua.find("opera") != std::string::npos) {
        return NAME_OPERA;
    }
    else if (ua.find("edg") != std::string::npos || ua.find("edge") != std::string::npos) {
        return NAME_EDGE;
    }
    else if (ua.find("chrome") != std::string::npos) {
        return NAME_CHROME;
    }
    else if (ua.find("firefox") != std::string::npos) {
        return NAME_MOZILLA;
    }
    else if (ua.find("safari") != std::string::npos) {
        return NAME_SAFARI;
    }
    else if (ua.find("trident") != std::string::npos) {
        return NAME_EXPLORER;
    }
    else {
        return NAME_UNKNOWN;
    }
}


std::string HTTP_Server::detectOS(const std::string& userAgent) {
    if (userAgent.find("Windows NT 10.0") != std::string::npos) return "Windows 10/11";
    if (userAgent.find("Windows NT 6.3") != std::string::npos) return "Windows 8.1";
    if (userAgent.find("Mac OS X") != std::string::npos) return "macOS";
    if (userAgent.find("Linux") != std::string::npos) return "Linux";
    if (userAgent.find("Android") != std::string::npos) return "Android";
    if (userAgent.find("iPhone") != std::string::npos) return "iOS";
    return "Unknown OS";
}


std::string HTTP_Server::detectDeviceType(const std::string& userAgent) {
    std::string ua = userAgent;
    std::transform(ua.begin(), ua.end(), ua.begin(), ::tolower);

    // Мобильные телефоны
    if (ua.find("mobile") != std::string::npos ||
        ua.find("android") != std::string::npos ||
        ua.find("iphone") != std::string::npos ||
        ua.find("ipod") != std::string::npos) {

        // Отдельно проверяем планшеты
        if (ua.find("tablet") != std::string::npos ||
            ua.find("ipad") != std::string::npos ||
            (ua.find("android") != std::string::npos && ua.find("mobile") == std::string::npos)) {
            return "Tablet";
        }
        return "Mobile Phone";
    }

    // Планшеты (отдельная проверка)
    if (ua.find("tablet") != std::string::npos ||
        ua.find("ipad") != std::string::npos ||
        ua.find("kindle") != std::string::npos ||
        ua.find("silk") != std::string::npos) {
        return "Tablet";
    }

    // Десктоп по умолчанию
    if (ua.find("windows nt") != std::string::npos ||
        ua.find("macintosh") != std::string::npos ||
        ua.find("linux") != std::string::npos ||
        ua.find("x11") != std::string::npos) {
        return "Desktop";
    }

    // Боты и прочее
    if (ua.find("bot") != std::string::npos ||
        ua.find("crawler") != std::string::npos ||
        ua.find("spider") != std::string::npos) {
        return "Bot/Crawler";
    }

    return "Unknown Device";
}


std::string HTTP_Server::getUserAgent(const std::string& request) {
    // Ищем строку User-Agent:
    size_t pos = request.find("User-Agent:");
    if (pos == std::string::npos) {
        return "";
    }

    // Перемещаемся к началу значения User-Agent
    pos += 11; // Длина "User-Agent:"

    // Пропускаем пробелы
    while (pos < request.length() && (request[pos] == ' ' || request[pos] == '\t')) {
        pos++;
    }

    // Ищем конец строки
    size_t endPos = request.find('\n', pos);
    if (endPos == std::string::npos) {
        endPos = request.length();
    }

    // Извлекаем User-Agent
    return request.substr(pos, endPos - pos);
}


std::string HTTP_Server::extractPath(const std::string& request) {
    // Ищем "GET " в запросе
    size_t getPos = request.find("GET ");
    if (getPos != std::string::npos) {
        size_t spacePos = request.find(' ', getPos + 4);
        if (spacePos != std::string::npos) {
            return request.substr(getPos + 4, spacePos - (getPos + 4));
        }
    }
    return "/"; // По умолчанию
}


std::string HTTP_Server::getAcceptLanguage(const std::string& request) {
    // Ищем строку Accept-Language:
    size_t pos = request.find("Accept-Language:");
    if (pos == std::string::npos) {
        return "en"; // По умолчанию английский
    }

    // Перемещаемся к началу значения
    pos += 16; // Длина "Accept-Language:"

    // Пропускаем пробелы
    while (pos < request.length() && (request[pos] == ' ' || request[pos] == '\t')) {
        pos++;
    }

    // Ищем конец строки
    size_t endPos = request.find('\n', pos);
    if (endPos == std::string::npos) {
        endPos = request.length();
    }

    // Извлекаем язык
    std::string lang = request.substr(pos, endPos - pos);

    // Убираем возможные пробелы в начале/конце
    size_t start = lang.find_first_not_of(" \t");
    size_t end = lang.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos) {
        lang = lang.substr(start, end - start + 1);
    }

    // Берём первый язык из списка (например, "ru,en;q=0.9" → "ru")
    size_t commaPos = lang.find(',');
    if (commaPos != std::string::npos) {
        lang = lang.substr(0, commaPos);
    }

    // Убираем качество (q=...)
    size_t semicolonPos = lang.find(';');
    if (semicolonPos != std::string::npos) {
        lang = lang.substr(0, semicolonPos);
    }

    return lang;
}
