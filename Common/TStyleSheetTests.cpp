// -*- Mode: C++; tab-width: 4; -*-

#include "ImlUnit.h"
#include "TStyleSheet.h"
#include "TParser.h"

USING_NAMESPACE_FIVEL

using namespace Typography;

extern void test_TStyleSheet(void);

static void test_style(const StyledText &inText, int inBegin, int inEnd,
					   const Typography::Style &inDesiredStyle)
{
    StyledText::const_iterator current = inText.begin() + inBegin;
    StyledText::const_iterator end = inText.begin() + inEnd;
    for (; current != end; ++current)
		TEST(*current->style == inDesiredStyle);
}

void test_TStyleSheet(void)
{
    // Install support for top-level forms of type "defstyle".
    TStyleSheetManager style_manager;
    TParser::RegisterIndexManager("defstyle", &style_manager);

    // Parse our index file and get our style sheet.
    gIndexFileManager.NewIndex("defstyle");
	TBNode *node = style_manager.Find("S1");
	TEST(node != NULL);

    // Set up a style.
    TStyleSheet *style1 = dynamic_cast<TStyleSheet*>(node);
    StyledText text1 = style1->MakeStyledText(" foo |bar| ^baz^ @wub@ || "
											  "|foo@bar@| \\n\\t\\^\\@\\|\\\\");
    TEST(*text1.GetText() == L" foo bar baz wub  foobar \n\t^@|\\");

    // Build something to compare it against.
    Typography::Style base_style("Times", 12);
	base_style.SetFaceStyle(kShadowFaceStyle);
    base_style.SetLeading(-1);
	base_style.SetShadowOffset(2);
    base_style.SetColor(Color(0xFF, 0xFF, 0xFF, 0x00));
    base_style.SetShadowColor(Color(0x00, 0x00, 0x00, 0x80));
    Typography::Style highlight_style(base_style);
    highlight_style.SetColor(Color(0x00, 0xFF, 0x00, 0x00));
    highlight_style.SetShadowColor(Color(0x00, 0xFF, 0x00, 0x80));
        
    // Make sure each of our ranges is correct.
    test_style(text1, 0, 5, base_style);
    test_style(text1, 5, 8,
			   Typography::Style(base_style).ToggleFaceStyle(kItalicFaceStyle));
    test_style(text1, 8, 9, base_style);
    test_style(text1, 9, 10, highlight_style);
    test_style(text1, 12, 13, base_style);
    test_style(text1, 13, 16,
			   Typography::Style(base_style).ToggleFaceStyle(kBoldFaceStyle));
    test_style(text1, 16, 18, base_style);
    test_style(text1, 18, 21,
			   Typography::Style(base_style).ToggleFaceStyle(kItalicFaceStyle));
    test_style(text1, 21, 24,
			   Typography::Style(base_style).ToggleFaceStyle(kBoldItalicFaceStyle));
}
