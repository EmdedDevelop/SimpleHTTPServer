#include <string>
#include <iostream>
#include <ws2tcpip.h>
#include <algorithm>
#include <map>
#include "server_funcs.h"
#include "html_templates.h"
#include "html_funcs.h"



std::string HTTP_Server::produceHtmlResponse(const std::string& browser, const std::string& os,
    const std::string& device, unsigned req_number,
    const std::string& language) {

    // Определяем эмодзи для устройств
    std::string deviceEmoji;
    if (device == "Mobile Phone") deviceEmoji = "📱";
    else if (device == "Tablet") deviceEmoji = "📟";
    else if (device == "Desktop") deviceEmoji = "💻";
    else if (device == "Bot/Crawler") deviceEmoji = "🤖";
    else deviceEmoji = "❓";

    // Определяем цвет в зависимости от браузера
    std::string browserColor;
    if (browser.find("Chrome") != std::string::npos) browserColor = "#4285F4"; // Chrome blue
    else if (browser.find("Firefox") != std::string::npos) browserColor = "#FF7139"; // Firefox orange
    else if (browser.find("Edge") != std::string::npos) browserColor = "#0078D7"; // Edge blue
    else if (browser.find("Yandex") != std::string::npos) browserColor = "#FF0000"; // Yandex red
    else if (browser.find("Opera") != std::string::npos) browserColor = "#FF1B2D"; // Opera red
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
        {"{browser}", browser},
        {"{os_label}", osText},
        {"{os}", os},
        {"{device_label}", deviceText},
        {"{device_emoji}", deviceEmoji},
        {"{device}", device},
        {"{request_label}", requestText},
        {"{request_number}", std::to_string(req_number)}
    };

    // Заменяем плейсхолдеры в шаблоне
    return replacePlaceholders(HtmlTemplates::HTML_TEMPLATE, replacements);

}


std::string HTTP_Server::produceHttpResponse(const std::string& path, const std::string& userAgent,
    unsigned req_number, const std::string& language) {

    if (path == "/") {
        std::string browser = detectBrowser(userAgent);
        std::string os = detectOS(userAgent);
        std::string device = detectDeviceType(userAgent);

        // Получаем красивый HTML
        std::string htmlContent = produceHtmlResponse(browser, os, device, req_number, language);

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


int HTTP_Server::BindingSocket2Addr()
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

int HTTP_Server::StartListenPort()
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


void HTTP_Server::ReadClientRequest(SOCKET &clientSocket)
{
    // Чтение запроса от клиента
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Добавляем нуль-терминатор
        std::cout << "Received request:\n" << buffer << std::endl;
    }
}


void HTTP_Server::RequestHandling()
{
    unsigned req_number = 0;

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

        ReadClientRequest(clientSocket);
        std::string request(buffer);

        // Извлекаем путь из запроса
        std::string path = extractPath(request);

        // Извлекаем User-Agent и язык
        std::string userAgent = getUserAgent(request);
        std::string language = getAcceptLanguage(request);

        // Создаем HTML-ответ
        std::string response = produceHttpResponse(path, userAgent, req_number++, language);

        // Отправляем ответ
        send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);

        // Закрываем соединение с клиентом
        closesocket(clientSocket);
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




std::string HTTP_Server::detectBrowser(const std::string& userAgent) {
    std::string ua = userAgent;
    std::transform(ua.begin(), ua.end(), ua.begin(), ::tolower); // Приводим к нижнему регистру

    if (ua.find("yabrowser") != std::string::npos || ua.find("yandex") != std::string::npos) {
        return "Yandex Browser";
    }
    else if (ua.find("opr") != std::string::npos || ua.find("opera") != std::string::npos) {
        return "Opera";
    }
    else if (ua.find("edg") != std::string::npos || ua.find("edge") != std::string::npos) {
        return "Microsoft Edge";
    }
    else if (ua.find("chrome") != std::string::npos) {
        return "Google Chrome";
    }
    else if (ua.find("firefox") != std::string::npos) {
        return "Mozilla Firefox";
    }
    else if (ua.find("safari") != std::string::npos) {
        return "Safari";
    }
    else if (ua.find("trident") != std::string::npos) {
        return "Internet Explorer";
    }
    else {
        return "Unknown Browser";
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


