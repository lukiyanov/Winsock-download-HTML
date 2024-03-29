#include "HttpDownloader.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "WinsockUtils.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <memory>
#include <vector>
#include <optional>

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace task;
using std::string;
using std::string_view;
using std::ostringstream;
using std::pair;
using std::vector;
using std::optional;

// --------------------------------------------------------------------------------------------------------------------
// ������� ������� �� ��������� �����.
// --------------------------------------------------------------------------------------------------------------------
void SendRequest(SOCKET connection, string_view pageUrl, string_view hostName);

// --------------------------------------------------------------------------------------------------------------------
// ��������� �� HTTP ����������� ����� ���� � ������� ��� � ��������� �����.
// --------------------------------------------------------------------------------------------------------------------
void ReceiveResponse(SOCKET connection, std::ostream& out);

// --------------------------------------------------------------------------------------------------------------------
// ��������� �� HTTP ��������� ������������ ����� �����.
// ���������� <���������, ������> ��� ������ - ������ ����� ������, ��������� ����� ��������� (����� ���� ������).
// --------------------------------------------------------------------------------------------------------------------
pair<string, vector<char>> ReceiveHeaders(SOCKET connection);

// --------------------------------------------------------------------------------------------------------------------
// ������������ ��������� HTTP � ���� ���-�� �� ���, ���������� ����������.
// ���������� �������� ��������� Content-Length, ���� �� ��� ���������.
// ���� �� ��� - std::nullopt.
// --------------------------------------------------------------------------------------------------------------------
optional<size_t> ProcessHeaders(const string& headers);

// --------------------------------------------------------------------------------------------------------------------
// �������� ������ �� ������ � ���������� � ����� ����� byteCount ����.
// --------------------------------------------------------------------------------------------------------------------
void ReceiveBytes(SOCKET connection, std::ostream& out, size_t byteCount);

// --------------------------------------------------------------------------------------------------------------------
// �������� ��� ������ �� ������ �� �������� ���������� � ���������� �� � �����.
// --------------------------------------------------------------------------------------------------------------------
void ReceiveBytesToEnd(SOCKET connection, std::ostream& out);




// --------------------------------------------------------------------------------------------------------------------
void task::HttpDownloader::AddDependencyRecognizer(const TagRecognizer& tagRecognizer)
{
	m_recognizers.push_back(tagRecognizer);
}


// --------------------------------------------------------------------------------------------------------------------
void HttpDownloader::DownloadFile(string_view pageUrl, std::ostream& out)
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
	string_view difectory,
	string_view fileName,
	string_view pageUrl,
	std::function<void(const string&, const std::exception&)> failedDependencyDownloadProcessor)
{
	// ���������� ����� �� ����� ����� � ����� ��� ����.
	ostringstream ssTmp;
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
	std::ofstream file(fullFilenameName, std::ios::out | std::ios::binary);
	file << source;
	file.flush();

	// C�������� ���������, �� ������� ������� ������.
	{
		// ��������� �� html ��� �����������.
		auto extractedPaths = ExtractPatternsFromSource(source, m_recognizers);
		string hostName(GetHttpHostNameByUrl(pageUrl));

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
				std::ofstream out(fullSubdirectoryName + '/' + GetFileName(absolutePath), std::ios::out | std::ios::binary);
				DownloadFile(absolutePath, out);
				out.close();
			}
			catch (const WinsockException& ex)
			{
				failedDependencyDownloadProcessor(path, ex);
			}
		}
	}
}


// --------------------------------------------------------------------------------------------------------------------
void SendRequest(SOCKET connection, string_view pageUrl, string_view hostName)
{
	ostringstream ss;
	ss << "GET " << pageUrl << " HTTP/1.0\r\n"
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
	auto [headers, firstPartOfData] = ReceiveHeaders(connection);
	auto contentLength = ProcessHeaders(headers);

	// �� ��, ����� ��������� ����.
	if (contentLength.has_value())
		contentLength = *contentLength - firstPartOfData.size();

	if (!firstPartOfData.empty())
		out.write(&firstPartOfData[0], firstPartOfData.size());

	if (contentLength.has_value())
		ReceiveBytes(connection, out, *contentLength);
	else
		ReceiveBytesToEnd(connection, out);
}


