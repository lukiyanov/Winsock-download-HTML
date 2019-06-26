#include <iostream>
#include "CmdLineParser.h"
#include "HttpDownloader.h"
#include "TagRecognizer.h"

// --------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	using namespace task;
	try
	{
		CmdLineParser cmdLine(argc, argv);

		HttpDownloader downloader;
		// Настраиваем, какие зависимости нужно скачивать.
		downloader.AddDependencyRecognizer(tags::link);
		downloader.AddDependencyRecognizer(tags::script);
		downloader.AddDependencyRecognizer(tags::img);
		// Собственно, скачиваем.
		downloader.DownloadPageWithDependencies(cmdLine.GetDirectory(), "page", cmdLine.GetUrl());
	}
	catch (const std::exception& ex)
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

// --------------------------------------------------------------------------------------------------------------------
