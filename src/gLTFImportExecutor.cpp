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
	OUT << TAB<< "Perform gLTFImportExecutor::executeImport\n";

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
		// First enrich the data structures, so it can be easily used by the mPbsMaterialsCreator
		enrichMaterialsTexturesAndImages();

		// Create the Material files
		result = mPbsMaterialsCreator.createOgrePbsMaterialFiles(data, 
			mMaterialsMap, 
			mTexturesMap, 
			mImagesMap,
			mSamplersMap);
	}

	return result;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeBinary (const std::string& fileName, Ogre::HlmsEditorPluginData* data)
{
	OUT << TABx2 << "Perform gLTFImportExecutor::executeBinary\n";

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

	// Read Json chunk
	char* jsonChar = new char[chunkLength];
	fs.read(jsonChar, chunkLength);
	OUT << "Json content: \n" << jsonChar << "\n\n";
	if (!executeJson(jsonChar, data))
	{
		data->mOutErrorText = "Binary gLTF file does not contain a valid json specification";
		return false;
	}

	data->mOutSuccessText = "Binary gLTF file succesfully imported in " +
		data->mInImportPath + data->mInFileDialogBaseName;
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeText (const std::string& fileName, Ogre::HlmsEditorPluginData* data)
{
	OUT << TABx2 << "Perform gLTFImportExecutor::executeText\n";

	// Assume a gltf json file; read the file and copy the content to a const char*
	std::string jsonString = getJsonAsString(fileName);
	const char* jsonChar = jsonString.c_str();
	OUT << "Json content: \n" << jsonString << "\n\n";

	return executeJson (jsonChar, data);
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeJson (const char* jsonChar, Ogre::HlmsEditorPluginData* data)
{
	OUT << TABx2 << "gLTFImportExecutor::executeJson\n";

	// Parse the json block
	rapidjson::Document d;
	d.Parse(jsonChar);
	if (d.HasParseError())
	{
		data->mOutErrorText = "gLTF file cannot be parsed";
		return false;
	}

	// Check for each member in the json data
	rapidjson::Value::ConstMemberIterator itEnd = d.MemberEnd();
	for (rapidjson::Value::ConstMemberIterator it = d.MemberBegin(); it != itEnd; ++it)
	{
		// TODO: Parse bufferViews, buffers (needed for materials)
		// TODO: Parse accessors, animations, asset, meshes, nodes, scene (?), skins, ... (needed for meshes and animations?

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
		if (it->value.IsArray() && name == "samplers")
		{
			mSamplersParser.parseSamplers(it); // Create a gLTFImportSamplersParser object and parse the samplers
			mSamplersMap = mSamplersParser.getParsedSamplers();
		}
	}

	// Debug
	/*
	std::map<std::string, gLTFMaterial>::iterator it;
	for (it = mMaterialsMap.begin(); it != mMaterialsMap.end(); it++)
	{
	OUT << "\n***************** Debug: Material name = " << it->first << " *****************\n";
	(it->second).out();
	}
	*/

	data->mOutSuccessText = "gLTF file succesfully imported in " +
		data->mInImportPath + data->mInFileDialogBaseName;
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::enrichMaterialsTexturesAndImages (void)
{
	OUT << "gLTFImportExecutor::enrichMaterialsTexturesAndImages\n";
	std::map<std::string, gLTFMaterial>::iterator it;
	gLTFMaterial material;
	gLTFTexture texture;
	gLTFImage image;
	std::string uri;
	for (it = mMaterialsMap.begin(); it != mMaterialsMap.end(); it++)
	{
		// Add the uri's to the material
		uri = getImageUriByTextureIndex((it->second).mEmissiveTexture.mIndex);
		(it->second).mEmissiveTexture.mUri = uri;
		uri = getImageUriByTextureIndex((it->second).mNormalTexture.mIndex);
		(it->second).mNormalTexture.mUri = uri;
		uri = getImageUriByTextureIndex((it->second).mOcclusionTexture.mIndex);
		(it->second).mOcclusionTexture.mUri = uri;
		uri = getImageUriByTextureIndex((it->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex);
		OUT << "DEBUG: (it->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex =" << (it->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex << "\n";
		OUT << "DEBUG: uri = getImageUriByTextureIndex((it->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex); =" << uri << "\n";
		(it->second).mPbrMetallicRoughness.mBaseColorTexture.mUri = uri;
		uri = getImageUriByTextureIndex((it->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex);
		(it->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mUri = uri;

		// Add the samplers to the material
		int sampler = getSamplerByTextureIndex((it->second).mEmissiveTexture.mIndex);
		(it->second).mEmissiveTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((it->second).mNormalTexture.mIndex);
		(it->second).mNormalTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((it->second).mOcclusionTexture.mIndex);
		(it->second).mOcclusionTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((it->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex);
		(it->second).mPbrMetallicRoughness.mBaseColorTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((it->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex);
		(it->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mSampler = sampler;
	}

	return true;
}

//---------------------------------------------------------------------
const gLTFImage& gLTFImportExecutor::getImageByTextureIndex (int index)
{
	OUT << TABx3 << "gLTFImportExecutor::getImageByTextureIndex\n";

	mHelperImage = gLTFImage(); // Rest values

	if (index < 0 || index > mTexturesMap.size() - 1)
		return mHelperImage;

	gLTFTexture texture = mTexturesMap[index];
	int source = texture.mSource;

	if (source < 0 || source > mImagesMap.size() - 1)
		return mHelperImage;

	mHelperImage = mImagesMap[source];
	OUT << "DEBUG index=" << index << "\n";
	OUT << "DEBUG mHelperImage.mUri=" << mHelperImage.mUri << "\n";
	return mHelperImage;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::getImageUriByTextureIndex (int index)
{
	OUT << TABx2 << "gLTFImportExecutor::getImageUriByTextureIndex\n";
	OUT << "DEBUG: index1=" << index << "\n";

	gLTFImage image = getImageByTextureIndex(index);
	OUT << "DEBUG index2=" << index << "\n";
	mHelperString = image.mUri; // The Uri is empty if the image could not be found
	return mHelperString;
}

//---------------------------------------------------------------------
int gLTFImportExecutor::getSamplerByTextureIndex (int index)
{
	OUT << TABx2 << "gLTFImportExecutor::getSamplerByTextureIndex\n";

	if (index < 0 || index > mTexturesMap.size() - 1)
		return 0;

	gLTFTexture texture = mTexturesMap[index];
	return texture.mSampler;
}