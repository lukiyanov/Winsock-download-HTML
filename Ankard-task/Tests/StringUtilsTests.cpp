#include "pch.h"
#include <CppUnitTest.h>

// Тестируемый класс и необходимые для этого зависимости.
#include <StringUtils.h>
#include <Exceptions.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace task;

namespace Tests
{
	TEST_CLASS(CmdLineTests)
	{
	public:
		TEST_METHOD(ToLower_Test)
		{
			using std::string;
			Assert::AreEqual(string("asdsdf*3_g"),  ToLower("asdsdf*3_g"));
			Assert::AreEqual(string("asds df*3_g"), ToLower("AsDs DF*3_G"));
			Assert::AreEqual(string(""),            ToLower(""));
		}

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
	};
}
