#pragma once
#include <string_view>

namespace task
{
	// ----------------------------------------------------------------------------------------------------------------
	struct TagRecognizer
	{
		// Находит, например, <script type="text/javascript" src="js/jquery.min.js"> среди текста.
		std::string outerPattern;
		// Находит, например, src="js/jquery.min.js" среди <script type="text/javascript" src="js/jquery.min.js">.
		std::string innerPattern;
	};

	// ----------------------------------------------------------------------------------------------------------------
	namespace tags
	{
		inline TagRecognizer link   { R"(<link .*?\/>)", R"(href\s*=\s*((".+?")|('.+?')))" };
		inline TagRecognizer script { R"(<script .*?>)", R"(src\s*=\s*((".+?")|('.+?')))" };
		inline TagRecognizer img    { R"(<img .*?>)",    R"(src\s*=\s*((".+?")|('.+?')))" };
	}

	// ----------------------------------------------------------------------------------------------------------------
}