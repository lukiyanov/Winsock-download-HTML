#include "WinsockUtils.h"
#include "Exceptions.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS		// ��� gethostbyname
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// --------------------------------------------------------------------------------------------------------------------
unsigned long task::GetServerAddress(std::string_view hostName)
{
	std::string hostNameTmp{ hostName };
	auto url = hostNameTmp.c_str();

	// ��� IP?
	if (unsigned long result; inet_pton(AF_INET, url, &result) == 1)	// 1 == OK
		return result;

	// ���� �� IP, �� ������� ��� ��� ��� �������.
	hostent* hostInfo = gethostbyname(url);
	if (!hostInfo)
		throw WinsockSocketException();
	return *((unsigned long*)hostInfo->h_addr);
}

// --------------------------------------------------------------------------------------------------------------------

