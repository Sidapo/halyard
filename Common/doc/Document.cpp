// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Tamale - Multimedia authoring and playback system
// Copyright 1993-2004 Trustees of Dartmouth College
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// @END_LICENSE

#include "CommonHeaders.h"
#include "TInterpreter.h"
#include "FileSystem.h"

#include "Document.h"
#include "TamaleProgram.h"
#include "Background.h"

USING_NAMESPACE_FIVEL
using namespace model;
using FileSystem::Path;


//=========================================================================
//  Document Layout
//=========================================================================
//  A document currently has the following directory structure:
//
//    MyDocument/
//      Runtime/   - Program-independent Scheme runtime files
//      Fonts/     - Program-independent fonts
//      Script/    - Program-specific scripts
//      baseq2/    - Program-specific Quake 2 data (optional)
//      Graphics/  - Program-specific graphics
//      Media/     - Program-specific audio/video (optional)
//      Data/      - Saved user data
//      data.tam   - Saved XML data
//
//  This will change as we revamp the engine.  For now, we can only open
//  one document per application.  This should also change.


//=========================================================================
//  Class Registrations
//=========================================================================
//  In theory, we shouldn't need to do this, but there are MSVC linker
//  bugs which make it a good idea.  See BEGIN_MODEL_CLASSES in Model.h
//  for a longer explanation.

BEGIN_MODEL_CLASSES()
	REGISTER_MODEL_CLASS(TamaleProgram)
	REGISTER_MODEL_CLASS(Background)
END_MODEL_CLASSES()


//=========================================================================
//  Document Format
//=========================================================================

enum {
    CURRENT_FORMAT = 0,
    COMPATIBLE_BACK_TO = 0,
    EARLIEST_READABLE = 0
};

const static ModelFormat gTamaleFormat("TamaleProgram", CURRENT_FORMAT,
									   COMPATIBLE_BACK_TO);


//=========================================================================
//  Document Methods
//=========================================================================

std::string Document::SetBaseAndGetFilePath(const std::string &inDirectory)
{
	FileSystem::Path path =
		FileSystem::SetBaseDirectory(inDirectory).AddComponent("data.tam");
	return path.ToNativePathString();
}

void Document::CheckStructure()
{
	// Sanity-check our directory structure.
	Path base = FileSystem::GetBaseDirectory();
	Path runtime = FileSystem::GetRuntimeDirectory();
	CheckDirectory(runtime);
	CheckDirectory(runtime.AddComponent("5L"));
	CheckFile(runtime.AddComponent("5L").AddComponent("loader.ss"));
	CheckDirectory(base.AddComponent("Fonts"));
	CheckDirectory(base.AddComponent("Scripts"));
	CheckFile(base.AddComponent("Scripts").AddComponent("start.ss"));
	CheckDirectory(base.AddComponent("Graphics"));
}

void Document::CheckDirectory(Path inPath)
{
	CHECK(inPath.DoesExist() && inPath.IsDirectory(),
		  ("Cannot find directory " + inPath.ToNativePathString() +
		   " in Tamale program").c_str());
}

void Document::CheckFile(Path inPath)
{
	CHECK(inPath.DoesExist() && inPath.IsRegularFile(),
		  ("Cannot find file " + inPath.ToNativePathString() +
		   " in Tamale program").c_str());
}

Document::Document(const std::string &inDirectory)
    : Model(gTamaleFormat)
{
	std::string data_file = SetBaseAndGetFilePath(inDirectory);
	CheckStructure();
    SaveAs(data_file.c_str());
	TInterpreterManager::GetInstance()->BeginScript();
}

Document::Document(const std::string &inDirectory, Flag inOpen)
    : Model(gTamaleFormat, EARLIEST_READABLE,
			SetBaseAndGetFilePath(inDirectory).c_str())
{
	CheckStructure();
	TInterpreterManager::GetInstance()->BeginScript();    
}

Document::~Document()
{
    
}

TamaleProgram *Document::GetTamaleProgram()
{
	return cast<TamaleProgram>(GetRoot());
}
