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
// Отсылка запроса на получение файла.
// --------------------------------------------------------------------------------------------------------------------
void SendRequest(SOCKET connection, string_view pageUrl, string_view hostName);

// --------------------------------------------------------------------------------------------------------------------
// Принимает по HTTP запрошенный ранее файл и выводит его в указанный поток.
// --------------------------------------------------------------------------------------------------------------------
void ReceiveResponse(SOCKET connection, std::ostream& out);

// --------------------------------------------------------------------------------------------------------------------
// Считывает по HTTP заголовки запрошенного ранее файла.
// Возвращает <заголовок, данные> где данные - первый пакет данных, считанный после заголовка (может быть пустым).
// --------------------------------------------------------------------------------------------------------------------
pair<string, vector<char>> ReceiveHeaders(SOCKET connection);

// --------------------------------------------------------------------------------------------------------------------
// Обрабатывает заголовки HTTP и если что-то не так, генерирует исключение.
// Возвращает значение заголовка Content-Length, если он был обнаружен.
// Если не был - std::nullopt.
// --------------------------------------------------------------------------------------------------------------------
optional<size_t> ProcessHeaders(const string& headers);

// --------------------------------------------------------------------------------------------------------------------
// Скачивает из сокета и записывает в поток ровно byteCount байт.
// Если в качестве byteCount передан std::nullopt, то скачивает данные до закрытия сокета.
// --------------------------------------------------------------------------------------------------------------------
void DownloadToStream(SOCKET connection, std::ostream& out, optional<size_t> byteCount);



// --------------------------------------------------------------------------------------------------------------------
void task::HttpDownloader::AddDependencyRecognizer(const TagRecognizer& tagRecognizer)
{
	m_recognizers.push_back(tagRecognizer);
}

// --------------------------------------------------------------------------------------------------------------------
void HttpDownloader::DownloadFile(string_view pageUrl, std::ostream& out)
{
	// Инициализируем сокет (TCP) и обеспечиваем его закрытие при выходе из функции.
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	auto socketCloser = [](SOCKET* pSocket) { closesocket(*pSocket); };
	std::unique_ptr<SOCKET, decltype(socketCloser)> connectionCloser(&connection, socketCloser);

	if (connection == INVALID_SOCKET)
		throw WinsockSocketException();

		// Получаем имя хоста.
		auto hostName = GetHttpHostNameByUrl(pageUrl);

		// Заполняем адрес сервера.
		sockaddr_in server = {0};
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = GetServerAddress(hostName);
		server.sin_port = htons(80);

		// Устанавливаем соединение.
		if (SOCKET_ERROR == connect(connection, (sockaddr*)& server, sizeof(server)))
			throw WinsockSocketException();

		// Скачиваем саму страницу.
		SendRequest(connection, pageUrl, hostName);
		ReceiveResponse(connection, out);
}


