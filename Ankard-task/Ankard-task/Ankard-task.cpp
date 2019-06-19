﻿#include <iostream>
#include "CmdLineParser.h"

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	using namespace task;
	try
	{
		CmdLineParser cmdLine(argc, argv);

		//CheckWriteAccessibility(cmdLine.GetDirectory());

		//PageDownloader page(cmdLine.GetUrl());

	}
	catch (std::exception& ex)
	{
		std::cout << "An exception has been raised.\n" << ex.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cout << "An unknown exception has been raised." << std::endl;
		return 2;
	}

	std::cout << "Done." << std::endl;
	return 0;
}

// ----------------------------------------------------------------------------
