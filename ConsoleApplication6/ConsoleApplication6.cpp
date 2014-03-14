// ConsoleApplication6.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

int		http_get(char *url);
SOCKET	StartClient(char *sServer, char *sPort, int iFamily = AF_UNSPEC, int iSocketType = SOCK_STREAM);

int _tmain(int argc, _TCHAR* argv[])
{
	int		result;
	WSADATA	wsaData;
	FILE	*fi, *fo;
	char	buffer[4096];
	wchar_t	wbuffer[4096];
	size_t	n;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);

//	fopen_s(&fi, "C:\\Users\\Ate\\Downloads\\pg_mappkg", "rb");
//	fopen_s(&fo, "C:\\Users\\Ate\\Downloads\\pg_mappkg_r.txt", "w, ccs=UNICODE");

	fopen_s(&fi, "C:\\Users\\Ate\\Downloads\\pg_voicefiles", "rb");
	fopen_s(&fo, "C:\\Users\\Ate\\Downloads\\pg_voicefiles_r.txt", "w, ccs=UNICODE");

	//	fputc(0xfe, fo);
	//	fputc(0xff, fo);

	while (fgets(buffer, 3, fi))
	{
		int tag, temp, length;

		sscanf_s(buffer, "%X", &tag);

		fgets(buffer, 3, fi);
		sscanf_s(buffer, "%X", &length);
		fgets(buffer, 3, fi);
		sscanf_s(buffer, "%X", &temp);
		length += temp * 0x100;

		fgets(buffer, 2 * length + 1, fi);

		switch (tag)
		{
		case 0x15:
		case 0x53:
		case 0x54:
			for (int i = 0; i < length; i++)
				sscanf_s(buffer + 2 * i, "%02X", buffer + i);
			wprintf(L"tag 0x%02X, length %3d, value %s\n", tag, length, buffer);
			fwprintf(fo, L"tag 0x%02X, length %3d, value %s\n", tag, length, buffer);
			if (tag == 0x54)
			{
				if (wcsstr((wchar_t *)buffer, L"http://"))
				{
					char url[4096];
					int  r;

					wcstombs_s(&n, url, sizeof(url), (wchar_t *)buffer, _TRUNCATE);
					while (r = http_get(url))
					{
						printf("Mislukt %d !!!\n", r);
						fwprintf(fo, L"Mislukt %d !!!\n", r);
					}
					//					wchar_t qq[4096], *f;
					//					int		r;
					//
					//					f = wcsrchr((wchar_t *)buffer, '/');
					//					swprintf_s(qq, _countof(qq), L"powershell.exe -Command (new-object System.Net.WebClient).DownloadFile('%s','C:\\Users\\Ate\\Downloads\\q\\%s')", buffer, f + 1);
					//					wcstombs_s(&n, buffer, sizeof(buffer), qq, _TRUNCATE);
					//					while (r = system(buffer))
					//					{
					//						printf("Mislukt %d !!!\n", r);
					//						fwprintf(fo, L"Mislukt %d !!!\n", r);
					//					}
				}
			}
			break;
		default:
			mbstowcs_s(&n, wbuffer, _countof(wbuffer), buffer, _TRUNCATE);
			wprintf(L"tag 0x%02X, length %3d, value %s\n", tag, length, wbuffer);
			fwprintf(fo, L"tag 0x%02X, length %3d, value %s\n", tag, length, wbuffer);
			break;
		}
	}

	fclose(fi);
	fclose(fo);

	return 0;
}

