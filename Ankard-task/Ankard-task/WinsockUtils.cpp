#include "WinsockUtils.h"
#include "Exceptions.h"

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")


// ----------------------------------------------------------------------------
unsigned long task::GetServerAddress(std::string_view hostName)
{
	std::string hostNameTmp{ hostName };
	auto url = hostNameTmp.c_str();

	// ��� IP?
	if (auto ip = inet_addr(url); ip != INADDR_NONE)
		return ip;

	// ���� �� IP, �� ������� ��� ��� ��� �������.
	hostent* hostInfo = gethostbyname(url);
	if (!hostInfo)
		throw WinsockSocketException();
	return *((unsigned long*)hostInfo->h_addr);
}


// ----------------------------------------------------------------------------


