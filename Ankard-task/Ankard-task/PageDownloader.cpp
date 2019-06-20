#include "PageDownloader.h"
#include "Exceptions.h"
#include <fstream>
#include <sstream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace task;

///////////////////////////////////////////////////////////////////////////////
//
//	PageDownloader
//
///////////////////////////////////////////////////////////////////////////////

unsigned long GetServerAddress(const std::string& pageUrl)
{
	auto url = pageUrl.c_str();

	hostent* hostInfo = nullptr;
	if (auto ip = inet_addr(url); ip != INADDR_NONE)
		hostInfo = gethostbyaddr((char*)& ip, sizeof(ip), AF_INET);	// 123.45.67.8
	else
		hostInfo = gethostbyname(url);								// maps.google.com

	if (!hostInfo)
		throw WinsockSocketException();

	return *((unsigned long*)hostInfo->h_addr);
}

// ----------------------------------------------------------------------------
void PageDownloader::DownloadPageToDirectory(const std::string& pageUrl, std::ostream& out)
{
	// Инициализируем сокет (TCP).
	SOCKET connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connection == INVALID_SOCKET)
		throw WinsockSocketException();

	try
	{
		// Получаем имя хоста
		// http://<имя_хоста>/.../...
		std::string hostName = "shelek.com";	// TODO

		// Заполняем адрес сервера.
		sockaddr_in server;
		ZeroMemory(&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = GetServerAddress(hostName);
		server.sin_port = htons(80);

		// Устанавливаем соединение.
		if (bool failed = connect(connection, (sockaddr*)& server, sizeof(server)); failed)
			throw WinsockSocketException();

		// Отсылаем запрос на получение файла.
		std::ostringstream ss;
		ss << "GET "   << pageUrl << " HTTP/1.1\r\n"
		   << "Host: " << "shelek.com" << "\r\n"
		   << "\r\n";
		auto request = ss.str();
		if (SOCKET_ERROR == send(connection, request.c_str(), request.length(), 0))
			throw WinsockSocketException();


		// Получаем ответ. (TODO: вынести в отдельную функцию, как и отправку запроса)
		{
			const int bufsize = 4 * 1024;
			static char buff[bufsize];
			int received = recv(connection, buff, bufsize, 0);

			// Получаем код результата. Если не 2хх, то не удалось загрузить.
			// ...

			// Находим размер файла, следующего за заголовком.
			// ...

			//for (int received = recv(connection, buff, bufsize, 0); received > 0; )
			{
				if (received == SOCKET_ERROR)
					throw WinsockSocketException();
				out.write(buff, received);
			}
			out.flush();
		}

		closesocket(connection);
	}
	catch (...)
	{
		closesocket(connection);
		throw;
	}
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
