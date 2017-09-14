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
#include "gLTFImportMaterialsExecutor.h"
#include "gLTFImportTexturesExecutor.h"
#include "gLTFImportImagesExecutor.h"
#include "rapidjson/document.h"

/** Class responsible for executing the import */
class gLTFImportExecutor
{
	public:
		gLTFImportExecutor (void) :
			mDetailedDiffuseMapCount(0) {};
		virtual ~gLTFImportExecutor(void) {};
		
		// Perform the import (called by plugin)
		bool executeImport(Ogre::HlmsEditorPluginData* data);

	protected:
		// Process the binary file / text file
		bool executeBinary (const std::string& fileName, Ogre::HlmsEditorPluginData* data); // proces .glb (binary) file
		bool executeText (const std::string& fileName, Ogre::HlmsEditorPluginData* data); // proces .gltf (json text) file

		// Create the Ogre Pbs material files; add json blocks
		bool createOgrePbsMaterialFiles (Ogre::HlmsEditorPluginData* data); // Create *.material.json files and copy images
		bool createTransparencyJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createDiffuseJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createSpecularJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createMetalnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createFresnelJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createNormalJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createRoughnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createReflectionJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createDetailDiffuseJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createDetailNormalJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createDetailWeightJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createEmissiveJsonBlock (std::ofstream* dst, const gLTFMaterial& material);

	private:
		std::map<std::string, gLTFMaterial> mMaterialsMap;
		unsigned short mDetailedDiffuseMapCount;
		gLTFImportMaterialsExecutor mMaterialsExecutor;
		gLTFImportTexturesExecutor mTexturesExecutor;
		gLTFImportImagesExecutor mImagesExecutor;
};

#endif
