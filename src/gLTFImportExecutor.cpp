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

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeImport (Ogre::HlmsEditorPluginData* data)
{
	OUT << "  Perform gLTFImportExecutor::executeImport\n";

	bool result = true;

	// Determine type of file
	std::string fileName = data->mInFileDialogPath + data->mInFileDialogName;
	std::string extension = getFileExtension(fileName);
	if (extension == "glb")
		result = executeBinary (fileName, data);
	else
		result = executeText(fileName, data);

	// Create Ogre Pbs material files if parsing was succesful
	if (result)
	{
		result = mPbsMaterialsCreator.createOgrePbsMaterialFiles(data, mMaterialsMap, mTexturesMap, mImagesMap);
	}

	return result;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeBinary (const std::string& fileName, Ogre::HlmsEditorPluginData* data)
{
	OUT << "    Perform gLTFImportExecutor::executeBinary\n";

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
	OUT << "    Perform gLTFImportExecutor::executeText\n";

	// Assume a gltf json file; read the file and copy the content to a const char*
	std::string jsonString = getJsonAsString(fileName);
	const char* jsonChar = jsonString.c_str();
	OUT << "Json content: \n" << jsonString << "\n\n";

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
		OUT << "key gLTF ==> " << it->name.GetString() << "\n";
		std::string name(it->name.GetString());
		if (it->value.IsArray() && name == "materials")
		{
			mMaterialsParser.parseMaterials(it); // Create a gLTFImportMaterialsParser object and parse the materials
			mMaterialsMap = mMaterialsParser.getParsedMaterials();
		}
		if (it->value.IsArray() && name == "textures")
		{
			mTexturesParser.parseTextures(it); // Create a gLTFImportTexturesParser object and parse the textures
			mTexturesMap = mTexturesParser.getParsedTextures();
		}
		if (it->value.IsArray() && name == "images")
		{
			mImagesParser.parseImages(it); // Create a gLTFImportImagesParser object and parse the images
			mImagesMap = mImagesParser.getParsedImages();
		}
	}

	// Debug
	std::map<std::string, gLTFMaterial>::iterator it;
	for (it = mMaterialsMap.begin(); it != mMaterialsMap.end(); it++)
	{
		OUT << "\n***************** Debug: Material name = " << it->first << " *****************\n";
		(it->second).out();
	}

	data->mOutSuccessText = "gLTF file succesfully imported";

	return true;
}
