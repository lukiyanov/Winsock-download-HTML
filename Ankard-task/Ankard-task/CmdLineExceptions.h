#pragma once
#include <stdexcept>

namespace task
{
	///////////////////////////////////////////////////////////////////////////
	//
	// ������ � ��������� ������.
	//
	///////////////////////////////////////////////////////////////////////////
	class CommandLineException : std::invalid_argument
	{
	public:
		CommandLineException(const std::string& what_arg)
			: invalid_argument(what_arg)
		{}
	};

	///////////////////////////////////////////////////////////////////////////
}