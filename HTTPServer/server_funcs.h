#pragma once

constexpr int port_num = 80;
constexpr unsigned BUF_SIZE = 4096;

enum ERROR_CODES {
	NO_ERROR_CODE = 0,
	WIN_SOCKS_INIT_FAILED,
	SOCKET_CREATE_FAILED,
	BINDING_SOCKET_FAILED,
	START_LISTEN_FAILED
};



class HTTP_Server
{
	char buffer[BUF_SIZE];
	SOCKET serverSocket;
	sockaddr_in serverAddr;
	int bytesReceived;


	int SocketCreate();
	int WinSockInit();
	void ServerAddrInit();
	void ReadClientRequest(SOCKET& clientSocket);
	int BindingSocket2Addr();
	int StartListenPort();
	std::string detectBrowser(const std::string& userAgent);
	std::string produceHtmlResponse(const std::string& browser, const std::string& os,
		const std::string& device, unsigned req_number,
		const std::string& language);
	std::string produceHttpResponse(const std::string& path, const std::string& userAgent,
		unsigned req_number, const std::string& language);
	std::string detectOS(const std::string& userAgent);
	std::string detectDeviceType(const std::string& userAgent);
	std::string getUserAgent(const std::string& request);
	std::string extractPath(const std::string& request);
	std::string getAcceptLanguage(const std::string& request);


public:
	int ServerInitialization();
	void RequestHandling();
};