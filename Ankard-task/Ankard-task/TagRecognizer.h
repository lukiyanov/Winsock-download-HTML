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
		inline TagRecognizer link   { R"(<link (.|\r|\n)*?\/>)", R"(...)" };
		inline TagRecognizer script { R"(<script (.|\r|\n)*?>)", R"(...)" };
		inline TagRecognizer img    { R"(<img (.|\r|\n)*?>)",    R"(...)" };
	}
}