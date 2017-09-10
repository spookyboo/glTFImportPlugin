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
#include "gLTFMaterial.h"
#include "rapidjson/document.h"

static const std::string TAB = "  ";
static const std::string TABx2 = "    ";
static const std::string TABx3 = "      ";
static const std::string TABx4 = "        ";

/** Class responsible for executing the import */
class gLTFImportExecutor
{
	public:
		gLTFImportExecutor(void) {};
		virtual ~gLTFImportExecutor(void) {};
		
		// Perform the import (called by plugin)
		bool executeImport(Ogre::HlmsEditorPluginData* data);

	protected:
		// Process the binary file / text file
		bool executeBinary (const std::string& fileName, Ogre::HlmsEditorPluginData* data); // proces .glb (binary) file
		bool executeText (const std::string& fileName, Ogre::HlmsEditorPluginData* data); // proces .gltf (json text) file

		// Create the Ogre Pbs material files
		bool createOgrePbsMaterialFiles (Ogre::HlmsEditorPluginData* data); // Create *.material.json file and copy images
		bool createDiffuseJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createSpecularJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createMetalnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createFresnelJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createNormalJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createRoughnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material);
		bool createReflectionJsonBlock (std::ofstream* dst, const gLTFMaterial& material);

		// Parse level 1
		bool parseMaterials (rapidjson::Value::ConstMemberIterator jsonIterator);
		bool parseTextures(rapidjson::Value::ConstMemberIterator jsonIterator);
		bool parseImages(rapidjson::Value::ConstMemberIterator jsonIterator);

		// Parse level 2
		PbrMetallicRoughness parsePbrMetallicRoughness (rapidjson::Value::ConstMemberIterator jsonIterator);
		NormalTexture parseNormalTexture(rapidjson::Value::ConstMemberIterator jsonIterator);
		OcclusionTexture parseOcclusionTexture(rapidjson::Value::ConstMemberIterator jsonIterator);
		EmissiveTexture parseEmissiveTexture(rapidjson::Value::ConstMemberIterator jsonIterator);

		// Parse generic
		MaterialGenericTexture parseMaterialGenericTexture (rapidjson::Value::ConstMemberIterator jsonIterator);
		Color3 parseColor3 (rapidjson::Value::ConstMemberIterator jsonIterator);
		Color4 parseColor4 (rapidjson::Value::ConstMemberIterator jsonIterator);

		// Miscellanious
		const std::string& getFileExtension (const std::string& fileName);
		void copyFile(const std::string& fileNameSource, std::string& fileNameDestination);
		const std::string& getJsonAsString(const std::string& jsonFileName);

	private:
		std::string mFileExtension;
		std::map<std::string, gLTFMaterial> mMaterialMap;
		Devnull devnull;
		PbrMetallicRoughness mPbrMetallicRoughness;
		NormalTexture mNormalTexture;
		OcclusionTexture mOcclusionTexture;
		EmissiveTexture mEmissiveTexture;
		MaterialGenericTexture mMaterialGenericTexture;
		Color3 mColor3;
		Color4 mColor4;
		std::string jsonString;
};

#endif
