#include "HttpDownloader.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "WinsockUtils.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <memory>

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace task;


// --------------------------------------------------------------------------------------------------------------------
// ������� ������� �� ��������� �����.
// --------------------------------------------------------------------------------------------------------------------
void SendRequest(SOCKET connection, std::string_view pageUrl, std::string_view hostName);

// --------------------------------------------------------------------------------------------------------------------
// ��������� �� HTTP ����������� ����� ���� � ������� ��� � ��������� �����.
// --------------------------------------------------------------------------------------------------------------------
void ReceiveResponse(SOCKET connection, std::ostream& out);


// --------------------------------------------------------------------------------------------------------------------
void task::HttpDownloader::AddDependencyRecognizer(const TagRecognizer& tagRecognizer)
{
	m_recognizers.push_back(tagRecognizer);
}

// --------------------------------------------------------------------------------------------------------------------
void HttpDownloader::DownloadFile(std::string_view pageUrl, std::ostream& out)
{
	// �������������� ����� (TCP) � ������������ ��� �������� ��� ������ �� �������.
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	auto socketCloser = [](SOCKET* pSocket) { closesocket(*pSocket); };
	std::unique_ptr<SOCKET, decltype(socketCloser)> connectionCloser(&connection, socketCloser);

	if (connection == INVALID_SOCKET)
		throw WinsockSocketException();

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
		ReceiveResponse(connection, out);
}


// --------------------------------------------------------------------------------------------------------------------
void HttpDownloader::DownloadPageWithDependencies(
	std::string_view difectory,
	std::string_view fileName,
	std::string_view pageUrl,
	std::function<void(const std::string&)> failedDependencyDownloadProcessor)
{
	using std::string;

	// ���������� ����� �� ����� ����� � ����� ��� ����.
	std::ostringstream ssTmp;
	ssTmp << difectory << '/' << fileName;
	string fullSubdirectoryName = ssTmp.str();	// ������ ��� ����� ���� ����� ������������ �����������.

	ssTmp << ".html";
	string fullFilenameName = ssTmp.str();	// ������ ��� html-����� �� �����.

	// �������� ��� html-��������.
	ssTmp.str("");
	ssTmp.clear();
	DownloadFile(pageUrl, ssTmp);
	string source = ssTmp.str();

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
				std::ofstream out(fullSubdirectoryName + '/' + GetFileName(absolutePath));
				DownloadFile(absolutePath, out);
				out.close();
			}
			catch (const WinsockException&)
			{
				failedDependencyDownloadProcessor(path);
			}
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
void ReceiveResponse(SOCKET connection, std::ostream& out)
{
	const size_t bufsize = 512;
	char buffer[bufsize];

	// 1. ������� ��������� ��������� ��������� �� \r\n\r\n.
	std::string headers;
	{
		int received;

		char rnrn[] = "\r\n\r\n";
		auto rnrnLength = strlen(rnrn);

		while (true)
		{
			received = recv(connection, buffer, bufsize - 1, 0);	// -1 - ����� ���� ���� ��������� '\0'

			// ���� received ������ 0 �� ���� ��� �� ����� ����� ����������, �� ��� �� ���������.
			if (!received)
				throw WinsockException("The end of the headers is not found");
			if (received < 0)
				throw WinsockSocketException();

			// ������ ������ ��� � ������ ������������ � ������������, ���� ������ � ����� ������
			// ���� rnrnLength �������� �� ��������� �� ����� ������.
			auto searchFrom = max(0, headers.length() - rnrnLength);	// ������������� ����� -> 0

			buffer[received] = '\0';
			headers += buffer;

			auto rnrnPos = headers.find(rnrn, searchFrom);
			if (rnrnPos == std::string::npos)
				continue;
			
			// �����.
			headers = headers.substr(0, rnrnPos + rnrnLength);
			break;

			// buffer[rnrnPos + rnrnLength, received - 1] - ������ ����� ������, �������, ������, ����� � �� ����.
		}
	}

	// 2. ����������� ���������, ����� ���������� ������ � ������ ��������.
	{
		// ��������� ��� �� ������ ������ �� ������� ���� 2��.
		std::regex xSuccess(R"(HTTP\/[0-9]+\.[0-9]+\s+2[0-9]{2}(.|\r|\n)*)");
		if (!std::regex_match(headers, xSuccess))
			throw WinsockException("The ansver is not 2xx.");		// ��� ������������ ���������������, 404 � ������.

		// TODO: Content-Length
		// TODO: ��������� ��� [rnrnPos + rnrnLength, received - 1] �������
	}

	// 3. ������� ������ ����� ������ � �����.
	/*
	std::ostringstream source;
	source << buffer;

	// 4. ������� � ����� ���� ���������� ����.
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

	// ����������� �� ����������.
	auto result = source.str();
	auto pos = result.find(rnrn);
	if (pos == std::string::npos)
		throw WinsockException("Headers end is not found.");
	result = result.substr(start);

	out << result;
	*/
}

// --------------------------------------------------------------------------------------------------------------------
