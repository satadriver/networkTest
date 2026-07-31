


#ifdef _WIN32
#include "winsock.h"
#include <Windows.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

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


#define _CRT_SECURE_NO_WARNINGS


struct NetAddress{
	unsigned int ip;
	int port;
};


void printdata(char* data, int size) {
	for (int i = 0; i < size; i++) {
		printf("%02x ", (unsigned char)data[i]);
		if ((i + 1) % 16 == 0) {
			printf("\r\n");
		}
	}
}



void writefile(char * buf,int size) {
#ifdef _WIN32
	int ret = 0;
	const char* logfile = "log.txt";
	HANDLE hf = CreateFileA(logfile, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hf != INVALID_HANDLE_VALUE) {
		int fs = SetFilePointer(hf, 0, 0, FILE_END);
		DWORD cnt = 0;
		ret = WriteFile(hf, buf, size, &cnt, 0);
		CloseHandle(hf);
	}
#elif defined __linux__
	const char* logfile = "log.txt";
	int fd = open(logfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
	ssize_t bytes = write(fd, buf, size);
	close(fd);
#endif
}



#ifdef __linux__
void* MyLinuxClient(void* param) {
	
	//sleep(3);

	printf("%s %d\r\n", __FUNCTION__, __LINE__);

	NetAddress* lpna = (NetAddress*)param;

	int ret = 0;
	int s = socket(2, 1, 0);
	//unsigned long s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1) {
		perror("socket error\r\n");
		printf("%s %d\r\n", strerror(errno));
		return 0;
	}
	
	printf("%s %d\r\n", __FUNCTION__, __LINE__);

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	if (lpna->ip == 0)
		sa.sin_addr.s_addr = 0;
	else
		sa.sin_addr.s_addr = lpna->ip;
	sa.sin_port = lpna->port;

	printf("server:%x port:%d\r\n",sa.sin_addr.s_addr,ntohs(sa.sin_port));
	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	printf("%s %d\r\n", __FUNCTION__, __LINE__);
	if (ret) {
		perror("connect error\r\n");
		return 0 ;
	}
	
	printf("%s %d\r\n", __FUNCTION__, __LINE__);

	const char* tag = "hello server!I am linux client!\r\n";

	char sendBuf[0x1000];
	strcpy(sendBuf, tag);
	int sendLen = send(s, sendBuf, strlen(tag), 0);

	char recvbuf[0x1000];
	int recvBufSize = sizeof(recvbuf);
	int recvlen = recv(s, recvbuf, recvBufSize - 1, 0);
	printf("%s %d recv bytes:%s\r\n", __FUNCTION__, __LINE__, recvbuf);
	
	printf("%s %d\r\n", __FUNCTION__, __LINE__);
	return 0;
}

#endif


int  TcpServer(char * strserver,int port) {
	printf("%s %d\r\n", __FUNCTION__, __LINE__);

	int ret = 0;
	unsigned long s = socket(2, 1, 0);
	//unsigned long s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1) {
		perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
#ifdef __linux__
	if (strserver == 0)
		sa.sin_addr.s_addr = 0;
	else
		sa.sin_addr.s_addr = inet_addr(strserver);
#else
	if (strserver == 0)
		sa.sin_addr.S_un.S_addr = 0;
	else
		sa.sin_addr.S_un.S_addr = inet_addr(strserver);
#endif
	sa.sin_port = ntohs(port);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret) {
		perror("bind\r\n");
		return -1;
	}

	ret = listen(s, 16);

	printf("%s %d listening ip:%s port:%d\r\n", __FUNCTION__, __LINE__,strserver, port);

	char sendBuf[0x1000];

	int recvBufSize = 0x10000;
	char* recvbuf =(char*) malloc(recvBufSize);


#ifdef __linux__
	NetAddress na = { 0 };
	na.ip = sa.sin_addr.s_addr;
	na.port = ntohs(port);

	unsigned long thread_id;
	pthread_create(&thread_id, NULL, MyLinuxClient, (void*)&na);
#endif


	printf("running...\r\n");
	while (1) {
		sockaddr_in ca;
#ifdef __linux__
		socklen_t casize = sizeof(sockaddr_in);
#else
		int casize = sizeof(sockaddr_in);
		
#endif
		
		int sc = accept(s, (sockaddr*)&ca, &casize);
		if (sc != -1) {

			const char* tag = "hello client!I am server!\r\n";

			strcpy(sendBuf, tag);
			int sendLen = send(sc, sendBuf, strlen(tag), 0);

			int recvlen = recv(sc, recvbuf, recvBufSize - 1, 0);
			if (recvlen <= 0 || recvlen >= recvBufSize) {
				//closesocket(sc);
				//break;
				printf("recv error\r\n");
			}
			else {
				recvbuf[recvlen] = 0;
				char* strip = inet_ntoa(ca.sin_addr);
				int cport = ntohs(ca.sin_port);
				printf("%s %d recv ip:%s port:%d bytes:%d,string:%s\r\n",
					__FUNCTION__, __LINE__, strip, ntohs(ca.sin_port), recvlen, recvbuf);
				printdata(recvbuf, recvlen);

				//closesocket(sc);
			}
		}
		else {
			continue;
		}
	}
	return 0;
}


int main(int argc, char** argv) {

	int ret = 0;
	
#ifdef _WIN32
	WSADATA wsa = { 0 };
	ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		return -1;
	}
#endif

	int port = 0;
	char* server = 0;
	int seq = 0;
	for (seq = 0; seq < argc; seq++) {
		char* item = argv[seq];
		if (strcmp(item, "--p") == 0) {
			port = atoi(argv[seq + 1]);
			seq++;
		}
		else if (strcmp(item, "--s") == 0) {
			port = atoi(argv[seq + 1]);
			server = argv[seq + 1];
			seq++;
		}
		else {

		}
	}

	if(port )
		TcpServer(server,port);

	printf("%s %d\r\n", __FUNCTION__, __LINE__);
	return 0;

}


