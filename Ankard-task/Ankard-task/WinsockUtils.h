#pragma once
#include <string_view>

namespace task
{
	// ----------------------------------------------------------------------------
	// ����������� IP-����� �������� � ��������� ����� ��� ��� ����� � IP-�����,
	// ������� � ������� UL, ������� ����� ���������������� ��� ���� sockaddr_in �� ���.
	// ������� ���������� ����������: "123.45.67.8" ��� "qwe.rty.com".
	// ----------------------------------------------------------------------------
	unsigned long GetServerAddress(std::string_view hostName);

	// ----------------------------------------------------------------------------
}