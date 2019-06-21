#pragma once
#include <string_view>

namespace task
{
	// --------------------------------------------------------------------
	// ����������� ����� � ������ �������.
	// --------------------------------------------------------------------
	std::string ToLower(std::string_view str);

	// --------------------------------------------------------------------
	// ��������� ��� ����� �� ������ ��������.
	// ���������� ������:
	//		http://<���_�����>/.../.../
	//		http://<���_�����>/.../...
	//		http://<���_�����>
	// ������� ������� �������� � ������ ��������� HTTP (� ������ ���),
	// ����� ���������� ����������.
	// ��������� ������� � IP-��������, ���������� � ������� ����.
	// ������� ������ �������.
	// --------------------------------------------------------------------
	std::string_view GetHttpHostNameByUrl(std::string_view pageUrl);

	// --------------------------------------------------------------------
}