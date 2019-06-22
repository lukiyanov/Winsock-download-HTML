#pragma once
#include <string_view>

namespace task
{
	struct TagRecognizer
	{
		std::string outerPattern;	// Находит, например <script type="text/javascript" src="js/jquery.min.js"> среди текста.
		std::string innerPattern;	// Находит, например src="js/jquery.min.js" среди <script type="text/javascript" src="js/jquery.min.js">.
	};

	namespace tags
	{
		inline TagRecognizer link   { R"(<link .*?\/>)", R"(href\s*=\s*((".+?")|('.+?')))" };
		inline TagRecognizer script { R"(<script .*?>)", R"(src\s*=\s*((".+?")|('.+?')))" };
		inline TagRecognizer img    { R"(<img .*?>)",    R"(src\s*=\s*((".+?")|('.+?')))" };
	}
}