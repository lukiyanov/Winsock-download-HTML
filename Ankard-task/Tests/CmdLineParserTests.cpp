#include "pch.h"
#include <CppUnitTest.h>

// Тестируемый класс и необходимые для этого зависимости.
#include <CmdLineParser.h>
#include <Exceptions.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace task;
using namespace std;

namespace Tests
{
	TEST_CLASS(CmdLineParserTests)
	{
	public:
		TEST_METHOD(CmdLineParser_Test)
		{
			Assert::ExpectException<CommandLineException>([]() {
				CmdLineParser parser(-99, nullptr);
			});

			Assert::ExpectException<CommandLineException>([]() {
				CmdLineParser parser(1, nullptr);
			});

			Assert::ExpectException<CommandLineException>([]() {
				// Мало аргументов
				char* argv[] = { "file.exe", "123" };
				CmdLineParser parser(2, argv);
			});

			Assert::ExpectException<CommandLineException>([]() {
				// Много аргументов
				char* argv[] = { "file.exe", "123", "456", "789" };
				CmdLineParser parser(4, argv);
			});

			// Корректный вариант: никаких исключений.
			char* argv[] = { "file.exe", "123", "456" };
			CmdLineParser parser(3, argv);

			Assert::AreEqual(string("123"), parser.GetUrl());
			Assert::AreEqual(string("456"), parser.GetDirectory());
		}
	};
}
