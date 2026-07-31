#ifdef _WIN32
#include "winsock.h"
#include <Windows.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS

#elif defined(__linux__)
#include <sys/socket.h>   // 提供 socket(), bind() 等函数
#include <netinet/in.h>   // 提供 sockaddr_in 结构体
#include <arpa/inet.h>    // 提供 inet_pton(), inet_ntoa() 等函数 (可选)
#include <stdio.h>
#include <sys/socket.h>    // msghdr 定义在此
#include <sys/uio.h>       // struct iovec（msghdr 成员依赖）
#include <fcntl.h>      // open, O_* 常量
#include <unistd.h>     // write, close
#include <errno.h>      // errno
#include <string.h>     // strerror
#include <stdlib.h>   // 必须包含
#include <pthread.h>
#else
#error "Unknown platform"
#endif


int TcpClient(char* sip, int sport,char * cip,int cport) {
	printf("%s %d sip:%s sport:%d cip:%s cport:%d\r\n", __FUNCTION__, __LINE__,sip,sport,cip,cport);

	int ret = 0;

	int s = socket(2, 1, 0);
	if (s == -1) {
		perror("socket\r\n");
		return -1;
	}

	if (cport || cip) {
#ifdef __linux__
		sockaddr_in ca = { 0 };
		ca.sin_family = AF_INET;
		if(cip)
			ca.sin_addr.s_addr = inet_addr(cip);
		else
			ca.sin_addr.s_addr = 0;
		ca.sin_port = ntohs(cport);
		ret = bind(s, (sockaddr*)&ca, sizeof(sockaddr_in));
#elif defined(_WIN32)
		sockaddr_in ca = { 0 };
		ca.sin_family = AF_INET;
		if (cip)
			ca.sin_addr.S_un.S_addr = inet_addr(cip);
		else
			ca.sin_addr.S_un.S_addr = 0;
		ca.sin_port = ntohs(cport);
		ret = bind(s, (sockaddr*)&ca, sizeof(sockaddr_in));
#else

#endif
		if (ret) {
			perror("bind");
			return -1;
		}
	}

#ifdef __linux__
	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(sip);
	sa.sin_port = ntohs(sport);
#elif defined(_WIN32)
	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = inet_addr(sip);
	sa.sin_port = ntohs(sport);
#else

#endif

	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret < 0) {
		perror("connect\r\n");
		return -1;
	}
	int bufsize = 0x10000;
	char* sendbuf = (char*) malloc(bufsize);

	char* recvbuf = (char*)malloc(bufsize);
	if(recvbuf == 0 || sendbuf == 0) {
#ifdef _WIN32
		closesocket(s);
#elif defined(__linux__)
		close(s);
#else

#endif
		printf("malloc error\r\n");
		return -1;
	}

	strcpy(sendbuf, "hello server!I am client!\r\n");
	int sendlen = send(s, sendbuf, strlen(sendbuf), 0);

	int recvLen = 0;
	recvLen = recv(s, recvbuf, bufsize - 1, 0);
	if (recvLen > 0 && recvLen < bufsize) {
		recvbuf[recvLen] = 0;
		printf("recv len:%d,data:%s\r\n", recvLen, recvbuf);
	}



#ifdef _WIN32
	closesocket(s);
#elif defined(__linux__)
	close(s);
#else

#endif

	free(sendbuf);

	free(recvbuf);

	return 0;
}




int main(int argc, char** argv) {

	int ret = 0;
	int seq = 0;

#ifdef _WIN32
	WSADATA wsa = { 0 };
	ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		return -1;
	}
#endif

	char* sip = 0;
	int sport = 0;
	int cport = 0;
	char* cip = 0;
	for (int seq = 1; seq < argc; seq++) {
		if (strcmp(argv[seq], "--s") == 0) {
			sip = argv[seq + 1];
			seq++;

		}
		else if (strcmp(argv[seq], "--p") == 0) {
			sport = atoi(argv[seq + 1]);
			seq++;
		}
		else if (strcmp(argv[seq], "-p") == 0) {
			cport = atoi(argv[seq + 1]);
			seq++;
		}
		else if (strcmp(argv[seq], "-s") == 0) {
			cip = argv[seq + 1];
			seq++;
		}
	}
	if (sip && sport)
		TcpClient(sip, sport,cip,cport);
	return 0;
}