int http_get(char *url)
{
	__int64	i, j, k;
	char	host[4096], document[4096];
	char	buffer[8192];
	char	*content;
	char	*content_length = "Content-Length: ";
	void	*q = NULL;
	SOCKET	sockfd;
	FILE	*fp;
	time_t	t = 0;

	strncpy_s(host, sizeof(host), url + 7, strchr(url + 7, '/') - url - 7);
	strcpy_s(document, sizeof(document), strchr(url + 7, '/'));

	sockfd = StartClient(host, "80");

//	sprintf_s(buffer, _countof(buffer), "C:\\Users\\Ate\\Downloads\\N9\\mappkg\\%s", strrchr(url, '/') + 1);
	sprintf_s(buffer, _countof(buffer), "C:\\Users\\Ate\\Downloads\\N9\\voicefiles\\%s", strrchr(url, '/') + 1);
	fopen_s(&fp, buffer, "wb");

	i = _snprintf_s(buffer, sizeof(buffer), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", document, host);
	if ((i < 0) || (i >= sizeof(buffer)))
		return -1;

	k = 0;
	while ((j = send(sockfd, buffer, (int)i, 0)) >= 0)
	{
		if ((i - j - k) == 0)
			break;
		else
			k += j;
	}
	if (j < 0)
	{
		printf("http_get: write(...) returned %d (errno = %d)\n", j, WSAGetLastError());

		fclose(fp);
		shutdown(sockfd, SD_BOTH);
		closesocket(sockfd);

		return WSAGetLastError();
	}

	i = 0;
	j = 0;
	while (1)
	{
		int n;
		char c;
		while ((n = recv(sockfd, &c, 1, 0)) >= 0)
		{
			if (n == 1)
				break;
		}
		if (n < 1)
		{
			printf("http_get_a: read(...) returned %d (errno = %d)\n", n, WSAGetLastError());

			fclose(fp);
			shutdown(sockfd, SD_BOTH);
			closesocket(sockfd);

			return WSAGetLastError();
		}

		buffer[i++] = c;

		if (j & 1)
		{
			if (c == 10)
				j++;
			else
				j = 0;
		}
		else
		{
			if (c == 13)
				j++;
			else
				j = 0;
		}

		if (j == 4)
			break;
	}


	buffer[i] = 0;

	content = strstr(buffer, content_length);
	if (content == NULL)
		return -1;

	i = _atoi64(content + strlen(content_length));

//	content = (char *)malloc(i + 1);

	k = 0;
	while ((j = recv(sockfd, buffer, (int)min(sizeof(buffer), i - k), 0)) >= 0)
	{
		time_t now = time(NULL);

		if (j > 0)
			fwrite(buffer, 1, (int)j, fp);

		if (now != t)
		{
			printf("\r                                \r%3lli%%, %lli / %lli", 100 * k / i, k, i);
			t = now;
		}

		if ((i - k - j) == 0)
			break;
		else
			k += j;
	}
	printf("\r                                \r");
	if (j < 0)
	{
		printf("http_get_b: read(...) returned %lli (errno = %d)\n", j, WSAGetLastError());

		free(content);
		content = NULL;

		fclose(fp);
		shutdown(sockfd, SD_BOTH);
		closesocket(sockfd);

		return WSAGetLastError();
	}

	fclose(fp);
	shutdown(sockfd, SD_BOTH);
	closesocket(sockfd);

	return 0;
}

SOCKET StartClient(char *sServer, char *sPort, int iFamily, int iSocketType)
{
	int					i, iRetVal, iAddrLen;
	char				sAddrName[NI_MAXHOST];
	ADDRINFO			Hints, *AddrInfo = NULL, *AI = NULL;
	SOCKET				sSocket = INVALID_SOCKET;
	sockaddr_storage	Addr;

	sSocket = INVALID_SOCKET;

	memset(&Hints, 0, sizeof (Hints));
	Hints.ai_family		= iFamily;
	Hints.ai_socktype	= iSocketType;

	iRetVal = getaddrinfo(sServer, sPort, &Hints, &AddrInfo);
	if (iRetVal != 0)
		printf("StartClient: getaddrinfo(...) returned %d (errno = %d)\n", iRetVal, WSAGetLastError());

	for (i = 0, AI = AddrInfo; AI != NULL; i++, AI = AI->ai_next)
	{
		sSocket = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
		if (sSocket == INVALID_SOCKET)
		{
			printf("StartClient: socket(...) returned INVALID_SOCKET (erro = %d)\n", WSAGetLastError());
			continue;
		}

		do
		{	// prevent potential self-connect
			memset(&Addr, 0, sizeof(Addr));
			Addr.ss_family = AI->ai_family;
			iRetVal = bind(sSocket, (sockaddr *)&Addr, sizeof(Addr));
			if (iRetVal == SOCKET_ERROR)
			{
				printf("StarClient: bind(...) returned SOCKET_ERROR (errno = %d)\n", WSAGetLastError());
				continue;
			}

			iAddrLen = sizeof(Addr);
			iRetVal = getsockname(sSocket, (LPSOCKADDR)&Addr, &iAddrLen);
			if (iRetVal == SOCKET_ERROR)
			{
				printf("StartClient: getsockname(...) returned SOCKET_ERROR (errno = %d)\n", WSAGetLastError());
				continue;
			}
		} while (ntohs(SS_PORT(&Addr)) == ntohs(SS_PORT(AI->ai_addr)));

//		printf("Attempting to connect to: %s\n", sServer);
		iRetVal = connect(sSocket, AI->ai_addr, (int)AI->ai_addrlen);
		if (iRetVal == 0)
			break;
		else if (iRetVal == SOCKET_ERROR)
		{
			printf("StartClient: connect(...) returned SOCKET_ERROR (errno = %d)\n", WSAGetLastError());

			if (getnameinfo(AI->ai_addr, (int)AI->ai_addrlen, sAddrName, sizeof(sAddrName), NULL, 0, NI_NUMERICHOST) != 0)
				strcpy_s(sAddrName, sizeof(sAddrName), "(unknown)");
			printf("Failed to connect to %s\n", sAddrName);

			iRetVal = closesocket(sSocket);
			if (iRetVal == SOCKET_ERROR)
				printf("StartClient: closesocket(...) returned SOCKET_ERROR (errno = %d)\n", WSAGetLastError());
		}
	}

	if (AI == NULL)
	{
		printf("Fatal error, unable to connect to the server\n");
		freeaddrinfo(AddrInfo);
		return -1;
	}

	iAddrLen = sizeof(Addr);
	iRetVal = getpeername(sSocket, (LPSOCKADDR)&Addr, &iAddrLen);
	if (iRetVal == 0)
	{
		if (getnameinfo((LPSOCKADDR)&Addr, iAddrLen, sAddrName, sizeof(sAddrName), NULL, 0, NI_NUMERICHOST) != 0)
			strcpy_s(sAddrName, sizeof(sAddrName), "(unknown)");
//		printf("Connected to %s, port %d, protocol %s, protocol family %s\n",
//				sAddrName, ntohs(SS_PORT(&Addr)),
//				(AI->ai_socktype == SOCK_STREAM) ? "TCP" : "UDP",
//				(AI->ai_family == PF_INET) ? "AF_INET" : "AF_INET6");
	}
	else if (iRetVal == SOCKET_ERROR)
		printf("StartClient: getpeername(...) returned SOCKET_ERROR (errno = %d)\n", WSAGetLastError());

	freeaddrinfo(AddrInfo);

	iAddrLen = sizeof(Addr);
	iRetVal = getsockname(sSocket, (LPSOCKADDR)&Addr, &iAddrLen);
	if (iRetVal == 0)
	{
        if (getnameinfo((LPSOCKADDR)&Addr, iAddrLen, sAddrName, sizeof(sAddrName), NULL, 0, NI_NUMERICHOST) != 0)
			strcpy_s(sAddrName, sizeof(sAddrName), "(unknown)");
//		printf("Using local address %s, port %d\n", sAddrName, ntohs(SS_PORT(&Addr)));
	}
	else if (iRetVal == SOCKET_ERROR)
		printf("StartClient: getsockname(...) returned SOCKET_ERROR (errno = %d)\n", WSAGetLastError());

	return sSocket;
}