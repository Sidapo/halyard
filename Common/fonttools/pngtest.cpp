#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "Image.h"
#include "Typography.h"

using namespace Typography;

char *fonts[] = {
    "Nimbus Sans L",
    "Nimbus Roman No9 L",
    "URW Gothic L",
    "URW Bookman L",
    "Century Schoolbook L",
    "Bitstream Charter",
    "Times",
    NULL
};
	

//=========================================================================
//  Test Program
//=========================================================================

Image *gImage;
FamilyDatabase *gFonts;

#define SYMBOL_FACE ("Standard Symbols L")
#define DINGBAT_FACE ("Dingbats")

void show_with_style_info(const wchar_t *inText,
			  const StyleInformation &inStyleInfo,
			  Point inPos, Distance inLength,
			  Justification inJustification)
{
    // Draw our text.
    size_t len = wcslen(inText);
    StyledTextSpan span(StyledCharIterator(inText, inStyleInfo.begin()),
			StyledCharIterator(inText + len, inStyleInfo.end()));
    TextRenderingEngine engine(span, inPos, inLength, inJustification, gImage);
    engine.RenderText();
}

void show(const wchar_t *inText, const Style &inStyle,
	  Point inPos, Distance inLength, Justification inJustification)
{
    // Build our style run.
    size_t len = wcslen(inText);
    StyleInformation styleInfo(inStyle);
    styleInfo.EndStyleAt(len);

    // Draw our text.
    show_with_style_info(inText, styleInfo, inPos, inLength, inJustification);
}

int main (int argc, char **argv) {
    try {
	// Our resources are located relative to our parent directory.
	FileSystem::SetBaseDirectory(FileSystem::Path().AddParentComponent());

	// Allocate an image for our output.
	PngImage image(640, 480);
	gImage = &image;

#if 0
	// Dump all entries in the specified font.
	FamilyDatabase *db = FamilyDatabase::GetFamilyDatabase();
	Face sym = db->GetFace("Dingbats", kRegularFaceStyle, 24);
	for (unsigned int code = 0; code <= 0xFFFF; code++)
	    if (sym.GetGlyphIndex(code))
		printf("0x%X\n", code);
	exit(0);
#endif

	// Create our base style.
	Style baseStyle("Bitstream Charter", 12);
	std::list<std::string> backups;
	backups.push_back(SYMBOL_FACE);
	backups.push_back(DINGBAT_FACE);
	baseStyle.SetBackupFamilies(backups);
	baseStyle.SetColor(Color(0, 0, 96));

	// Display a title.
	show(L"Font Engine Demo", Style(baseStyle).SetSize(36),
	     Point(10, 50), 620, kCenterJustification);

	// Display some text samples.
	Style sampleStyle(baseStyle);
	sampleStyle.SetSize(14);
	for (int fi = 0; fonts[fi] != NULL; fi++) {
	    wchar_t *str =
		wcsdup(L"The quick brown fox jumped over the lazy dog. DdT.");
	    str[wcslen(str)-4] = 0x2206;
	    str[wcslen(str)-3] = 0x03B4;
	    show(str, Style(sampleStyle).SetFamily(fonts[fi]),
		 Point(10, 100 + fi * 20), 360, kLeftJustification);
	}

	// Do an elaborate, multi-styled paragraph.
	const wchar_t *para = (L"Ms. Matthews' case raises several "
			       "interesting questions.  RGB color is cool.  "
			       "We like several cheeses! T.");
	Style para_style(baseStyle);
	para_style.SetSize(14);
	Style bold_italic_para_style(para_style);
	bold_italic_para_style.SetFaceStyle(kBoldItalicFaceStyle);
	StyleInformation styleInfo(para_style);
	styleInfo.ChangeStyleAt(34, bold_italic_para_style);
	styleInfo.ChangeStyleAt(45, para_style);
	styleInfo.ChangeStyleAt(58,
				Style(para_style).SetColor(Color(255, 0, 0)));
	styleInfo.ChangeStyleAt(59,
				Style(para_style).SetColor(Color(0, 255, 0)));
	styleInfo.ChangeStyleAt(60,
				Style(para_style).SetColor(Color(0, 0, 255)));
	styleInfo.ChangeStyleAt(61, para_style);
	styleInfo.ChangeStyleAt(wcslen(para)-1,
				Style(para_style).SetColor(Color(255, 0, 0)));
	styleInfo.EndStyleAt(wcslen(para));
	show_with_style_info(para, styleInfo, Point(10, 300), 360,
			     kLeftJustification);

	wchar_t symbols[5];
	symbols[0] = 0x2206;
	symbols[1] = 0x03B4;
	symbols[2] = 0x2022;
	symbols[3] = L'a';
	symbols[4] = 0x0000;
	show(symbols, Style(baseStyle).SetSize(50),
	     Point(10, 400), 360, kLeftJustification);

	Justification justifications[] =
	    {kLeftJustification, kRightJustification, kCenterJustification};
	for (int i = 0; i < 3; i++) {
	    show(L"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, "
		 "sed diam nonumy eirmod tempor invidunt ut labore et dolore "
		 "magna aliquyam erat, sed diam voluptua. At vero eos et "
		 "accusam et justo duo dolores et ea rebum.",
		 baseStyle, Point(370, 100 + i * 100), 260, justifications[i]);
	}

        image.save("visual-test.png");
#ifdef HAVE_EYE_OF_GNOME
	system("eog visual-test.png");
#endif
    }
    catch (std::exception &error)
    {
	std::cerr << std::endl << error.what() << std::endl;
	return 1;
    }
    catch (...)
    {
	std::cerr << std::endl
		  << "An unknown exception occurred!" << std::endl;
	return 1;
    }
}
