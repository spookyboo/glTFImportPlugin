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
#include "base64.h"

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeImport (Ogre::HlmsEditorPluginData* data)
{
	OUT << TAB<< "Perform gLTFImportExecutor::executeImport\n";

	bool result = true;

	// Determine type of file
	int startBinaryBuffer = 0;
	std::string fileName = data->mInFileDialogPath + data->mInFileDialogName;
	std::string extension = getFileExtension(fileName);
	if (extension == "glb")
		result = executeBinary (fileName, data, startBinaryBuffer);
	else
		result = executeText(fileName, data);

	// Create Ogre Pbs material files if parsing was succesful
	if (result)
	{
		// First propagate the data structure elements, so it can be easily used by the mPbsMaterialsCreator
		propagateData (data, startBinaryBuffer);

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
bool gLTFImportExecutor::executeBinary (const std::string& fileName, 
	Ogre::HlmsEditorPluginData* data, 
	int& startBinaryBuffer)
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
		fs.close();
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
		fs.close();
		return false;
	}

	startBinaryBuffer += 12; // Size of the header
	startBinaryBuffer += 8; // Size of the chunckLength and chunckType data
	startBinaryBuffer += chunkLength; // Size of the actual chunck length

	// Read Json chunk
	char* jsonChar = new char[chunkLength];
	fs.read(jsonChar, chunkLength);
	OUT << "Json content: \n" << jsonChar << "\n\n";
	if (!executeJson(fileName, jsonChar, data))
	{
		data->mOutErrorText = "Binary gLTF file does not contain a valid json specification";
		fs.close();
		delete[] jsonChar;
		return false;
	}

	data->mOutSuccessText = "Binary gLTF file succesfully imported in " +
		data->mInImportPath + data->mInFileDialogBaseName;

	delete[] jsonChar;
	fs.close();
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

	return executeJson (fileName, jsonChar, data);
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeJson (const std::string& fileName, 
	const char* jsonChar, 
	Ogre::HlmsEditorPluginData* data)
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
		if (it->value.IsArray() && name == "bufferViews")
		{
			mBufferViewsParser.parseBufferViews(it); // Create a gLTFImportBufferViewsParser object and parse the bufferViews
			mBufferViewsMap = mBufferViewsParser.getParsedBufferViews();
		}
		if (it->value.IsArray() && name == "buffers")
		{
			mBuffersParser.parseBuffers(fileName, it); // Create a gLTFImportBuffersParser object and parse the buffers
			mBuffersMap = mBuffersParser.getParsedBuffers();
		}

		gLTFImportBuffersParser mBuffersParser;
		gLTFImportBufferViewsParser mBufferViewsParser;

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
bool gLTFImportExecutor::propagateData (Ogre::HlmsEditorPluginData* data, int startBinaryBuffer)
{
	OUT << "gLTFImportExecutor::propagateData\n";
	std::map<std::string, gLTFMaterial>::iterator itMaterials;
	std::map<int, gLTFBufferView>::iterator itBufferViews;
	std::map<int, gLTFImage>::iterator itImages;
	gLTFMaterial material;
	std::string materialName;
	gLTFTexture texture;
	gLTFImage image;
	gLTFBufferView bufferView;
	gLTFBuffer buffer;
	std::string uriImage;
	std::string uriBuffer;
	int bufferIndex;
	int textureIndex;
	int sampler;

	// Loop through bufferviews and propagate the data
	if (mBuffersMap.size() > 0)
	{
		for (itBufferViews = mBufferViewsMap.begin(); itBufferViews != mBufferViewsMap.end(); itBufferViews++)
		{
			bufferIndex = (itBufferViews->second).mBufferIndex;
			if (bufferIndex > -1)
			{
				// Assume uriBuffer is a file
				uriBuffer = mBuffersMap[bufferIndex].mUri;
				(itBufferViews->second).mUri = uriBuffer;
			}
		}
	}

	// Loop through materials and propagate the data
	for (itMaterials = mMaterialsMap.begin(); itMaterials != mMaterialsMap.end(); itMaterials++)
	{
		materialName = (itMaterials->second).mName;

		// For each texture index it must be determined whether the uri of the image is from a binary file
		// containing an image or a link to an image file + add the uri's to the material

		// 0. baseColorTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
			uriImage = getImageUriByTextureIndex(textureIndex);
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex(data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mUri = uriImage;

		// 1. emissiveTexture
		textureIndex = (itMaterials->second).mEmissiveTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
			uriImage = getImageUriByTextureIndex(textureIndex);
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex(data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mEmissiveTexture.mUri = uriImage;

		// 2. normalTexture
		textureIndex = (itMaterials->second).mNormalTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
			uriImage = getImageUriByTextureIndex(textureIndex);
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex(data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mNormalTexture.mUri = uriImage;

		// 3. mOcclusionTexture
		textureIndex = (itMaterials->second).mOcclusionTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
			uriImage = getImageUriByTextureIndex(textureIndex);
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex(data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mOcclusionTexture.mUri = uriImage;

		// 4. metallicRoughnessTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
			uriImage = getImageUriByTextureIndex(textureIndex);
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex(data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mUri = uriImage;

		// Add the uri's to the material
		//uriImage = getImageUriByTextureIndex((itMaterials->second).mEmissiveTexture.mIndex);
		//(itMaterials->second).mEmissiveTexture.mUri = uriImage;
		//uriImage = getImageUriByTextureIndex((itMaterials->second).mNormalTexture.mIndex);
		//(itMaterials->second).mNormalTexture.mUri = uriImage;
		//uriImage = getImageUriByTextureIndex((itMaterials->second).mOcclusionTexture.mIndex);
		//(itMaterials->second).mOcclusionTexture.mUri = uriImage;
		//uriImage = getImageUriByTextureIndex((itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex);
		//(itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mUri = uriImage;
		//uriImage = getImageUriByTextureIndex((itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex);
		//(itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mUri = uriImage;

		// Add the samplers to the material
		sampler = getSamplerByTextureIndex((itMaterials->second).mEmissiveTexture.mIndex);
		(itMaterials->second).mEmissiveTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((itMaterials->second).mNormalTexture.mIndex);
		(itMaterials->second).mNormalTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((itMaterials->second).mOcclusionTexture.mIndex);
		(itMaterials->second).mOcclusionTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex);
		(itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex);
		(itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mSampler = sampler;
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
	return mHelperImage;
}

//---------------------------------------------------------------------
int gLTFImportExecutor::getImageIndexByTextureIndex (int index)
{
	OUT << TABx3 << "gLTFImportExecutor::getImageIndexByTextureIndex\n";

	if (index < 0 || index > mTexturesMap.size() - 1)
		return -1;

	gLTFTexture texture = mTexturesMap[index];
	int source = texture.mSource;
	
	return source;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::getImageUriByTextureIndex (int index)
{
	OUT << TABx2 << "gLTFImportExecutor::getImageUriByTextureIndex\n";

	gLTFImage image = getImageByTextureIndex(index);
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

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::extractAndCreateImageFromBufferByTextureIndex (Ogre::HlmsEditorPluginData* data, 
	const std::string& materialName, 
	int index,
	int startBinaryBuffer)
{
	OUT << TABx3 << "gLTFImportExecutor::extractAndCreateImageFromBufferByTextureIndex\n";
	mHelperString = "";

	// Get the image; return if it doesn't have a bufferIndex
	gLTFImage image = getImageByTextureIndex(index);
	if (image.mBufferView < 0)
		return mHelperString;

	// Get the bufferView
	gLTFBufferView bufferView = mBufferViewsMap[image.mBufferView];
	std::string fileName;
	if (isFilePathAbsolute(bufferView.mUri))
		fileName = bufferView.mUri;
	else
		fileName = data->mInFileDialogPath + bufferView.mUri;

	std::streampos begin, end;
	std::ifstream ifs(fileName, std::ios::binary);

	// Read the data
	char* imageBlock = new char[bufferView.mByteLength];
	ifs.seekg(startBinaryBuffer + bufferView.mByteOffset + 8, std::ios::beg); // Add another 8 bytes for the chunkLength and chunkType
	ifs.read(imageBlock, bufferView.mByteLength);
	ifs.close();

	// Write the image file
	// TODO: Check on existence of the image file or just overwrite?
	std::string extension = ".png";
	if (image.mMimeType == "image/jpeg")
		extension = ".jpg";
	if (image.mMimeType == "image/jpg")
		extension = ".jpg";
	if (image.mMimeType == "image/bmp")
		extension = ".bmp";
	if (image.mMimeType == "image/tiff")
		extension = ".tiff";

	std::stringstream outFileName;
	outFileName << "C:/temp/" << materialName << "_" << index << extension; // TODO: Determine target path
	mHelperString = outFileName.str();
	std::ofstream ofs(mHelperString, std::ios::binary);
	ofs.write(imageBlock, bufferView.mByteLength);
	OUT << TABx4 << "Written image file " << outFileName.str() << "\n";
	ofs.close();

	delete[] imageBlock;
	return mHelperString;
}