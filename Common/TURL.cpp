//////////////////////////////////////////////////////////////////////////////
//
//   (c) Copyright 1999,2000 Trustees of Dartmouth College, All rights reserved.
//        Interactive Media Lab, Dartmouth Medical School
//
//			$Author$
//          $Date$
//          $Revision$
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// TURL.cpp : 
//

#include "THeader.h"
#include "TURL.h"

TURL::TURL() : TString()
{
}

TURL::TURL(const TString &inStr) : TString(inStr)
{
	CheckSyntax();
}

TURL::~TURL()
{
}

TURL &TURL::operator=(const TString &inStr)
{
	TString	*tmpStr = this;

	*tmpStr = inStr;
	CheckSyntax();

	return (*this);
}

TURL &TURL::operator+=(const TString &inStr)
{
	TString *tmpStr = this;

	*tmpStr += inStr;
	CheckSyntax();

	return (*this);
}

TString TURL::GetServerPart()
{
	TString		retStr;

	if (m_HaveGoodURL)
		retStr = m_ServerPart;

	return (retStr);
}

TString TURL::GetDirPart()
{
	TString		retStr;

	if (m_HaveGoodURL)
		retStr = m_DirPart;

	return (retStr);
}

//
//	GetFirstDir - Strip the first directory off the directory part.
//
TString TURL::GetFirstDir()
{
	TString		firstDir;

	if (not m_HaveGoodURL)
		return (firstDir);

	int32	firstPos;

	firstPos = Find('/');
	if (firstPos == -1)
		return (firstDir);
	firstPos++;

	int32	secondPos = Find('/', firstPos);

	if (secondPos == -1)
		firstDir = Mid(firstPos);
	else
		firstDir = Mid(firstPos, secondPos - firstPos);

	return (firstDir);
}

//
//	CheckSyntax - Check the syntax of the URL.
//
bool TURL::CheckSyntax()
{
	int32	pos = 0;

	m_HaveGoodURL = false;
	m_ServerPart.Empty();
	m_DirPart.Empty();
	m_Type = UnknownURL;

	// trim off leading and trailing spaces
	LTrim();
	RTrim();

	// check the service part
	if (StartsWith("http://"))
	{
		m_Type = HttpURL;
		pos = 7;
	}
	else if (StartsWith("ftp://"))
	{
		m_Type = FtpURL;
		pos = 6;
	}
	else if (StartsWith("file://"))
	{
		m_Type = FileURL;
		pos = 7;
	}
	else if (StartsWith("rtsp://"))
	{
		m_Type = RtspURL;
		pos = 7;
	}
	else
		return (false);

	// check the server part
	int32	pos2;

	pos2 = Find('/', pos);
	if (pos2 != -1)
	{
		if (pos2 != pos)
		{
			// grab the server part
			m_ServerPart = Mid(pos, pos2 - pos);
		}
		else
		{
			// no server part, only legal for file://
			if (m_Type != FileURL)
				return (false);
		}
	}
	else
	{
		// error, not enough slashes

		return (false);
	}

	// check the directory part, we just need something
	if (pos2 < (int32) Length())
		m_DirPart = Mid(pos2);	// grab the initial / too
	else
	{
		// error, no directory part

		return (false);
	}
	
	m_HaveGoodURL = true;
	return (true);
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

 Revision 1.1  2000/04/06 17:06:10  chuck
 Initial check-in

*/
