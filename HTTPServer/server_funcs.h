#pragma once

constexpr int port_num = 80;
constexpr unsigned BUF_SIZE = 4096;
constexpr uint8_t BROWSER_TYPES = 8;
//constexpr int timeout = 500; // 0.5 сек
constexpr uint8_t MAX_THREADS = 4;

#define IMITATION_OF_REALITY

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
    char buffer[BUF_SIZE] = {};
    SOCKET serverSocket;
    sockaddr_in serverAddr;
	uint8_t req_number = 0;
	const std::string browser_names[BROWSER_TYPES] = { "Yandex Browser", "Opera", "Microsoft Edge", "Google Chrome",
									 "Mozilla Firefox", "Safari", "Internet Explorer", "Unknown Browser" };
	unsigned req_browser_number[BROWSER_TYPES] {};
    std::string thread_info;


	int SocketCreate();
	int WinSockInit();
	void ServerAddrInit();
	bool safeReadClientRequest(const SOCKET sock, int &bytes_read, char *clientIP, const unsigned req_number);
	int BindingSocket2Addr() const;
	int StartListenPort() const;
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
    void handleClient(SOCKET clientSocket);


public:
	int ServerInitialization();
	void RequestHandling();
};





class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;

public:
    ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; i++) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                            });

                        if (stop && tasks.empty()) return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
                });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};
