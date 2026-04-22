#include <string>
#include <iostream>
#include <ws2tcpip.h>
#include <algorithm>
#include <map>
#include "server_funcs.h"
#include "html_templates.h"
#include "html_funcs.h"



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
        {"{request_number}", std::to_string(req_browser_number[browser_number]++)}
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



bool HTTP_Server:: hasDataAvailable(SOCKET sock, int timeout_ms) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    // select возвращает:
    // >0: данные доступны
    // 0: таймаут (нет данных)
    // <0: ошибка
    int result = select(0, &readfds, nullptr, nullptr, &tv);

    if (result > 0 && FD_ISSET(sock, &readfds)) {
        return true;  // Данные есть, можно читать
    }
    return false;  // Нет данных или ошибка
}



int HTTP_Server::ReadClientRequest(const SOCKET clientSocket)
{
	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    std::cout << "DEBUG: recv returned " << bytesReceived << " bytes" << std::endl;

	if (bytesReceived > 0) {
		buffer[bytesReceived] = '\0';
		std::cout << "DEBUG: bytesReceived: "
			<< std::string(buffer, bytesReceived) << std::endl;
	}

	return bytesReceived;
}


bool HTTP_Server::safeReadClientRequest(const SOCKET clientSocket, int &bytesRead, char* clientIP)
{    
    if (hasDataAvailable(clientSocket, 200)) {
        bytesRead = ReadClientRequest(clientSocket);
        if (bytesRead <= 0) {
            std::cerr << "[" << req_number << "] Failed to read request from "
                << clientIP << " (bytes: " << bytesRead << ")" << std::endl;
            return true;
        }
    }
    else {
        // Клиент "молчит" - закрываем соединение
        std::cerr << "[" << req_number << "] Client sent no data (half-open connection) " << std::endl;
        return true;
    }

    return false;
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

        // Выводим информацию о подключении
        char clientIP[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN) == nullptr) {
            std::cerr << "Failed to convert IP address" << std::endl;
            strcpy_s(clientIP, "unknown");
        }

        std::cout << "[" << req_number << "] Client connected from: "
            << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // Установи таймаут на чтение (2 секунд)
//        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,
 //           (char*)&timeout, sizeof(timeout));

        int bytesRead;
        if (safeReadClientRequest(clientSocket, bytesRead, clientIP))
        {
            closesocket(clientSocket);
            continue;
        }

        // Преобразуем в строку
        std::string request(buffer, bytesRead);

        // Проверяем, что запрос не пустой
        if (request.empty()) {
            std::cerr << "[" << req_number << "] Empty request from " << clientIP << std::endl;
            closesocket(clientSocket);
            continue;
        }

        // Извлекаем путь из запроса
        std::string path = extractPath(request);
        if (path.empty()) {
            std::cerr << "[" << req_number << "] Invalid request path from " << clientIP << std::endl;
            // Можно отправить 400 Bad Request
            std::string errorResponse = "HTTP/1.1 400 Bad Request\r\n\r\n";
            send(clientSocket, errorResponse.c_str(), static_cast<int>(errorResponse.length()), 0);
            closesocket(clientSocket);
            continue;
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
            std::cerr << "[" << req_number << "] Error generating response for "
                << clientIP << ": " << e.what() << std::endl;
            response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        }

        // Отправляем ответ
        int bytesSent = send(clientSocket, response.c_str(),
            static_cast<int>(response.length()), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "[" << req_number << "] Send failed to "
                << clientIP << ": " << WSAGetLastError() << std::endl;
        }
        else if (bytesSent < static_cast<int>(response.length())) {
            std::cerr << "[" << req_number << "] Partial send to "
                << clientIP << ": " << bytesSent << "/"
                << response.length() << " bytes" << std::endl;
        }
        else {
            std::cout << "[" << req_number << "] Response sent to "
                << clientIP << " (" << bytesSent << " bytes)" << std::endl;
        }

        // Закрываем соединение с клиентом
        closesocket(clientSocket);

        // Увеличиваем номер запроса только если всё успешно
        req_number++;

        std::cout << "[" << req_number - 1 << "] Connection closed with " << clientIP << std::endl;
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
