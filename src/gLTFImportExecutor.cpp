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

#include "gLTFImportExecutor.h"
#include <sstream>
#include <fstream>

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeImport (Ogre::HlmsEditorPluginData* data)
{
	// Determine type of file
	std::string fileName = data->mInFileDialogPath + data->mInFileDialogName;
	std::string extension = getFileExtension(fileName);
	if (extension == "glb")
		return executeBinary (fileName, data);
	else
		return executeText(fileName, data);

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeBinary (const std::string& fileName, Ogre::HlmsEditorPluginData* data)
{
	std::streampos begin, end;
	std::ifstream fs(fileName, std::ios::binary);

	// Read first 4 bytes
	uint32_t magic;
	fs.seekg(0, std::ios::beg);
	fs.read((char*)&magic, sizeof(magic));
	if (magic != 0x46546C67)
	{
		data->mOutErrorText = "Binary gLTF file is not valid";
		return false;
	}

	// Read next 4 bytes
	uint32_t version;
	fs.read((char*)&version, sizeof(version));

	// Read next 4 bytes
	uint32_t length;
	fs.read((char*)&length, sizeof(length));

	// Read next 4 bytes
	uint32_t chunkLength;
	fs.read((char*)&chunkLength, sizeof(chunkLength));

	// Read next 4 bytes
	uint32_t chunkType;
	fs.read((char*)&chunkType, sizeof(chunkType));
	if (chunkType != 0x4E4F534A)
	{
		data->mOutErrorText = "Binary gLTF file does not contain a valid json specification";
		return false;
	}

	// TODO: Read Json chunk

	data->mOutSuccessText = "Binary gLTF file succesfully imported";
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeText (const std::string& fileName, Ogre::HlmsEditorPluginData* data)
{
	// Assume a gltf json file; read the file and copy the content to a const char*
	std::ostringstream sstream;
	std::ifstream fs(fileName);
	sstream << fs.rdbuf();
	const std::string str(sstream.str());
	const char* jsonChar = str.c_str();

	// Parse the json document
	rapidjson::Document d;
	d.Parse(jsonChar);
	if (d.HasParseError())
	{
		data->mOutErrorText = "gLTF file cannot be parsed";
		return false;
	}

	// Check for each member in the json file
	rapidjson::Value::ConstMemberIterator itEnd = d.MemberEnd();
	for (rapidjson::Value::ConstMemberIterator it = d.MemberBegin(); it != itEnd; ++it)
	{
		std::string name(it->name.GetString());
		if (it->value.IsArray() && name == "materials")
		{
			parseMaterial(it);
		}
		if (it->value.IsArray() && name == "textures")
		{
			// TODO
		}
		if (it->value.IsArray() && name == "images")
		{
			// TODO
		}
	}

	data->mOutSuccessText = "gLTF file succesfully imported";
}

//---------------------------------------------------------------------
//bool gLTFImportExecutor::readGlb()
//{
	//return true;
//}

//---------------------------------------------------------------------
bool gLTFImportExecutor::parseMaterial (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	// 1. "name"
	
	// 2.     "pbrMetallicRoughness"
	// 2.1.     "baseColorFactor"
	// 2.2.     "baseColorTexture"
	// 2.2.1.     "index"
	// 2.2.2.     "texCoord"
	// 2.3.     "metallicFactor"
	// 2.4.     "roughnessFactor"
	// 2.5.     "metallicRoughnessTexture"
	// 2.5.1.     "index"
	// 2.5.2.     "texCoord"
	// 2.6      "metallicFactor"
	// 2.7      "roughnessFactor"
	
	// 3.     "normalTexture"
	// 3.1.     "index"
	// 3.2.     "texCoord"
	// 3.3      "scale"
	
	// 4.     "occlusionTexture"
	// 4.1      "index"
	// 4.2      "texCoord"
	// 4.3      "strength"

	// 5.     "emissiveTexture"
	// 5.1      "index"
	// 5.2      "texCoord"

	// 6.     "emissiveFactor"
	
	// 7.     "alphaMode"
	
	// 8.     "alphaCutoff"
	
	// 9.     "doubleSided"

	return true;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::getFileExtension(const std::string& fileName)
{
	std::string::size_type idx;
	idx = fileName.rfind('.');
	mFileExtension = "";

	if (idx != std::string::npos)
	{
		mFileExtension = fileName.substr(idx + 1);
	}

	return mFileExtension;
}
