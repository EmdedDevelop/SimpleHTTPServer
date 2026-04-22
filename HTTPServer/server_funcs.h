#pragma once

constexpr int port_num = 80;
constexpr unsigned BUF_SIZE = 4096;
constexpr uint8_t BROWSER_TYPES = 8;

enum ERROR_CODES {
	NO_ERROR_CODE = 0,
	WIN_SOCKS_INIT_FAILED,
	SOCKET_CREATE_FAILED,
	BINDING_SOCKET_FAILED,
	START_LISTEN_FAILED
};

enum BROWSER_NAMES {
	NAME_YANDEX,
	NAME_OPERA,
	NAME_EDGE,
	NAME_CHROME,
	NAME_MOZILLA,
	NAME_SAFARI,
	NAME_EXPLORER,
	NAME_UNKNOWN
};


class HTTP_Server
{
	char buffer[BUF_SIZE];
	SOCKET serverSocket;
	sockaddr_in serverAddr;
	uint8_t req_number;
	const std::string browser_names[BROWSER_TYPES] = { "Yandex Browser", "Opera", "Microsoft Edge", "Google Chrome",
									 "Mozilla Firefox", "Safari", "Internet Explorer", "Unknown Browser" };
	unsigned req_browser_number[BROWSER_TYPES] {};


	int SocketCreate();
	int WinSockInit();
	void ServerAddrInit();
	int ReadClientRequest(SOCKET& clientSocket);
	int BindingSocket2Addr();
	int StartListenPort();
	uint8_t detectBrowser(const std::string& userAgent);
	std::string produceHtmlResponse(const uint8_t browser_number, const std::string& os,
		const std::string& device, const std::string& language);
	std::string produceHttpResponse(const std::string& path, const std::string& userAgent,
		const std::string& language);
	std::string detectOS(const std::string& userAgent);
	std::string detectDeviceType(const std::string& userAgent);
	std::string getUserAgent(const std::string& request);
	std::string extractPath(const std::string& request);
	std::string getAcceptLanguage(const std::string& request);


public:
	int ServerInitialization();
	void RequestHandling();
};