// --------------------------------------------------------------------------------------------------------------------
pair<string, vector<char>> ReceiveHeaders(SOCKET connection)
{
	const size_t bufsize = 512;
	char buffer[bufsize];

	string headers;
	int received;

	char rnrn[] = "\r\n\r\n";
	auto rnrnLength = strlen(rnrn);
	size_t rnrnPos, headersPrevLength;

	do
	{
		received = recv(connection, buffer, bufsize - 1, 0);	// -1 - ����� ���� ���� ��������� '\0'

		// ���� received ������ 0 �� ���� ��� �� ����� ����� ����������, �� ��� �� ���������.
		if (!received)
			throw WinsockException("The end of the headers is not found");
		if (received < 0)
			throw WinsockSocketException();

		// ������ ������ ��� � ������ ������������ � ������������, ���� ������ � ����� ������
		// ���� rnrnLength �������� �� ��������� �� ����� ������.
		headersPrevLength = headers.length();
		auto searchFrom = ((headersPrevLength > rnrnLength) ? headersPrevLength - rnrnLength : 0);

		buffer[received] = '\0';
		headers += buffer;

		rnrnPos = headers.find(rnrn, searchFrom);
	} while (rnrnPos == string::npos);

	// �����.
	// buffer[0, border - 1] - �� ��� ��������� ����� ����������, ������ '\n'.
	// buffer[border, received - 1] - ������ ����� ������, �������, ������, ����� � �� ����.
	// buffer[received] - '\0', ������� �� ��������� ����.
	const size_t border = rnrnPos - headersPrevLength + rnrnLength;
	if (border > static_cast<size_t>(received))	// ������-�� ������ �� ����� ����, �� ����� ��������� �������� �� ���.
		throw std::logic_error("ReceiveHeaders(): rnrnPos - headersPrevLength + rnrnLength > received");

	return pair(
		headers.substr(0, rnrnPos + rnrnLength),
		vector(&buffer[border], &buffer[received])
	);
}


// --------------------------------------------------------------------------------------------------------------------
// ����������� ���������, ����� ���������� ������ � ������ ��������.
optional<size_t> ProcessHeaders(const string& headers)
{
	// ��������� ��� �� ������ ������ �� ������� ���� 2��.
	// ��� ������������ ���������������, 404 � ������.
	std::regex xSuccess(R"(\s*HTTP\/[0-9]+\.[0-9]+\s+2[0-9]{2}[\S\s]*)", std::regex::icase);
	if (!std::regex_match(headers, xSuccess))
		throw WinsockException("The ansver is not 2xx.");

	// ���� Content-Length.
	std::regex xContentLength(R"(Content-Length:.*?[0-9]+.*?\r\n)", std::regex::icase);
	std::smatch match;
	if (std::regex_search(headers, match, xContentLength))
	{
		std::string contentLengthLine = match.str();
		// �������� ����� �� ������.
		if (!std::regex_search(contentLengthLine, match, std::regex("[0-9]+")))
			throw std::runtime_error("The search for the byte count in the Content-Length header failed");
			
		return atol(match.str().c_str());
	}

	return std::nullopt;
}


// --------------------------------------------------------------------------------------------------------------------
void ReceiveBytesToEnd(SOCKET connection, std::ostream& out)
{
	const size_t bufsize = 512;
	char buffer[bufsize];
	size_t received;

	// ������� � ����� �� ��� ������ �������.
	do
	{
		received = recv(connection, buffer, bufsize - 1, 0);
		if (received > 0)
			out.write(&buffer[0], received);
		else if (received < 0)
			throw WinsockSocketException();

	} while (received);
}


// --------------------------------------------------------------------------------------------------------------------
void ReceiveBytes(SOCKET connection, std::ostream& out, size_t byteCount)
{
	const size_t bufsize = 512;
	char buffer[bufsize];
	size_t received, receivedTotal = 0;

	// ������� � ����� ����� byteCount ����.
	do
	{
		received = recv(connection, buffer, bufsize - 1, 0);
		if (received > 0)
		{
			receivedTotal += received;

			if (receivedTotal <= byteCount)
				out.write(&buffer[0], received);
			else
				throw WinsockException("Received more bytes than indicated at the Content-Length field.");
		}
		else if (received < 0)
			throw WinsockSocketException();

	} while (received);

	if (receivedTotal < byteCount)
		throw WinsockException("Received less bytes than indicated at the Content-Length field.");
}


// --------------------------------------------------------------------------------------------------------------------
