#include "CmdLineParser.h"
#include "Exceptions.h"
using namespace task;
using namespace std;

// ----------------------------------------------------------------------------
CmdLineParser::CmdLineParser(int argc, char** argv)
{
	const int expectedArgCount = 2;
	if (argc != expectedArgCount + 1)
		throw CommandLineException("Invalid argument count (2 expected).");

	m_url       = argv[1];
	m_directory = argv[2];
}

// ----------------------------------------------------------------------------
