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
//  ImageTextRenderingEngine
//=========================================================================
//  A subclass of TextRenderingEngine which draws into an Image (for
//  testing purposes).



//=========================================================================
//  Test Program
//=========================================================================

Image *gImage;
FamilyDatabase *gFonts;

#define SYMBOL_FACE ("Standard Symbols L")
#define DINGBAT_FACE ("Dingbats")

void show(const wchar_t *inText, const std::string &inFont, int inSize,
	  Point inPos, Distance inLength, Justification inJustification)
{
    // Build our face stack.
    Face face     = gFonts->GetFace(inFont,       kRegularFaceStyle, inSize);
    Face symbol   = gFonts->GetFace(SYMBOL_FACE,  kRegularFaceStyle, inSize);
    Face dingbats = gFonts->GetFace(DINGBAT_FACE, kRegularFaceStyle, inSize);
    FaceStack stack(&face);
    stack.AddSecondaryFace(&symbol);
    stack.AddSecondaryFace(&dingbats);

    // Draw our text.
    TextRenderingEngine engine(inText, inText + wcslen(inText), &stack,
			       inPos, inLength, inJustification, gImage);
    engine.RenderText();
}

int main (int argc, char **argv) {
    try {
	// Our resources are located relative to our parent directory.
	FileSystem::SetBaseDirectory(FileSystem::Path().AddParentComponent());

	// Allocate an image for our output.
	PngImage image(640, 480);
	gImage = &image;

	// Load our FamilyDatabase.
	gFonts = new FamilyDatabase();
	gFonts->ReadFromFontDirectory();

	// Dump all entries in the specified font.
#if 0
	Face sym = gFonts->GetFace("Dingbats", kRegularFaceStyle, 24);
	for (unsigned int code = 0; code <= 0xFFFF; code++)
	    if (sym.GetGlyphIndex(code))
		printf("0x%X\n", code);
	exit(0);
#endif

	// Display a title.
	show(L"Font Engine Demo", "Bitstream Charter", 36,
	     Point(10, 50), 620, kCenterJustification);

	// Display some text samples.
	for (int fi = 0; fonts[fi] != NULL; fi++) {
	    wchar_t *str =
		wcsdup(L"The quick brown fox jumped over the lazy dog. DdT.");
	    str[wcslen(str)-4] = 0x2206;
	    str[wcslen(str)-3] = 0x03B4;
	    show(str, fonts[fi], 14, Point(10, 100 + fi * 20), 360,
		 kLeftJustification);
	}

	show(L"Font Drawing Demo (Fun, Fun!)", "Bitstream Charter", 30,
	     Point(10, 300), 360, kCenterJustification);

	wchar_t symbols[5];
	symbols[0] = 0x2206;
	symbols[1] = 0x03B4;
	symbols[2] = 0x2022;
	symbols[3] = L'a';
	symbols[4] = 0x0000;
	show(symbols, "Bitstream Charter", 50,
	     Point(10, 400), 360, kLeftJustification);

	Justification justifications[] =
	    {kLeftJustification, kRightJustification, kCenterJustification};
	for (int i = 0; i < 3; i++) {
	    show(L"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, "
		 "sed diam nonumy eirmod tempor invidunt ut labore et dolore "
		 "magna aliquyam erat, sed diam voluptua. At vero eos et "
		 "accusam et justo duo dolores et ea rebum.",
		 "Bitstream Charter", 12,
		 Point(370, 100 + i * 100), 260, justifications[i]);
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
