
#define  _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#ifdef _WIN32


#include <stdio.h>
#include <string.h>
#include <intrin.h>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <Ws2tcpip.h>

#include <conio.h>

#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable :4996)

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








using namespace std;


#pragma pack(1)

typedef struct TCPSERVERPARAM {
	int sock;
	sockaddr_in sa;
};

#pragma pack()






void printdata(char* data, int size) {
	printf("\r\n");
	for(int i = 0; i < size; i++) {
		unsigned char cc = data[i];
		printf("%02x ", (unsigned char)cc);
		if((i+1) % 16 == 0) {
			printf("\r\n");
		}
		else if ((i + 1) % 8 == 0) {
			printf("  ");
		}
	}
	printf("\r\n");
}

int  UdpClient(char* ip, int port) {
	printf("%s %d ip:%s port:%d\r\n", __FUNCTION__, __LINE__,ip, port);
	int ret = 0;
	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (s == -1) {
		perror("socket error\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
#ifdef __linux__
	sa.sin_addr.s_addr = inet_addr(ip);
#elif defined(_WIN32)
	sa.sin_addr.S_un.S_addr = inet_addr(ip);
#else

#endif
	sa.sin_port = ntohs(port);

	printf("presend data to %s %d\r\n", ip, port);

	char sendBuf[0x1000] = { 0 };
	const char* tag = "Hello Udp server,I am Udp client!\r\n";
	strcpy(sendBuf, tag);
	int sendLen = sendto(s, sendBuf, strlen(tag), 0, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (sendLen > 0) {
		char recvbuf[0x1000];
		socklen_t addrlen = sizeof(sockaddr_in);
		sockaddr_in addr = { 0 };
		int recvlen = recvfrom(s, recvbuf, sizeof(recvbuf) - 1, 0, (sockaddr*)&addr, &addrlen);
		if (recvlen <= 0) {
			perror("recvfrom error\r\n");
		}
		else {
			recvbuf[recvlen] = 0;
			printf("recvfrom bytes:%s,size:%d\r\n", recvbuf, recvlen);
		}
	}
	else {
		perror("sendto error\r\n");
	}
#ifdef _WIN32
	closesocket(s);
#elif defined(__linux__)
	close(s);
#endif

	return 0;
}


int  UdpServer(char * ip,int port) {
	printf("%s %d ip:%s port:%d\r\n", __FUNCTION__, __LINE__, ip, port);

	int ret = 0;
	int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == -1) {
		perror("socket error\r\n");
		return -1;
	}
	DWORD dwip = INADDR_ANY;
	if (ip) {
		dwip = inet_addr(ip);
	}
	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
#ifdef __linux__
	sa.sin_addr.s_addr = dwip;
#elif defined(_WIN32)
	sa.sin_addr.S_un.S_addr = dwip;
#else

#endif
	sa.sin_port = ntohs(port);

	ret = bind(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if(ret < 0) {
		perror("bind error\r\n");
		return -1;
	}

	int recvbufsize = 0x10000;
	char* recvbuf = new char[recvbufsize];
	while (1) {
		
		socklen_t calen = sizeof(sockaddr_in);
		sockaddr_in ca = { 0 };
		int recvlen = recvfrom(s, recvbuf, recvbufsize-1, 0, (sockaddr*)&ca, &calen);
		if (recvlen < 0) {
			perror("recvfrom error\r\n");
			return -1;
		}
		else if(recvlen == 0) {
			//printf("recvfrom zero bytes\r\n");
		}
		else {
			recvbuf[recvlen] = 0;
			char* strip = inet_ntoa(ca.sin_addr);
			int port = ntohs(ca.sin_port);
			printf("%s %d recv ip:%s port:%d size:%d,data:%s\r\n",
				__FUNCTION__, __LINE__, strip, ntohs(ca.sin_port), recvlen, recvbuf);

			//printdata(recvbuf, recvlen);

			char sendbuf[1024];
			const char* str = "Hello Udp client,I am Udp server!\r\n";
			strcpy(sendbuf, str);
			int sendLen = sendto(s, sendbuf, strlen(str),0,(sockaddr*) & ca, sizeof(sockaddr));
			/*
			HANDLE hf = CreateFileA("udpsrv.data", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hf != INVALID_HANDLE_VALUE) {
				int fs = SetFilePointer(hf, 0, 0, FILE_END);
				DWORD cnt = 0;
				ret = WriteFile(hf, recvbuf, recvlen, &cnt, 0);
				CloseHandle(hf);
			}*/
		}
	}
#ifdef _WIN32
	closesocket(s);
#elif defined(__linux__)
	close(s);
#endif

	return 0;
}




typedef int (*ptrsocket)(int, int, int);
typedef int (*ptrconnect)(int, sockaddr*, int);
typedef int (*ptrsend)(int,  char*, int,int);
typedef int (*ptrrecv)(int, char*, int,int);
typedef int (*ptrclose)(int);
typedef unsigned short (*ptrntohs)(unsigned short);

#ifdef _WIN32

#else
__attribute__((aligned(256)))
#endif

int  LinuxTcpClient() {

	ptrsocket lpsocket = *(ptrsocket*)0x42957F0;
	
	ptrntohs lpntohs = *(ptrntohs*)0x4297100;

	ptrconnect lpconnect = *(ptrconnect*)0x4295618;
	ptrsend lpsend = *(ptrsend*)0x4295508;

	ptrrecv lprecv = *(ptrrecv*)0x42940D8;
	ptrclose lpclose = *(ptrclose*)0x42950F0;

	int ret = 0;

	int s = lpsocket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
#ifdef __linux__
	sa.sin_addr.s_addr = 0xc0a8a301;
#elif defined(_WIN32)
	sa.sin_addr.S_un.S_addr = 0x01a3a8c0 ;
#else

#endif
	sa.sin_port = lpntohs(12345);

	ret = lpconnect(s, (sockaddr*)&sa, sizeof(sockaddr_in));

	int bufsize = 1024;
	char sendBuf[1024];
	sendBuf[0] = 'H';
	sendBuf[1] = 'e';
	sendBuf[2] = 'l';
	sendBuf[3] = 'l';
	sendBuf[4] = '0';
	sendBuf[5] = 0;
	int sendLen = lpsend(s, sendBuf,  6, 0);

	char recvbuf[1024];

	int recvLen = 0;
	recvLen = lprecv(s, recvbuf, bufsize - 1, 0);

#ifdef _WIN32
	closesocket(s);
#elif defined(__linux__)
	lpclose(s);
#endif

	return 0;
}


int  TcpClient(char* ip,int port) {

	printf("%s %d ip:%s,port:%d\r\n", __FUNCTION__, __LINE__,ip,port);

	int ret = 0;

	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (s == -1) {
		perror("socket\r\n");
		return -1;
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
#ifdef __linux__
	sa.sin_addr.s_addr = inet_addr(ip);
#elif defined(_WIN32)
	sa.sin_addr.S_un.S_addr = inet_addr(ip);
#else

#endif
	sa.sin_port = ntohs(port);

	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret < 0) {
		perror("connect\r\n");
		return -1;
	}

	printf("%s %d\r\n", __FUNCTION__, __LINE__);

	int bufsize = 0x10000;
	char* sendBuf = new char[bufsize];
	const char* tag = "hello Tcp server,I'm tcp client!\r\n";

	strcpy(sendBuf, tag);
	int sendLen = send(s, sendBuf, strlen(tag), 0);
	if (sendLen > 0) {
		printf("%s %d send data:%s,size:%d ok\r\n", __FUNCTION__, __LINE__,sendBuf,strlen(sendBuf));
		char* recvbuf = new char[bufsize];
		int recvLen = recv(s, recvbuf, bufsize - 1, 0);
		if (recvLen > 0 && recvLen < bufsize) {
			recvbuf[recvLen] = 0;
			printf("recv data:%s,size:%d\r\n", recvbuf, recvLen);
		}
		else {
			printf("%s %d recv error\r\n", __FUNCTION__, __LINE__);
		}
		delete[] recvbuf;
	}
	else {
		printf("%s %d send error\r\n", __FUNCTION__, __LINE__);
	}



#ifdef _WIN32
	closesocket(s);
#elif defined(__linux__)
	close(s);
#endif

	delete [] sendBuf;

	return 0;
}





int __stdcall TcpServerThread(TCPSERVERPARAM* param) {

	int sc = (int)param->sock;
	sockaddr_in* ca = &param->sa;

	char sendbuf[0x1000] = { 0 };

	int recvBufSize = 0x10000;
	char* recvbuf = new char[recvBufSize];

	while (1) {
		int recvlen = recv(sc, recvbuf, recvBufSize - 1, 0);
		if (recvlen <= 0 || recvlen >= recvBufSize) {
			memset(recvbuf, 0, recvBufSize);
			break;
		}
		else {
			recvbuf[recvlen] = 0;
			char* strip = inet_ntoa(ca->sin_addr);
			int port = ntohs(ca->sin_port);
			printf("%s %d recv ip:%s port:%d size:%d,data:%s\r\n",__FUNCTION__, __LINE__, strip, ntohs(ca->sin_port), recvlen, recvbuf);
			printdata(recvbuf, recvlen);
			/*
			HANDLE hf = CreateFileA("tcpsrv.data", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hf != INVALID_HANDLE_VALUE) {
				int fs = SetFilePointer(hf, 0, 0, FILE_END);
				DWORD cnt = 0;
				ret = WriteFile(hf, recvbuf, recvlen, &cnt, 0);
				CloseHandle(hf);
			}*/
		}
	}
#ifdef _WIN32
	closesocket(sc);
#elif defined(__linux__)
	close(s);
#endif
	delete param;
	return 0;
}


int  TcpServer(char * ip,int port) {

	int ret = 0;
	printf("%s %d ip:%s port:%d\r\n", __FUNCTION__, __LINE__, ip, port);
	int ss = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (ss == -1) {
		perror("socket error\r\n");
		return -1;
	}

	DWORD dwip = INADDR_ANY;
	if (ip) {
		dwip = inet_addr(ip);
	}

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
#ifdef __linux__
	sa.sin_addr.s_addr = dwip;
#elif defined(_WIN32)
	sa.sin_addr.S_un.S_addr = dwip;
#else

#endif
	sa.sin_port = ntohs(port);

	ret = bind(ss, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret) {
		perror("bind\r\n");
		return -1;
	}

	ret = listen(ss, 16);

	while (1) {
		sockaddr_in ca;
		socklen_t sasize = sizeof(sockaddr_in);
		int sc = accept(ss, (sockaddr*)&ca, &sasize);
		if (sc != -1) {
			TCPSERVERPARAM* param = (TCPSERVERPARAM * )new char[sizeof(TCPSERVERPARAM)];
			memcpy(& param->sa, &ca, sizeof(sockaddr_in));
			param->sock = sc;

			HANDLE ht = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)TcpServerThread, param, 0, 0);
			if (ht)
				CloseHandle(ht);
		}
		else {
			continue;
		}
	}
	return 0;
}





int main(int argc,char ** argv) {

	int ret = 0;

	if (argc < 5) {
		printf("example:%s -s	0				443		-t\r\n", argv[0]);
		printf("example:%s -c	192.168.1.1		80		-t\r\n", argv[0]);
		return -1;
	}

#ifdef _WIN32
	WSADATA wsa = { 0 };
	ret = WSAStartup(0x0202, &wsa);
	if (ret) {
		return -1;
	}
#endif

	int servermode = 0;
	int port = 0;
	char* ip = 0;
	int protocol = 0;
	for (int num = 1; num < argc; num++) {
		char* cmd = argv[num];
		if (strcmp(cmd, "-u") == 0) {
			protocol = 17;
		}
		else if (strcmp(cmd, "-t") == 0) {
			protocol = 6;
		}
		else if (strcmp(cmd, "-c") == 0) {
			servermode = 0;
			ip = argv[num + 1];
			port = atoi(argv[num + 2]);
			num += 2;
		}
		else if (strcmp(cmd, "-s") == 0) {
			servermode = 1;
			ip = argv[num + 1];
			port = atoi(argv[num + 2]);
			num += 2;
		}
		else {
			printf("parse argv error\r\n");
			return 0;
		}
	}

	printf("server ip:%s,port:%d protocol:%d\r\n", ip, port, protocol);

	if (servermode) {
		if (protocol == 6) {
			TcpServer(ip, port);
		}
		else if (protocol == 17) {
			UdpServer(ip, port);
		}
	}
	else {
		if (protocol == 6) {
			TcpClient(ip, port);
		}
		else if (protocol == 17) {
			UdpClient(ip, port);
		}
	}

	printf("press any key to quit...\r\n");
	//ret = getchar();
	return 0;
}