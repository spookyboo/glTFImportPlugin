/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __gLTFImportExecutor_H__
#define __gLTFImportExecutor_H__

#include <map>
#include "hlms_editor_plugin.h"
#include "gLTFImportConstants.h"
#include "gLTFMaterial.h"
#include "gLTFImportMaterialsParser.h"
#include "gLTFImportTexturesParser.h"
#include "gLTFImportImagesParser.h"
#include "gLTFImportPbsMaterialsCreator.h"
#include "rapidjson/document.h"

/** Class responsible for executing the import */
class gLTFImportExecutor
{
	public:
		gLTFImportExecutor (void) {};
		virtual ~gLTFImportExecutor(void) {};
		
		// Perform the import (called by plugin)
		bool executeImport(Ogre::HlmsEditorPluginData* data);

	protected:
		// Process the binary file / text file
		bool executeBinary (const std::string& fileName, Ogre::HlmsEditorPluginData* data); // proces .glb (binary) file
		bool executeText (const std::string& fileName, Ogre::HlmsEditorPluginData* data); // proces .gltf (json text) file


	private:
		std::map<std::string, gLTFMaterial> mMaterialsMap;
		std::map<std::string, gLTFTexture> mTexturesMap;
		std::map<std::string, gLTFImage> mImagesMap;
		gLTFImportMaterialsParser mMaterialsParser;
		gLTFImportTexturesParser mTexturesParser;
		gLTFImportImagesParser mImagesParser;
		gLTFImportPbsMaterialsCreator mPbsMaterialsCreator;
};

#endif