// --------------------------------------------------------------------------------------------------------------------
void HttpDownloader::DownloadPageWithDependencies(
	string_view difectory,
	string_view fileName,
	string_view pageUrl,
	std::function<void(const string&)> failedDependencyDownloadProcessor)
{
	// Определяем имена на диске файла и папки для него.
	ostringstream ssTmp;
	ssTmp << difectory << '/' << fileName;
	string fullSubdirectoryName = ssTmp.str();	// Полное имя папки куда будем складировать зависимости.

	ssTmp << ".html";
	string fullFilenameName = ssTmp.str();	// Полное имя html-файла на диске.

	// Получаем код html-страницы.
	ssTmp.str("");
	ssTmp.clear();
	DownloadFile(pageUrl, ssTmp);
	string source = ssTmp.str();

	// Сохраняем html-файл.
	std::ofstream file(fullFilenameName);
	file << source;
	file.flush();

	// Cкачиваем документы, от которых зависит данный.
	{
		// Извлекаем из html все зависимости.
		auto extractedPaths = ExtractPatternsFromSource(source, m_recognizers);
		string hostName(GetHttpHostNameByUrl(pageUrl));

		// Если есть зависимости - создаём под них директорию.
		if (!extractedPaths.empty())
			std::filesystem::create_directory(fullSubdirectoryName);

		// Скачиваем зависимости.
		for (auto path : extractedPaths)
		{
			auto absolutePath = GetAbsoluteHttpPath(path, hostName);
			if (absolutePath.empty())
				continue;

			// Каждую из зависимостей пытаемся скачать как отдельный файл, по очереди.
			try // Часть зависимостей может не скачаться по тем или иным причинам.
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
void SendRequest(SOCKET connection, string_view pageUrl, string_view hostName)
{
	ostringstream ss;
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
	auto [headers, firstPartOfData] = ReceiveHeaders(connection);
	auto contentLength = ProcessHeaders(headers);

	// Всё ок, можно скачивать файл.
	if (contentLength.has_value())
		contentLength = *contentLength - firstPartOfData.size();
	out.write(&firstPartOfData[0], firstPartOfData.size());
	DownloadToStream(connection, out, contentLength);
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
		received = recv(connection, buffer, bufsize - 1, 0);	// -1 - чтобы было куда поместить '\0'

		// Если received вернул 0 ДО того как мы нашли конец заголовков, то это не нормально.
		if (!received)
			throw WinsockException("The end of the headers is not found");
		if (received < 0)
			throw WinsockSocketException();

		// Искать каждый раз с начала бессмысленно и неэффективно, ищем только в новых данных
		// плюс rnrnLength символов от скачанных до этого частей.
		headersPrevLength = headers.length();
		auto searchFrom = ((headersPrevLength > rnrnLength) ? headersPrevLength - rnrnLength : 0);

		buffer[received] = '\0';
		headers += buffer;

		rnrnPos = headers.find(rnrn, searchFrom);
	} while (rnrnPos == string::npos);

	// Нашли.
	// Пусть border = rnrnPos - headersPrevLength + rnrnLength, тогда:
	// buffer[0, border - 1] - всё ещё последняя часть заголовков, символ '\n'.
	// buffer[border, received - 1] - первая часть данных, которых, однако, может и не быть.
	// buffer[received] - '\0', который мы поставили выше.
	return pair(
		headers.substr(0, rnrnPos + rnrnLength),
		vector(&buffer[rnrnPos - headersPrevLength + rnrnLength], &buffer[received])
	);
}

// --------------------------------------------------------------------------------------------------------------------
// Анализируем заголовки, узнаём успешность ответа и размер контента.
optional<size_t> ProcessHeaders(const string& headers)
{
	// Проверяем код из первой строки на наилчие кода 2хх.
	// Тут отваливаются перенаправления, 404 и прочее.
	std::regex xSuccess(R"(HTTP\/[0-9]+\.[0-9]+\s+2[0-9]{2}(.|\r|\n)*?)", std::regex::icase);
	if (!std::regex_match(headers.substr(0, 30), xSuccess))	// 30 символов достаточно для анализа первой строки
		throw WinsockException("The ansver is not 2xx.");

	// Ищем Content-Length.
	std::regex xContentLength(R"(Content-Length:.*?[0-9]+.*?\r\n)", std::regex::icase);
	std::smatch match;
	if (std::regex_search(headers, match, xContentLength))
	{
		std::string contentLengthLine = match.str();
		// Выделяем число из строки.
		if (!std::regex_search(contentLengthLine, match, std::regex("[0-9]+")))
			throw std::runtime_error("The search for the byte count in the Content-Length header failed");
			
		return atol(match.str().c_str());
	}

	return std::nullopt;
}

// --------------------------------------------------------------------------------------------------------------------
void DownloadToStream(SOCKET connection, std::ostream& out, optional<size_t> byteCount)
{
/*
	// 4. Выводим в поток весь оставшийся файл.
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
	auto pos = result.find(rnrn);
	if (pos == string::npos)
		throw WinsockException("Headers end is not found.");
	result = result.substr(start);

	out << result;
*/
}

// --------------------------------------------------------------------------------------------------------------------
