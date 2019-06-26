#include "HttpDownloader.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "WinsockUtils.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace task;


// --------------------------------------------------------------------------------------------------------------------
// ������� ������� �� ��������� �����.
// --------------------------------------------------------------------------------------------------------------------
void SendRequest(SOCKET connection, std::string_view pageUrl, std::string_view hostName);

// --------------------------------------------------------------------------------------------------------------------
// ��������� �� HTTP ����������� ����� ���� � ���������� ��� � ���� ������.
// --------------------------------------------------------------------------------------------------------------------
std::string ReceiveResponse(SOCKET connection);

// --------------------------------------------------------------------------------------------------------------------
// ������� ����� �� "<!DOCTYPE", ���� �� ����. ���� ��� - �� ������ ������.
// --------------------------------------------------------------------------------------------------------------------
void TrimFront(std::string& htmlSource);


// --------------------------------------------------------------------------------------------------------------------
void task::HttpDownloader::AddDependencyRecognizer(const TagRecognizer& tagRecognizer)
{
	m_recognizers.push_back(tagRecognizer);
}

// --------------------------------------------------------------------------------------------------------------------
/// ����� ��� ������� ����� �������� ����������� ���� ���� ����� �� ���������, ��������, ��������.
std::string HttpDownloader::DownloadFile(std::string_view pageUrl)
{
	// �������������� ����� (TCP).
	/// �� ����� �� ������������ ����� �������?
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
		throw WinsockSocketException();

	try
	{
		// �������� ��� �����.
		auto hostName = GetHttpHostNameByUrl(pageUrl);

		// ��������� ����� �������.
		sockaddr_in server = {0};
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = GetServerAddress(hostName);
		server.sin_port = htons(80);

		// ������������� ����������.
		if (SOCKET_ERROR == connect(connection, (sockaddr*)& server, sizeof(server)))
			throw WinsockSocketException();

		// ��������� ���� ��������.
		SendRequest(connection, pageUrl, hostName);
		std::string source = ReceiveResponse(connection);

		closesocket(connection);
		return source;
	}
	catch (...)
	{
		closesocket(connection);
		throw;
	}
}


// --------------------------------------------------------------------------------------------------------------------
void HttpDownloader::DownloadPageWithDependencies(
	std::string_view difectory,
	std::string_view fileName,
	std::string_view pageUrl)
{
	using std::string;

	// ���������� ����� �� ����� ����� � ����� ��� ����.
	std::ostringstream ssTmp;
	ssTmp << difectory << '/' << fileName;
	string fullSubdirectoryName = ssTmp.str();	// ������ ��� ����� ���� ����� ������������ �����������.

	ssTmp << ".html";
	string fullFilenameName = ssTmp.str();	// ������ ��� html-����� �� �����.

	// �������� ��� html-��������.
	ssTmp.clear();
	string source = DownloadFile(pageUrl);

	// ��������� html-����.
	std::ofstream file(fullFilenameName);
	file << source;
	file.flush();

	// C�������� ���������, �� ������� ������� ������.
	{
		// ��������� �� html ��� �����������.
		auto extractedPaths = ExtractPatternsFromSource(source, m_recognizers);
		std::string hostName(GetHttpHostNameByUrl(pageUrl));

		// ���� ���� ����������� - ������ ��� ��� ����������.
		if (!extractedPaths.empty())
			std::filesystem::create_directory(fullSubdirectoryName);

		// ��������� �����������.
		for (auto path : extractedPaths)
		{
			auto absolutePath = GetAbsoluteHttpPath(path, hostName);
			if (absolutePath.empty())
				continue;

			// ������ �� ������������ �������� ������� ��� ��������� ����, �� �������.
			try // ����� ������������ ����� �� ��������� �� ��� ��� ���� ��������.
			{
				std::string fileSource = DownloadFile(absolutePath);
				std::ofstream out(fullSubdirectoryName + '/' + GetFileName(absolutePath));
				out << fileSource;
				out.close();
			}
			/// ������ ������������ �� ����� ������, ���� �� ������������� ������������
			catch (const WinsockException&) {}	// �� ������ ������. �� ��������� - � ���� � ���.
		}
	}
}


// --------------------------------------------------------------------------------------------------------------------
void SendRequest(SOCKET connection, std::string_view pageUrl, std::string_view hostName)
{
	std::ostringstream ss;
	ss << "GET " << pageUrl << " HTTP/1.1\r\n"
		<< "Host: " << hostName << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n";
	auto request = ss.str();
	if (SOCKET_ERROR == send(connection, request.c_str(), static_cast<int>(request.length()), 0))
		throw WinsockSocketException();
}


// --------------------------------------------------------------------------------------------------------------------
std::string ReceiveResponse(SOCKET connection)
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
	/// � ���� � ������ ��� ������� ������ ��� ���������� ��� ��������?�
	std::string firstLine(buffer, buffer + min(30, received));

	std::regex xSuccess(R"(HTTP\/[0-9]+\.[0-9]+\s+2[0-9]{2}(.|\r|\n)*)");
	if (!std::regex_match(firstLine, xSuccess))		// �� �������. ��� ������������ ���������������, 404 � ������.
		throw WinsockException("The ansver is not 2xx.");

	// ��������� ������ ��������� ������ � ����������.
	std::ostringstream source;
	buffer[received] = '\0';
	source << buffer;

	// ��������� �� ���������.
	do
	{
		/// ���� � ������� ���� CloseConnection, ������ �� �������� ������ ��� http �� ����� ������� ����, ����� ������������ ��������� ContentSize
		received = recv(connection, buffer, bufsize - 1, 0);
		if (received >= 0)
		{
			buffer[received] = '\0';
			source << buffer;
		}
		else
			throw WinsockSocketException();

	} while (received);

	// ����������� �� ����������.
	auto result = source.str();
	char rnrn[] = "\r\n\r\n";
	auto pos = result.find(rnrn);
	if (pos == std::string::npos)
		throw WinsockException("Headers end is not found.");
	auto start = pos + strlen(rnrn);
	result = result.substr(start);

	return result;
}

// --------------------------------------------------------------------------------------------------------------------
void TrimFront(std::string& htmlSource)
{
	auto pos = htmlSource.find("<!DOCTYPE");
	if (pos == std::string::npos)
		return;

	htmlSource = htmlSource.substr(pos);
}

// --------------------------------------------------------------------------------------------------------------------
