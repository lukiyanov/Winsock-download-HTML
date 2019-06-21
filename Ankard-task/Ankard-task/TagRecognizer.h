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
		inline TagRecognizer link   { R"(<link (.|\r|\n)*?\/>)", R"(...)" };
		inline TagRecognizer script { R"(<script (.|\r|\n)*?>)", R"(...)" };
		inline TagRecognizer img    { R"(<img (.|\r|\n)*?>)",    R"(...)" };
	}
}