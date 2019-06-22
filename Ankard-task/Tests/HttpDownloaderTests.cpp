#include "pch.h"
#include <CppUnitTest.h>
#include <filesystem>

// Тестируемый класс и необходимые для этого зависимости.
#include <HttpDownloader.h>
#include <Exceptions.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace task;

namespace Tests
{
	TEST_CLASS(HttpDownloaderTests)
	{
	public:
		TEST_METHOD(DownloadFile_Test)
		{
			HttpDownloader downloader;
			std::string source = downloader.DownloadFile("http://shelek.com/view/pvo/98");

			Assert::IsTrue(source.length() > 1024);	// Страница довольно большая.
			Assert::IsTrue(source.find("<html>") != std::string::npos);
		}

		TEST_METHOD(DownloadPageWithDependencies_Test)
		{
			HttpDownloader downloader;
			downloader.AddDependencyRecognizer(tags::link);
			downloader.AddDependencyRecognizer(tags::script);
			downloader.AddDependencyRecognizer(tags::img);

			std::filesystem::remove("../page.html");
			std::filesystem::remove_all("../page");
			downloader.DownloadPageWithDependencies("..", "page", "http://shelek.com/view/theory/151");

			Assert::IsTrue(std::filesystem::exists("../page.html"));
			Assert::IsTrue(std::filesystem::exists("../page"));
			Assert::IsTrue(std::filesystem::exists("../page/ajax_on.js"));
			Assert::IsTrue(std::filesystem::exists("../page/modal.css"));
			Assert::IsTrue(std::filesystem::exists("../page/animate.min.css"));
		}
	};
}
