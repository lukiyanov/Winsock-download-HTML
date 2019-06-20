#include "Exceptions.h"
#include <winsock2.h>

using namespace task;

#pragma comment(lib, "Ws2_32.lib")

// ----------------------------------------------------------------------------
std::string WinsockSocketException::GetLastWinsockError()
{
	int errCode = WSAGetLastError();

	char* message = NULL;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)& message, 0, NULL);
	std::string result = message;
	LocalFree(message);

	return result;
}

// ----------------------------------------------------------------------------
