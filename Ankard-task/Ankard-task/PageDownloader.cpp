#include "PageDownloader.h"
#include "Exceptions.h"
#include <winsock.h>
using namespace task;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
//	PageDownloader
//
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
void PageDownloader::DownloadPageToDirectory(const string& pageUrl, const string& directory)
{
	// TODO
	throw std::exception("TODO");
}


///////////////////////////////////////////////////////////////////////////////
//
//	WinsockInitializerAndCleaner
//
///////////////////////////////////////////////////////////////////////////////

PageDownloader::WinsockInitializerAndCleaner PageDownloader::m_winsockInitializerAndCleaner;

// ----------------------------------------------------------------------------
PageDownloader::WinsockInitializerAndCleaner::WinsockInitializerAndCleaner()
{
	if (!IsWinsockInitialized())
		InitializeWinsock();
}

// ----------------------------------------------------------------------------
PageDownloader::WinsockInitializerAndCleaner::~WinsockInitializerAndCleaner()
{
	if (IsWinsockInitialized())
		CleanupWinsock();
}

// ----------------------------------------------------------------------------
bool PageDownloader::WinsockInitializerAndCleaner::IsWinsockInitialized()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED)
		return false;

	closesocket(sock);
	return true;
}

// ----------------------------------------------------------------------------
void PageDownloader::WinsockInitializerAndCleaner::InitializeWinsock()
{
	WSADATA wsaData;

	if (WSAStartup(0x0101, &wsaData))
	{
		WSACleanup();
		throw WinsockInitializeException();
	}
}

// ----------------------------------------------------------------------------
void PageDownloader::WinsockInitializerAndCleaner::CleanupWinsock()
{
	if (WSACleanup())
		throw WinsockCleanupException();
}

// ----------------------------------------------------------------------------
