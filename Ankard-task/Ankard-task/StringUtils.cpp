#include "StringUtils.h"
#include "Exceptions.h"
#include <algorithm>
#include <string>
using namespace task;

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
	static const size_t expectedPrefixLength = sizeof(expectedPrefix) - 1; // -1, �.�. '\0' ������� �� �����.

	// 1. ��������� ������� �������� "http://". ���� ������ ��� => �������� ������.
	auto prefix = ToLower(pageUrl.substr(0, expectedPrefixLength));
	if (prefix != expectedPrefix)
		throw InvalidUrlFormat();

	// 2. ���� ������ "/" �����. �� ����� => ����� ������� �� ����� ������, ����� => ����� - �� ��.
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
