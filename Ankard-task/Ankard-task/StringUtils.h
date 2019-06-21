#pragma once
#include <string_view>
#include <list>
#include "TagRecognizer.h"

namespace task
{
	// --------------------------------------------------------------------
	// ����������� ������������ ����� � ������ �������.
	// --------------------------------------------------------------------
	std::string ToLower(std::string_view str);

	// --------------------------------------------------------------------
	// ��������� ��� ����� �� ������ ��������.
	// --------------------------------------------------------------------
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
	// ������� ��� ��������� � ������������ � ��������� ����������.
	// --------------------------------------------------------------------
	std::list<std::string> ExtractPatternsFromSource(const std::string& source, const std::list<TagRecognizer>& recognizers);
}