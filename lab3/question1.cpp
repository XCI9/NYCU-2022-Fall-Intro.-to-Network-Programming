#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>		//inet_pton()
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <algorithm>		//std::max
#include <string_view>

//read data until receiving data contained specific string
std::string readUntilString(int socketFd, const std::string_view endString, const int estimateLength = 0) {
	static constexpr int bufferSize{ 1000 };
	char buffer[bufferSize+1];
	std::string data{};
	
	//reserve memory for less reallocation
	if(estimateLength > 0)
		data.reserve(estimateLength);

	int readBytesCount{ 0 };
	while ((readBytesCount = read(socketFd, &buffer[0], bufferSize)) != 0) {
		buffer[readBytesCount] = '\0';
		data += buffer;
		
		if (data.find(endString, std::max(static_cast<int>(data.length()) - bufferSize * 2, 0)) != std::string::npos)
			return data;
	}
	fprintf(stderr, "specific string not found! \n");
	return data;
}

int main(int argc, char** argv) {
	//set server header, inp111.zoolab.org:10002 -> 140.113.213.213:10002
	struct sockaddr_in serverInfoHeader {};
    serverInfoHeader.sin_family = AF_INET;
    serverInfoHeader.sin_port = htons(10002);
    inet_pton(AF_INET, "140.113.213.213", &serverInfoHeader.sin_addr);
	
	//connect to server
	int sockfd{ socket(AF_INET, SOCK_STREAM, 0) };
	if (connect(sockfd, (struct sockaddr*)&serverInfoHeader, sizeof(serverInfoHeader)) == 0) [[likely]]
		printf("connect success!\n");
	else 
		fprintf(stderr, "connect failed!, error code : %d", errno);
    
	//get server welcome message
	std::string welcomeInfo{ readUntilString(sockfd, "start.\n", 1000) };
	printf("%s", welcomeInfo.c_str());
	
	//start receiving data
	write(sockfd, "GO\n", strlen("GO\n"));
	std::string data{ readUntilString(sockfd, "received?\n", 300'0000) };
	//printf("%s",data.c_str());
	
	static constexpr auto EndOfDataIndicator{ "\n==== END DATA ====\n" };
	static constexpr auto BeginOfDataIndicator{ "==== BEGIN DATA ====\n" };
	unsigned long long receiveByteCount{ data.rfind(EndOfDataIndicator) - data.find(BeginOfDataIndicator) - strlen(BeginOfDataIndicator) };

	char answer[20]{};
	sprintf(answer, "%llu\n", receiveByteCount);	//convert int to string
	write(sockfd, &answer, strlen(answer));
	
	printf("receive %llu bytes.\n", receiveByteCount);
	//get server check answer
	std::string result(1000, '\0');
	read(sockfd, &result[0], 1000);
	printf("%s", result.c_str());

	return 0;
}
