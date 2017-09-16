/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __gLTFImportConstants_H__
#define __gLTFImportConstants_H__

#include <string>
#include <sstream>
#include <fstream>

static const std::string TAB = "  ";
static const std::string TABx2 = "    ";
static const std::string TABx3 = "      ";
static const std::string TABx4 = "        ";
static const std::string TABx5 = "          ";

static std::string gFileExtension = "";
static std::string gJsonString = "";

//---------------------------------------------------------------------
struct Devnull : std::ostream, std::streambuf
{
	Devnull() : std::ostream(this) {}
};

// (un)comment first/second line to enable/disable debug to screen
#define OUT std::cout
//#define OUT devnull

//---------------------------------------------------------------------
static const std::string& getFileExtension (const std::string& fileName)
{
	std::string::size_type idx;
	idx = fileName.rfind('.');

	if (idx != std::string::npos)
	{
		gFileExtension = fileName.substr(idx + 1);
	}

	return gFileExtension;
}

//---------------------------------------------------------------------
static void copyFile (const std::string& fileNameSource, std::string& fileNameDestination)
{
	std::ifstream src(fileNameSource.c_str(), std::ios::binary);
	std::ofstream dst(fileNameDestination.c_str(), std::ios::binary);
	dst << src.rdbuf();
	dst.close();
	src.close();
}

//---------------------------------------------------------------------
static const std::string& getJsonAsString (const std::string& jsonFileName)
{
	std::ostringstream sstream;
	std::ifstream fs(jsonFileName);
	sstream << fs.rdbuf();
	gJsonString = sstream.str();
	return gJsonString;
}

#endif
