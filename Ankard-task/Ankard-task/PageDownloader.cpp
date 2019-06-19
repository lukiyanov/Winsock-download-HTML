#include "PageDownloader.h"
#include "Exceptions.h"
#include <winsock.h>
#include <fstream>

using namespace task;
using namespace std;

#pragma comment(lib, "wsock32.lib")

///////////////////////////////////////////////////////////////////////////////
//
//	PageDownloader
//
///////////////////////////////////////////////////////////////////////////////

unsigned long GetServerAddress(const string& pageUrl)
{
	auto url = pageUrl.c_str();

	hostent* hostInfo = nullptr;
	if (auto ip = inet_addr(url); ip != INADDR_NONE)
		hostInfo = gethostbyaddr((char*)& ip, sizeof(ip), AF_INET);	// 123.45.67.8
	else
		hostInfo = gethostbyname(url);								// maps.google.com

	if (!hostInfo)
		throw std::exception("TODO");	// TODO

	return *((unsigned long*)hostInfo->h_addr);
}

// ----------------------------------------------------------------------------
void PageDownloader::DownloadPageToDirectory(const string& pageUrl, const string& directory)
{
	// Инициализируем сокет как TCP.
	SOCKET connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connection == INVALID_SOCKET)
		throw std::exception("TODO");	// TODO

	// Заполняем адрес сервера.
	sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_addr.s_addr = GetServerAddress(pageUrl);
	server.sin_family = AF_INET;
	server.sin_port = htons(80);

	// Устанавливаем соединение.
	if (bool failed = connect(connection, (sockaddr*)& server, sizeof(server)); failed)
	{
		closesocket(connection);
		throw std::exception("Can't establish the connection");	// TODO
	}

	// Отсылаем запрос на получение файла.
	char request[] =
		"GET / HTTP/1.1\r\n"
		"Host: www.101zenstories.com\r\n"
		"Connection: close\r\n"
		"\r\n";
	send(connection, request, strlen(request), 0);

	// Получаем ответ и заодно собираем по кусочкам запрошенный файл.
	std::ofstream file("C:\\Users\\guest\\Documents\\GitHub\\test.html");

	const int bufsize = 51200;
	char buff[bufsize];
	int received = recv(connection, buff, bufsize, 0);
	file.write(buff, received);
	file.flush();







	closesocket(connection);

	// TODO
	//throw std::exception("TODO");
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
