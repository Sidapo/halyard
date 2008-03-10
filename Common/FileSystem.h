// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*-
// @BEGIN_LICENSE
//
// Halyard - Multimedia authoring and playback system
// Copyright 1993-2008 Trustees of Dartmouth College
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

#ifndef FileSystem_H
#define FileSystem_H

#include "TException.h"

//////////
/// Portable interface to various filesystem functions.  Over time,
/// it might be good to refactor more code into this library.
///
namespace FileSystem {

	//////////
	/// A file-system related error. 
	/// TODO - Refactor and combine with Typography::Error?
	///
	class Error : public FIVEL_NS TException {
	public:
		Error(const char *inErrorFile, int inErrorLine, int inErrorCode);
		Error(const char *inErrorFile, int inErrorLine,
			  const std::string &inErrorMessage)
			: TException(inErrorFile, inErrorLine, inErrorMessage) {}
		
		virtual const char *GetClassName() const
		    { return "FileSystem::Error"; }
	};
	
	//////////
	/// A portable representation of a pathname.  Not yet complete.
	///
	class Path {
		std::string mPath;

	public:
		//////////
		/// Create a path to the current working directory.
		///
		Path();

		//////////
		/// Create a path from a string.
		/// TODO - Specify string format.
		/// TODO - Check for trailing directory separator.
		/// 
		Path(const std::string &inPath);
		
		//////////
		/// Return the extension of the last element of the path, if
		/// it appears to have one.  Otherwise, return "".
		///
		std::string GetExtension() const;

		//////////
		/// Change the extension of a path, or add one if none exists.
		///
		/// \param inNewExtension  The extension to use.
		/// \return  A path with the new extension.
		///
		Path ReplaceExtension(std::string inNewExtension) const;

		//////////
		/// Does this path point to an actual object on disk?
		///
		bool DoesExist() const;

		//////////
		/// Does this path point to a file on disk?
		///
		bool IsRegularFile() const;

		//////////
		/// Does this path point to a directory on disk?
		///
		bool IsDirectory() const;

		//////////
		/// Read in all the entries from a directory.
		///
		std::list<std::string> GetDirectoryEntries() const;

		//////////
		/// Delete the file pointed to by this path.  (If we name this
		/// "DeleteFile", it gets clobbered by a #define in windows.h.)
		///
		void RemoveFile() const;

		//////////
		/// Add a component to the end of the path (as though the
		/// the path were a directory).
		///
		/// \param inFileName  The component to add to the path.
		/// \return  The path plus the new component.
		///
		Path AddComponent(const std::string &inFileName) const;

		//////////
		/// Add a "parent directory" component to the end of
		/// the path.  This may have exciting behavior in the
		/// presence of symlinks.
		/// 
		Path AddParentComponent() const;
		
		//////////
		/// Convert a Path object into a local path string.  This isn't
		/// entirely correct on the Macintosh (where two files can have the
		/// same pathname if two hard drives have the same name), but we
		/// need it to interface to third-party libraries that assume paths
		/// are unique.
		///
		std::string ToNativePathString () const;

		//////////
		/// Rename this file to 'inNewName'.
		///
		void RenameFile(const Path &inNewName) const;

		//////////
		/// Replace this file with 'inTemporaryFile'.  On some platforms,
		/// it's possible to implement this as an atomic operation.
		///
		void ReplaceWithTemporaryFile(const Path &inTemporaryFile) const;

		//////////
		/// Create an empty file with the specified MIME type.  (The type
		/// is only used on operating systems which support file types.)
		///
		void CreateWithMimeType(const std::string &inMimeType);

		//////////
		/// Compare two paths for equality.  This is a dumb test--it doesn't
		/// know anything about simplifying paths, and it doesn't look at
		/// the disk.  It just does a memberwise comparison.  It isn't
		/// good for much besides the test suites, actually.
		///
		friend bool operator==(const Path& inLeft, const Path& inRight);

		//////////
		/// Create a path from an absolute native pathname string.
		/// DO NOT PASS A RELATIVE PATH STRING.  The quality of the
		/// Path returned by this function is directly related to
		/// the quality of the input path.
		///
		static Path NativePath(const std::string &inPath);
	};

	//////////
	/// Set the base directory for the application.  Defaults to
	/// the current working directory.  Used by GetFontDirectory,
	/// ResolveFontPath, etc.
	///
	Path SetBaseDirectory(const Path &inDirectory);

	//////////
	/// Set the base directory for the application using a native
	/// path name.  Any trailing directory separator will be removed.
	///
	Path SetBaseDirectory(const std::string &inDirectory);

	//////////
	/// Return the current base directory for this application.
	///
	Path GetBaseDirectory();

    //////////
    /// Set the directory to use for log files and other application
    /// data.
    ///
    void SetAppDataDirectory(const std::string &inDirectory);

