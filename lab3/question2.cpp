#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>

auto _t0{ std::chrono::high_resolution_clock::now() };
static unsigned long long bytesent{ 0 };
void handler(int s) {
	auto _t1{ std::chrono::high_resolution_clock::now() };
	double elapsed{ std::chrono::duration<double>(_t1 - _t0).count() };
	fprintf(stderr, "\n%.6f %llu bytes sent in %.6fs (%.6f Mbps; %.6f MBps)\n",
			std::chrono::duration<double>(_t1.time_since_epoch()).count(),
			bytesent, elapsed, 8.0 * (bytesent / 1000000.0) / elapsed, (bytesent / 1000000.0) / elapsed);
	exit(0);
}

int main(int argc, char** argv) {
	using namespace std::chrono_literals;

	if (argc != 3)
		fprintf(stderr, "use ./client <speed> <send interval>");

	//static constexpr int const sendIntervalMiliisecond{ 100 };
	const auto sendIntervalMilisecond{ std::stol(argv[2]) };
	const double throughput{ std::stod(argv[1]) };

	signal(SIGINT,  handler);
	signal(SIGTERM, handler);
	
	struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10003);
    //inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);	//140.113.213.213
	inet_pton(AF_INET, "140.113.213.213", &servaddr.sin_addr);

	int sockfd{ socket(AF_INET, SOCK_STREAM, 0) };
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("connect success!\n");
	
	std::string header(1000, '\0');
	if(read(sockfd, &header[0], 1000) == 0)
		fprintf(stderr, "can't get info from server.");
	
	//100KB of char
	static constexpr const int headerLength{ 14 + 20 + 32 }; //eth + ip + tcp
	const int packetCount{ static_cast<int>(std::ceil(throughput * sendIntervalMilisecond * 1'000 / (65535 - headerLength)))};
	std::string str(static_cast<int>(throughput * sendIntervalMilisecond * 1'000 - packetCount * headerLength), 'x');
	printf("send %d bytes data, with header count estimate: %d\n",
		   static_cast<int>(throughput * sendIntervalMilisecond * 1'000 - packetCount * headerLength), packetCount);
	
	auto startTime{ std::chrono::high_resolution_clock::now() };
	int loopTime{ 1 };
	while(1) {
		auto successSend{ write(sockfd, &str[0], str.size()) };
		bytesent += successSend;

		auto currentTime{ std::chrono::high_resolution_clock::now() };
		std::this_thread::sleep_for(1ms * sendIntervalMilisecond * loopTime++ - (currentTime - startTime));
	}

	return 0;
}
