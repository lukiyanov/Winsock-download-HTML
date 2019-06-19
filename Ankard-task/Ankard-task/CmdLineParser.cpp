#include "CmdLineParser.h"
#include "Exceptions.h"
#include <filesystem>
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

	CheckDirectoryExistance();
}

// ----------------------------------------------------------------------------
void CmdLineParser::CheckDirectoryExistance()
{
	const std::filesystem::path directoryPath = m_directory;

	if (!std::filesystem::is_directory(directoryPath))
		throw DirectoryDoesNotExist();
}

// ----------------------------------------------------------------------------
