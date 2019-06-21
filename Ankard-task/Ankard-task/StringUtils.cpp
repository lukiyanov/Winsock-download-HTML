#include "StringUtils.h"
#include "Exceptions.h"
#include <algorithm>
#include <string>
using namespace task;

// --------------------------------------------------------------------
std::string task::ToLower(std::string_view str)
{
	std::string result = str.data();
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}


// --------------------------------------------------------------------
std::string_view GetHttpHostNameByUrl(std::string_view pageUrl)
{
	static const char expectedPrefix[] = "http://";
	static const size_t expectedPrefixLength = sizeof(expectedPrefix) - 1; // -1, �.�. '\0' ������� �� �����.

	// 1. ������� "http://". ���� ������ ��� => �������� ������.
	auto hostName = ToLower(pageUrl);
	if (auto prefix = hostName.substr(0, expectedPrefixLength); prefix != expectedPrefix)
		throw InvalidUrlFormat();

	// 2. ���� ������ "/" �����. �� ����� => ����� ������� �� ����� ������, ����� => ����� - �� ��.
	auto slashPos = hostName.find('/');
	if (slashPos == std::string::npos)
		return hostName;

	hostName = hostName.substr(expectedPrefixLength, slashPos - expectedPrefixLength);
	return hostName;
}


// --------------------------------------------------------------------
