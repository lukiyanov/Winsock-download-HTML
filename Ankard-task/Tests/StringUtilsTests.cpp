#include "pch.h"
#include <CppUnitTest.h>

// Тестируемый файл и необходимые для этого зависимости.
#include <StringUtils.h>
#include <Exceptions.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace task;

namespace Tests
{
	TEST_CLASS(StringUtilsTests)
	{
	public:

		// ------------------------------------------------------------------------------------------------------------
		TEST_METHOD(ToLower_Test)
		{
			using std::string;
			Assert::AreEqual(string("asdsdf*3_g"),  ToLower("asdsdf*3_g"));
			Assert::AreEqual(string("asds df*3_g"), ToLower("AsDs DF*3_G"));
			Assert::AreEqual(string(""),            ToLower(""));
		}

		// ------------------------------------------------------------------------------------------------------------
		TEST_METHOD(GetHttpHostNameByUrl_Test)
		{
			using std::string_view;

			Assert::AreEqual(string_view("12.34.5.7"), GetHttpHostNameByUrl("hTTp://12.34.5.7/sdf/sdf.html"));
			Assert::AreEqual(string_view("sefsd_sdf"), GetHttpHostNameByUrl("http://sefsd_sdf/sdf/sdf.html"));
			Assert::AreEqual(string_view("sefSd_sDf"), GetHttpHostNameByUrl("http://sefSd_sDf/"));
			Assert::AreEqual(string_view("sefSd_sDf"), GetHttpHostNameByUrl("http://sefSd_sDf"));

			Assert::ExpectException<InvalidUrlFormat>([]() {
				GetHttpHostNameByUrl("https://sefSd_sDf/");
			});
		}

		// ------------------------------------------------------------------------------------------------------------
		TEST_METHOD(GetAbsoluteHttpPath_Test)
		{
			using std::string;
			Assert::AreEqual(string("http://host.ru/asd"), GetAbsoluteHttpPath("asd",          "host.ru"));	// Относительный -> абсолютный
			Assert::AreEqual(string("http://asdf"),        GetAbsoluteHttpPath("http://asdf",  "host.ru"));	// Корректный -> не трогаем
			Assert::AreEqual(string("http://asdf"),        GetAbsoluteHttpPath("https://asdf", "host.ru")); // HTTPS -> HTTP
			Assert::AreEqual(string(""),			       GetAbsoluteHttpPath("tln://asdf",   "host.ru")); // Другой протокол -> пропуск
		}

		// ------------------------------------------------------------------------------------------------------------
		TEST_METHOD(GetFileName_Test)
		{
			using std::string;
			Assert::AreEqual(string(""), GetFileName(""));
			Assert::AreEqual(string("file.css"),     GetFileName("file.css"));
			Assert::AreEqual(string("file.css"),     GetFileName("asdf/asdr/file.css"));
			Assert::AreEqual(string("file.css.txt"), GetFileName("http://qwerty.com/asdf/asdr/file.css.txt"));
		}

		// ------------------------------------------------------------------------------------------------------------
		TEST_METHOD(ExtractPatternsFromSource_Test)
		{
			using std::string;

			{
				std::list<TagRecognizer> tagList;
				tagList.push_back(tags::link);
				tagList.push_back(tags::img);
				tagList.push_back(tags::script);
				auto res = ExtractPatternsFromSource(
					"efsdfdsfsdfdsf<img sdfdsfd src =  'THIS'ssfds/>sdfdsfsd<link s3r23f href='that'2323d/>",
					tagList);
				Assert::AreEqual(size_t(2), res.size());
				Assert::AreEqual(string("THIS"), res.front());
				Assert::AreEqual(string("that"), res.back());
			}
		}

		// ------------------------------------------------------------------------------------------------------------
	};
}