    //////////
    /// Get the directory to use for log files and other application
    /// data.
    ///
    Path GetAppDataDirectory();

    //////////
    /// Set the directory to use for large, machine-specific files which
    /// change.  This is different from the AppDataDirectory largely
    /// because we don't want to store huge files in people's roaming
    /// Windows profiles.
    ///
    void SetAppLocalDataDirectory(const std::string &inDirectory);

    //////////
    /// Get the directory to use for large, machine-specific files which
    /// change.
    ///
    Path GetAppLocalDataDirectory();

    //////////
    /// Set the name of the data directories we'll use for this script.
    /// Note that this is just the basename of the directory itself, not
    /// the full path.
    ///
    void SetScriptDataDirectoryName(const std::string &inName);

    //////////
    /// Get the directory which we should use to store per-script data.
    ///
    Path GetScriptDataDirectory();

    //////////
    /// Get the directory which we should use to store large, per-script,
    /// per-machine data, such as files downloaded by the updater.
    ///
    Path GetScriptLocalDataDirectory();

	//////////
	/// Get the directory 5L uses to store fonts.  (Eventually there
	/// will be more of these functions, and we might combine them
	/// into one function with a string argument.)
	///
	inline Path GetFontDirectory()
	    { return GetBaseDirectory().AddComponent("Fonts"); }

	//////////
	/// We refer to fonts (and directories containing fonts) using
	/// Unix-style relative paths.  The "Fonts" directory is represented as
	/// "", a directory within it is represented as "dirname", and a file
	/// /// within the directory is represented as "dirname/font.tff".
	/// This /// slightly odd interface is a placeholder to allow recursive
	/// font directory scanning *before* we finish migrating fully to
	/// boost::filesystem.
	///
	Path ResolveFontPath(const std::string &inRelPath);

    //////////
    /// Set the name of the scripts directory that we're loading from.
    /// This is used by the test suites to load scripts from "TestScripts"
    /// instead of the usual "Scripts" directory.
    ///
    void SetScriptsDirectoryName(const std::string &inName);

    //////////
    /// Get the name of the scripts directory that we're loading from.
    /// Please call GetScriptsDirectory instead if at all possible.
    ///
    std::string GetScriptsDirectoryName();

	//////////
	/// Get the directory 5L uses to store scripts.
	///
	Path GetScriptsDirectory();

	//////////
	/// Get the path to a specific script file.
	///
	inline Path GetScriptFilePath(const std::string &inScriptFileName)
	    { return GetScriptsDirectory().AddComponent(inScriptFileName); }

	//////////
	/// Get the directory 5L uses to store runtime support files.
	///
	inline Path GetRuntimeDirectory()
	    { return GetBaseDirectory().AddComponent("Runtime"); }

	//////////
	/// Get the path to a specific runtime file.
	///
	inline Path GetRuntimeFilePath(const std::string &inRuntimeFileName)
	    { return GetRuntimeDirectory().AddComponent(inRuntimeFileName); }

	//////////
	/// Get the directory which contains graphics.
    //
	inline Path GetGraphicDirectory()
	    { return GetBaseDirectory().AddComponent("Graphics"); }

	//////////
	/// Get the directory which contains script graphics--ones which we
	/// will use as chrome for various parts of the engine UI,
	/// including the icon and the splash screen.
    //
	inline Path GetScriptGraphicDirectory()
	    { return GetGraphicDirectory().AddComponent("script"); }

	//////////
	/// Get the path to a "script" graphic.  See GetScriptGraphicDirectory.
	///
	inline Path GetScriptGraphicFilePath(const std::string &inFileName)
	    { return GetScriptGraphicDirectory().AddComponent(inFileName); }

	//////////
	/// Get the directory 5L uses to store user data.
	///
	inline Path GetDataDirectory()
	    { return GetBaseDirectory().AddComponent("Data"); }

	//////////
	/// Given a file name, return a path pointing to a file with
	/// that name in the font directory.
	///
	inline Path GetDataFilePath(const std::string &inDataFileName)
	    { return GetDataDirectory().AddComponent(inDataFileName); }

	//////////
	/// Get the directory 5L uses to store palette information.
	///
	inline Path GetPalettesDirectory()
	    { return GetBaseDirectory().AddComponent("Palettes"); }

	// TODO - Factor out more platform-specific config to use
	// the directories listed above.

	//////////
	/// Checks existence of file and directory and displays specific
	/// error message if non-existent.
	///
	/// \param inPath  The path to check on.
	/// \param inShouldBeDir  True if the path refers to a directory, false if
	///                       path refers to a file
	///
	void ExistenceCheck(const Path &inPath, const bool &isDir);
}

#endif // FileSystem_H
