#include "StringUtils.h"
#include "Exceptions.h"
#include <algorithm>
#include <string>
#include <sstream>
using namespace task;

//#define _REGEX_MAX_STACK_COUNT 10000
#include <regex>
//#pragma comment(linker, "/STACK:10000000")	// для regex - он весьма прожорлив.

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

	// Для каждого из искомых тегов, чтобы понять, какой из них дал матч.
	std::list<std::pair<std::regex, std::regex>> regexForTags;

	// Строим полное regex-выражение для поиска всех тегов.
	std::ostringstream fullRegex;
	auto recognizer = recognizers.cbegin();
	regexForTags.push_back(std::pair(std::regex((*recognizer).outerPattern), std::regex((*recognizer).innerPattern)));
	fullRegex << "(" << (*recognizer).outerPattern << ")";
	++recognizer;

	for (; recognizer != recognizers.cend(); ++recognizer)
	{
		fullRegex << "|(" << (*recognizer).outerPattern << ")";

		regexForTags.push_back(std::pair(std::regex((*recognizer).outerPattern), std::regex((*recognizer).innerPattern)));
	}
	

	// Ищем (независимо от регистра).
	std::regex xRule(fullRegex.str(), std::regex::icase);
	auto sourceBegin = std::sregex_iterator(source.begin(), source.end(), xRule);
	auto sourceEnd   = std::sregex_iterator();

	for (std::sregex_iterator i = sourceBegin; i != sourceEnd; ++i)
	{
		std::string matchSubstr = (*i).str();

		// Что-то нашли. Ищем, какое из выражений сработало.
		for (auto rec = regexForTags.cbegin(), end = regexForTags.cend(); rec != end; ++rec)
		{
			if (std::regex_match(matchSubstr, (*rec).first))
			{
				// Нашли, какое. Теперь ищем атрибут с адресом внутри тега.
				std::smatch match;
				if (std::regex_search(matchSubstr, match, (*rec).second))
				{
					// Ок, нашли атрибут, содержащий адрес.
					std::string attribute = match.str();

					// Наконец, выделяем сам адрес.
					if (std::regex_search(attribute, match, std::regex(R"(((".+?")|('.+?')))")))
					{
						std::string address = match.str();
						// Избавляемся от кавычек окончательно.
						address = address.substr(1, address.length() - 2);
						results.push_back(address);
					}
				}

				break;	// Раз этот сработал, остальные теги пропускаем.
			}
		}
	}

	return results;
}

// --------------------------------------------------------------------
std::string task::GetAbsoluteHttpPath(const std::string& path, const std::string& hostName)
{
	// Каждую зависимость рассматриваем как один из четырёх случаев:
	// 1. Относительный путь => добавляем "http://имя-хоста.com/".
	// 2. Абсолютный путь HTTP => оставляем как есть.
	// 3. Абсолютный путь HTTPS => меняем на HTTP и пытаемся скачать.
	// 4. Абсолютный путь с другим протоколом => пропускаем.

	auto protocolEndsPos = path.find("://");
	if (protocolEndsPos == std::string::npos)
		return std::string("http://") + hostName + '/' + path;

	auto protocol = path.substr(0, protocolEndsPos);
	if (protocol == "http")
		return path;
	else if (protocol == "https")
		return std::string("http") + path.substr(protocolEndsPos);

	return std::string();
}

// --------------------------------------------------------------------
std::string task::GetFileName(const std::string& path)
{
	auto lastSlash = path.find_last_of('/');
	if (lastSlash == std::string::npos)
		return path;

	return path.substr(lastSlash + 1);
}

// --------------------------------------------------------------------
