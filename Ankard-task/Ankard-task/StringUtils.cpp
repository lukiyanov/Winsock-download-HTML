#include "StringUtils.h"
#include "Exceptions.h"
#include <algorithm>
#include <string>
#include <sstream>
#include <regex>
using namespace task;

#pragma comment(linker, "/STACK:10485760")	// для regex_search()

// --------------------------------------------------------------------
std::string task::ToLower(std::string_view str)
{
	std::string result(str);
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

// --------------------------------------------------------------------
std::string_view task::GetHttpHostNameByUrl(std::string_view pageUrl)
{
	static const char expectedPrefix[] = "http://";
	static const size_t expectedPrefixLength = sizeof(expectedPrefix) - 1; // -1, т.к. '\0' считать не нужно.

	// 1. Проверяем наличие префикса "http://". Если такого нет => неверный формат.
	auto prefix = ToLower(pageUrl.substr(0, expectedPrefixLength));
	if (prefix != expectedPrefix)
		throw InvalidUrlFormat();

	// 2. Ищем первую "/" слева. Не нашли => адрес сервера до конца строки, нашли => адрес - до неё.
	auto slashPos = pageUrl.find('/', expectedPrefixLength);
	if (slashPos == std::string::npos)
	{
		auto tmp = pageUrl.substr(expectedPrefixLength);
		return tmp;
	}

	auto tmp = pageUrl.substr(expectedPrefixLength, slashPos - expectedPrefixLength);
	return tmp;
}

// --------------------------------------------------------------------
std::list<std::string> task::ExtractPatternsFromSource(const std::string& source, const std::list<TagRecognizer>& recognizers)
{
	std::list<std::string> results;
	if (recognizers.empty())
		return results;

	// Строим полное regex-выражение для поиска всех тегов.
	std::ostringstream fullRegex;
	auto recognizer = recognizers.cbegin();
	fullRegex << "(" << (*recognizer).outerPattern << ")";
	recognizer++;

	for (; recognizer != recognizers.cend(); recognizer++)
	{
		fullRegex << "|(" << (*recognizer).outerPattern << ")";
	}

	// Ищем.
	std::smatch match;
	std::regex xRule(fullRegex.str());

	std::string text = source;
	while (std::regex_search(text, match, xRule)) {
		std::string matching = match[0];

		// TODO: нашли подходящий тег, надо обработать.

		text = match.suffix().str();
	}
}

// --------------------------------------------------------------------
