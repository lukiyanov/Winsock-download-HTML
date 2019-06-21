#include "HttpDownloader.h"
#include "Exceptions.h"
using namespace task;

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")


// ----------------------------------------------------------------------------
HttpDownloader::WinsockInitializerAndCleaner HttpDownloader::m_winsockInitializerAndCleaner;

// ----------------------------------------------------------------------------
HttpDownloader::WinsockInitializerAndCleaner::WinsockInitializerAndCleaner()
{
	if (!IsWinsockInitialized())
		InitializeWinsock();
}

// ----------------------------------------------------------------------------
HttpDownloader::WinsockInitializerAndCleaner::~WinsockInitializerAndCleaner()
{
	if (IsWinsockInitialized())
		CleanupWinsock();
}

// ----------------------------------------------------------------------------
bool HttpDownloader::WinsockInitializerAndCleaner::IsWinsockInitialized()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED)
		return false;

	closesocket(sock);
	return true;
}

// ----------------------------------------------------------------------------
void HttpDownloader::WinsockInitializerAndCleaner::InitializeWinsock()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		WSACleanup();
		throw WinsockInitializeException();
	}
}

// ----------------------------------------------------------------------------
void HttpDownloader::WinsockInitializerAndCleaner::CleanupWinsock()
{
	if (WSACleanup())
		throw WinsockCleanupException();
}

// ----------------------------------------------------------------------------
