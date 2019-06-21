#include "PageDownloader.h"
#include "Exceptions.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <regex>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace task;

///////////////////////////////////////////////////////////////////////////////
//
//	PageDownloader
//
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// "qwe.rty.com" -> UL, ������� ����� ���������������� ��� ���� sockaddr_in �� ���.
unsigned long GetServerAddress(const std::string& hostName)
{
	auto url = hostName.c_str();

	hostent* hostInfo = nullptr;
	if (auto ip = inet_addr(url); ip != INADDR_NONE)
		hostInfo = gethostbyaddr((char*)& ip, sizeof(ip), AF_INET);	// ��� ������ ������� �� ��������, TODO: �������������� � ��� ��������� ��� �������� ���� ��� � ������ ����.
	else
		hostInfo = gethostbyname(url);

	if (!hostInfo)
		throw WinsockSocketException();

	return *((unsigned long*)hostInfo->h_addr);
}

// ----------------------------------------------------------------------------
// ������� ������� �� ��������� �����.
void SendRequest(SOCKET connection, std::string_view pageUrl, std::string_view hostName)
{
	std::ostringstream ss;
	ss << "GET " << pageUrl << " HTTP/1.1\r\n"
		<< "Host: " << hostName << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n";
	auto request = ss.str();
	if (SOCKET_ERROR == send(connection, request.c_str(), request.length(), 0))
		throw WinsockSocketException();
}

// ----------------------------------------------------------------------------
// ���������� ���������� ������������� �����, ���� �� ��������� ������� �����.
std::string ReceivePage(SOCKET connection)
{
	const size_t bufsize = 512;
	char buffer[bufsize];

	// �������� ��������� � �� ����������� ��� ��������� �����.
	int received = recv(connection, buffer, bufsize - 1, 0);	// -1 - ����� ���� ���� ��������� '\0'
	if (!received)
		throw WinsockException("Can\'t read the page.");
	else if (received < 0)
		throw WinsockSocketException();

	// ��������� ��� �� ������ ������ �� ������� ���� 2��.
	// 30 �������� - ������ ���������� ��� ������� ������ ������.
	std::string firstLine(buffer, buffer + min(30, received));

	std::regex xSuccess(R"(HTTP\/[0-9]+\.[0-9]+\s+2[0-9]{2}(.|\r|\n)*)");
	if (!std::regex_match(firstLine, xSuccess))				// �� �������. ��� ������������ ��� ���������������, 404 � ������.
		throw WinsockException("The ansver is not 2xx.");

	// ��������� ������ ��������� ������ � ����������.
	buffer[received] = '\0';
	std::ostringstream source;
	source << buffer;

	// ��������� �� ���������.
	do
	{
		received = recv(connection, buffer, bufsize - 1, 0);
		if (received >= 0)
		{
			buffer[received] = '\0';
			source << buffer;
		}
		else
			throw WinsockSocketException();

	} while (received);

	return source.str();
}

// ----------------------------------------------------------------------------
inline void WriteSourceToStream(std::ostream& out, const std::string& what)
{
	out << what;
	out.flush();
}

// --------------------------------------------------------------------
// ����������� ����� � ������ �������.
inline std::string ToLower(std::string_view str)
{
	std::string result = str.data();
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return std::move(result);
}

// --------------------------------------------------------------------
// ���������� ������: http://<���_�����>/.../... ��� http://<���_�����>
std::string GetHostNameByUrl(std::string_view pageUrl)
{
	static const char expectedPrefix[] = "http://";
	static const size_t expectedPrefixLength = sizeof(expectedPrefix) - 1; // '\0' �� �������

	// 1. ������� "http://". ���� ������ ��� => �������� ������.
	auto hostName = ToLower(pageUrl);
	if (auto prefix = hostName.substr(0, expectedPrefixLength); prefix != expectedPrefix)
		throw InvalidUrlFormat();
	
	hostName = hostName.substr(expectedPrefixLength);

	// 2. ���� ������ "/" �����. �� ����� => ����� ������� �� ����� ������, ����� => ����� - �� ��.
	auto slashPos = hostName.find('/');
	if (slashPos == std::string::npos)
		return hostName;

	return hostName.substr(0, slashPos);
}

// ----------------------------------------------------------------------------
void PageDownloader::DownloadPageToDirectory(const std::string& pageUrl, std::ostream& out)
{
	// �������������� ����� (TCP).
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
		throw WinsockSocketException();

	try
	{
		// �������� ��� �����.
		auto hostName = GetHostNameByUrl(pageUrl);

		// ��������� ����� �������.
		sockaddr_in server;
		ZeroMemory(&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = GetServerAddress(hostName);
		server.sin_port = htons(80);

		// ������������� ����������.
		if (bool failed = connect(connection, (sockaddr*)& server, sizeof(server)); failed)
			throw WinsockSocketException();

		// ��������� ���� ��������.
		SendRequest(connection, pageUrl, hostName);
		std::string pageSource = ReceivePage(connection);
		WriteSourceToStream(out, pageSource);

		// ������������ �����������.
		// TODO

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

	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
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
