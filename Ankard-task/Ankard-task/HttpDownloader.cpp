#include "HttpDownloader.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "WinsockUtils.h"
#include <fstream>
#include <sstream>
#include <regex>

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace task;


// ----------------------------------------------------------------------------
// Отсылка запроса на получение файла.
// ----------------------------------------------------------------------------
void SendRequest(SOCKET connection, std::string_view pageUrl, std::string_view hostName);

// ----------------------------------------------------------------------------
// Принимает по HTTP запрошенный ранее файл и возвращает его в виде строки.
// ----------------------------------------------------------------------------
std::string ReceiveResponse(SOCKET connection);

// ----------------------------------------------------------------------------
// Удаляет мусор до "<!DOCTYPE", если он есть. Если нет - не делает ничего.
// ----------------------------------------------------------------------------
void TrimFront(std::string& htmlSource);

// ----------------------------------------------------------------------------
std::string HttpDownloader::DownloadFile(std::string_view pageUrl)
{
	// Инициализируем сокет (TCP).
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
		throw WinsockSocketException();

	try
	{
		// Получаем имя хоста.
		auto hostName = GetHttpHostNameByUrl(pageUrl);

		// Заполняем адрес сервера.
		sockaddr_in server;
		ZeroMemory(&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = GetServerAddress(hostName);
		server.sin_port = htons(80);

		// Устанавливаем соединение.
		if (bool failed = connect(connection, (sockaddr*)& server, sizeof(server)); failed)
			throw WinsockSocketException();

		// Скачиваем саму страницу.
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


// ----------------------------------------------------------------------------
void HttpDownloader::DownloadPageWithDependencies(std::string_view difectory, std::string_view fileName, std::string_view pageUrl)
{
	using std::string;

	// Определяем имена на диске файла и папки для него.
	std::ostringstream ssTmp;
	ssTmp << difectory << '/' << fileName;
	string fullSubirectoryName = ssTmp.str();	// Полное имя папки куда будем складировать зависимости.

	ssTmp << ".html";
	string fullFilenameName = ssTmp.str();	// Полное имя html-файла на диске.

	// Получаем код html-страницы.
	ssTmp.clear();
	string source = DownloadFile(pageUrl);
	TrimFront(source);	// костыль

	// Сохраняем html-файл.
	std::ofstream file(fullFilenameName);
	file << source;
	file.flush();

	// TODO: анализируем и скачиваем зависимости
}


// ----------------------------------------------------------------------------
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
std::string ReceiveResponse(SOCKET connection)
{
	const size_t bufsize = 512;
	char buffer[bufsize];

	// Получаем заголовки и их анализируем вне основного цикла.
	int received = recv(connection, buffer, bufsize - 1, 0);	// -1 - чтобы было куда поместить '\0'
	if (!received)
		throw WinsockException("Can\'t read the page.");
	else if (received < 0)
		throw WinsockSocketException();

	// Проверяем код из первой строки на наилчие кода 2хх.
	// 30 символов - вполне достаточно для анализа первой строки.
	std::string firstLine(buffer, buffer + min(30, received));

	std::regex xSuccess(R"(HTTP\/[0-9]+\.[0-9]+\s+2[0-9]{2}(.|\r|\n)*)");
	if (!std::regex_match(firstLine, xSuccess))				// Не удалось. Тут отваливаются все перенаправления, 404 и прочее.
		throw WinsockException("The ansver is not 2xx.");

	// Добавляем первую считанную порцию к результату.
	std::ostringstream source;
	buffer[received] = '\0';
	source << buffer;

	// Считываем всё остальное.
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

	// Избавляемся от заголовков.
	auto result = source.str();
	char rnrn[] = "\r\n\r\n";
	auto pos = result.find(rnrn);
	if (pos == std::string::npos)
		throw WinsockException("Headers end is not found.");
	auto start = pos + sizeof(rnrn) - 1;
	result = result.substr(start);

	return result;
}

// ----------------------------------------------------------------------------
void TrimFront(std::string& htmlSource)
{
	auto pos = htmlSource.find("<!DOCTYPE");
	if (pos == std::string::npos)
		return;

	htmlSource = htmlSource.substr(pos);
}

// ----------------------------------------------------------------------------
