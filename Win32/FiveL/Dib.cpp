//////////////////////////////////////////////////////////////////////////////
//
//   (c) Copyright 1999, Trustees of Dartmouth College, All rights reserved.
//        Interactive Media Lab, Dartmouth Medical School
//
//			$Author$
//          $Date$
//          $Revision$
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Dib.cpp : 
//

#include "stdafx.h"
#include "TCommon.h"
#include "LUtil.h"
#include "Globals.h"
#include "Dib.h"

Dib::Dib()
{
	m_dib = NULL;
	m_dib_info = NULL;
	m_dib_bits = NULL;
}

Dib::Dib(HDIB theDib)
{	
	m_dib = NULL;
	m_dib_info = NULL;
	m_dib_bits = NULL;
	
	Create(theDib);
}

Dib::~Dib()
{
	if (m_dib != NULL)
		free (m_dib);
}

void Dib::Create(HANDLE theDib)
{ 
	if (theDib == NULL)
		return;
		
	m_dib = theDib;
	m_dib_info = DibInfoHeaderPtr(m_dib);
	m_dib_bits = DibBitsPtr(m_dib);

	//m_dib_info = (LPBITMAPINFO) ::GlobalLock(m_dib);
	//m_dib_bits = FindDIBBits();

//#ifdef DEBUG
//	// dump a bunch of info about the DIB
//	gDebugLog.Log(DEBUG_GRAPHICS, "Created DIB");
//
//	gDebugLog.Log(DEBUG_GRAPHICS, "Height <%d>, Width <%d>, Bit depth <%d>",
//		Height(), Width(), BitCount());
//	gDebugLog.Log(DEBUG_GRAPHICS, "Size <%d>, Palette size <%d>",
//		Size(), PaletteSize());
//#endif	
}


HPALETTE Dib::GetPalette(void)
{
	return (DibPalDibTable(m_dib));
}

COLORREF Dib::GetPaletteEntry(int32 index)
{
	RGBQUAD		rgbVal;
	COLORREF	colorRef = 0x00FFFFFF;	// white by default

	if (DibGetColor(m_dib, index, &rgbVal))
		colorRef = RGB(rgbVal.rgbRed, rgbVal.rgbGreen, rgbVal.rgbBlue);

	return (colorRef);
}

long Dib::PaletteSize(void)
{
	return (DibColorSize(m_dib));
}
		
/*
 $Log$
 Revision 1.1  2001/09/24 15:11:00  tvw
 FiveL v3.00 Build 10

 First commit of /iml/FiveL/Release branch.

 There are now seperate branches for development and release
 codebases.

 Development - /iml/FiveL/Dev
 Release - /iml/FiveL/Release

 Revision 1.3  2000/04/07 17:05:15  chuck
 v 2.01 build 1

 Revision 1.2  1999/09/24 19:57:18  chuck
 Initial revision

*/
