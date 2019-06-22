#pragma once
#include <string_view>

namespace task
{
	struct TagRecognizer
	{
		std::string outerPattern;	// �������, �������� <script type="text/javascript" src="js/jquery.min.js"> ����� ������.
		std::string innerPattern;	// �������, �������� src="js/jquery.min.js" ����� <script type="text/javascript" src="js/jquery.min.js">.
	};

	namespace tags
	{
		inline TagRecognizer link   { R"(<link .*?\/>)", R"(href\s*=\s*((".+?")|('.+?')))" };
		inline TagRecognizer script { R"(<script .*?>)", R"(src\s*=\s*((".+?")|('.+?')))" };
		inline TagRecognizer img    { R"(<img .*?>)",    R"(src\s*=\s*((".+?")|('.+?')))" };
	}
}