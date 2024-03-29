#pragma once
#include <stdexcept>
#include <string>

namespace task
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	�������� ������ ��������� ������.
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class CommandLineException : public std::runtime_error
	{
	public:
		CommandLineException(const std::string& what_arg)
			: runtime_error(what_arg)
		{}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	��������� ���������� �� ����������.
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class DirectoryDoesNotExist : public std::runtime_error
	{
	public:
		DirectoryDoesNotExist()
			: runtime_error("Directory does not exist")
		{}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	������ ��� ������ � Winsock (����� ������).
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class WinsockException : public std::runtime_error
	{
	public:
		WinsockException(const std::string& what_arg)
			: runtime_error(what_arg)
		{}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	������ ��� ������������� Winsock.
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class WinsockInitializeException : public WinsockException
	{
	public:
		WinsockInitializeException()
			: WinsockException("Winsock initialization error")
		{}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	������ ��� ������� Winsock.
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class WinsockCleanupException : public WinsockException
	{
	public:
		WinsockCleanupException()
			: WinsockException("Winsock cleanup error")
		{}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	������ ��� ������ � �������� Winsock, ����� ������ � ��������, �������
	//	����� �������������� ����� WSAGetLastError().
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class WinsockSocketException : public WinsockException
	{
	public:
		WinsockSocketException()
			: WinsockException(GetLastWinsockError().c_str())
		{}

	private:
		std::string GetLastWinsockError();
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	�� �� ����� �������� � URL ������ �������.
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class InvalidUrlFormat : public std::runtime_error
	{
	public:
		InvalidUrlFormat()
			: runtime_error("Invalid URL format.")
		{}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}