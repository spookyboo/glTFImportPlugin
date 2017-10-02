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
#include "Ogre.h"
#include "OgreImage.h"
#include "base64.h"

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeImport (Ogre::HlmsEditorPluginData* data)
{
	OUT << "Perform gLTFImportExecutor::executeImport\n";

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

		// Create the Meshes
		result = mOgreMeshCreator.createOgreMeshFiles (data, mMeshesMap, mAccessorsMap, startBinaryBuffer);
	}

	return result;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeBinary (const std::string& fileName, 
	Ogre::HlmsEditorPluginData* data, 
	int& startBinaryBuffer)
{
	OUT << TAB << "Perform gLTFImportExecutor::executeBinary\n";

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
	// TODO: Check whether the test below must remain of removed
//	if (chunkType != 0x4E4F534A)
//	{
		//data->mOutErrorText = "Binary gLTF file does not contain a valid json specification";
		//fs.close();
		//return false;
	//}

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
	OUT << TAB << "Perform gLTFImportExecutor::executeText\n";

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
	OUT << TAB << "Perform gLTFImportExecutor::executeJson\n";

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
		// TODO: Parse animations, assets, nodes, scene, skins, ... ?????

		OUT << "-------------------------------- key gLTF ==> " << it->name.GetString() << " --------------------------------\n";
		std::string name(it->name.GetString());

		if (it->value.IsArray() && name == "accessors")
		{
			mAccessorsParser.parseAccessors(it); // Create a gLTFImportAccessorsParser object and parse the accessors
			mAccessorsMap = mAccessorsParser.getParsedAccessors();
		}
		if (it->value.IsArray() && name == "meshes")
		{
			mMeshesParser.parseMeshes(it); // Create a gLTFImportMeshesParser object and parse the meshes
			mMeshesMap = mMeshesParser.getParsedMeshes();
		}
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
	OUT << TABx2 << "Perform gLTFImportExecutor::propagateData\n";

	// Propagate the data to rearrange it in such a manner that creation of Ogre3d meshes and materials is easier
	propagateBufferViews();
	propagateMaterials(data, startBinaryBuffer);
	propagateAccessors();
	propagateMeshes(data, startBinaryBuffer);
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::propagateBufferViews (void)
{
	// Loop through bufferviews and propagate the data
	OUT << TABx3 << "Perform gLTFImportExecutor::propagateBufferViews\n";
	std::map<int, gLTFBufferView>::iterator itBufferViews;
	int bufferIndex;
	std::string uriBuffer;
	if (mBuffersMap.size() > 0)
	{
		for (itBufferViews = mBufferViewsMap.begin(); itBufferViews != mBufferViewsMap.end(); itBufferViews++)
		{
			bufferIndex = (itBufferViews->second).mBufferIndex;
			if (bufferIndex > -1)
			{
				// Assume uriBuffer is a file
				uriBuffer = mBuffersMap[bufferIndex].mUri;
				(itBufferViews->second).mUriDerived = uriBuffer;
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::propagateMaterials (Ogre::HlmsEditorPluginData* data, int startBinaryBuffer)
{
	// Loop through materials and propagate the data
	OUT << TABx3 << "Perform gLTFImportExecutor::propagateMaterials\n";
	std::map<std::string, gLTFMaterial>::iterator itMaterials;
	std::string materialName;
	std::string uriImage;
	int textureIndex;
	int sampler;
	for (itMaterials = mMaterialsMap.begin(); itMaterials != mMaterialsMap.end(); itMaterials++)
	{
		materialName = (itMaterials->second).mName;

		/* For each texture index it must be determined whether the uri of the image is from a binary file
		* containing an image or a link to an image file.
		* The uri names are changed (for convenience) before they are set in the gLTFMaterial
		*/

		// 0. baseColorTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
		{
			uriImage = getImageUriByTextureIndex(textureIndex);
			uriImage = copyImageFile("baseColorTexture", data, materialName, uriImage);
		}
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex("baseColorTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mUri = uriImage;

		// 1. emissiveTexture
		textureIndex = (itMaterials->second).mEmissiveTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
		{
			uriImage = getImageUriByTextureIndex(textureIndex);
			uriImage = copyImageFile("emissiveTexture", data, materialName, uriImage);
		}
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex("emissiveTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mEmissiveTexture.mUri = uriImage;

		// 2. normalTexture
		textureIndex = (itMaterials->second).mNormalTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
		{
			uriImage = getImageUriByTextureIndex(textureIndex);
			uriImage = copyImageFile("normalTexture", data, materialName, uriImage);
		}
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex("normalTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mNormalTexture.mUri = uriImage;

		// 3. mOcclusionTexture
		textureIndex = (itMaterials->second).mOcclusionTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
		{
			uriImage = getImageUriByTextureIndex(textureIndex);
			uriImage = copyImageFile("occlusionTexture", data, materialName, uriImage);
		}
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex("occlusionTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mOcclusionTexture.mUri = uriImage;

		// Occlusion is represented by the R channel
		convertTexture(uriImage, TTF_R_2_GBA); // Convert the image file into a usable occlusion texture

		// 4. metallicRoughnessTexture
		// Although not used by Ogre3d materials, they are still copied/extracted and used for metallicTexture and roughnessTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
		{
			uriImage = getImageUriByTextureIndex(textureIndex);
			uriImage = copyImageFile("metallicRoughnessTexture", data, materialName, uriImage);
		}
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex("metallicRoughnessTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mUri = uriImage;

		// 5. metallicTexture (copy of metallicRoughnessTexture)
		// The metallicRoughnessTexture is used as if it was a metallicTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
		{
			uriImage = getImageUriByTextureIndex(textureIndex);
			uriImage = copyImageFile("metallicTexture", data, materialName, uriImage);
		}
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex("metallicTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mMetallicTexture.mUri = uriImage;
		
		// Convert to Metallic texture. Metallic is represented by the B channel
		convertTexture(uriImage, TTF_G_2_RBA); // Convert the image file into a usable metallic texture

		// 6. roughnessTexture (copy of metallicRoughnessTexture)
		// The metallicRoughnessTexture is used as if it was a roughnessTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex;
		if (getImageByTextureIndex(textureIndex).mBufferView == -1)
		{
			uriImage = getImageUriByTextureIndex(textureIndex);
			uriImage = copyImageFile("roughnessTexture", data, materialName, uriImage);
		}
		else
			uriImage = extractAndCreateImageFromBufferByTextureIndex("roughnessTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mRoughnessTexture.mUri = uriImage;

		// Convert to Roughness texture. Roughness is represented by the G channel
		convertTexture(uriImage, TTF_G_2_RBA); // Convert the image file into a usable roughness texture

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
		(itMaterials->second).mPbrMetallicRoughness.mMetallicTexture.mSampler = sampler;
		(itMaterials->second).mPbrMetallicRoughness.mRoughnessTexture.mSampler = sampler;
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::propagateAccessors(void)
{
	OUT << TABx3 << "Perform gLTFImportExecutor::propagateAccessors\n";
	std::map<int, gLTFAccessor>::iterator itAccessors;

	// Iterate though the accessors
	for (itAccessors = mAccessorsMap.begin(); itAccessors != mAccessorsMap.end(); itAccessors++)
	{
		(itAccessors->second).mBufferIndexDerived = mBufferViewsMap[(itAccessors->second).mBufferView].mBufferIndex;
		(itAccessors->second).mByteOffsetBufferViewDerived = mBufferViewsMap[(itAccessors->second).mBufferView].mByteOffset;
		(itAccessors->second).mByteLengthDerived = mBufferViewsMap[(itAccessors->second).mBufferView].mByteLength;
		(itAccessors->second).mByteStrideDerived = mBufferViewsMap[(itAccessors->second).mBufferView].mByteStride;
		(itAccessors->second).mTargetDerived = mBufferViewsMap[(itAccessors->second).mBufferView].mTarget;
		(itAccessors->second).mUriDerived = mBufferViewsMap[(itAccessors->second).mBufferView].mUriDerived;
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::propagateMeshes (Ogre::HlmsEditorPluginData* data, int startBinaryBuffer)
{
	OUT << TABx3 << "Perform gLTFImportExecutor::propagateMeshes\n";
	std::map<int, gLTFMesh>::iterator itMeshes;
	std::map<int, gLTFPrimitive>::iterator itPrimitives;
	std::string materialName;
	gLTFAccessor accessor;

	// Iterate though the meshes
	for (itMeshes = mMeshesMap.begin(); itMeshes  != mMeshesMap.end(); itMeshes++)
	{
		// Iterate though the primitives
		for (itPrimitives = (itMeshes->second).mPrimitiveMap.begin(); itPrimitives != (itMeshes->second).mPrimitiveMap.end(); itPrimitives++)
		{
			(itPrimitives->second).mMaterialNameDerived = getMaterialNameByIndex((itPrimitives->second).mMaterial);

			// Iterate through Attribute map
			std::map<std::string, int>::iterator itAttr;
			for (itAttr = (itPrimitives->second).mAttributes.begin(); itAttr != (itPrimitives->second).mAttributes.end(); itAttr++)
			{
				// Assign the 
				if (itAttr->first == "POSITION")
				{
					(itPrimitives->second).mPositionAccessorDerived = itAttr->second;
				}
				if (itAttr->first == "NORMAL")
					(itPrimitives->second).mNormalAccessorDerived = itAttr->second;
				if (itAttr->first == "TANGENT")
					(itPrimitives->second).mTangentAccessorDerived = itAttr->second;
				if (itAttr->first == "TEXCOORD_0")
					(itPrimitives->second).mTexcoord_0AccessorDerived = itAttr->second;
				if (itAttr->first == "TEXCOORD_1")
					(itPrimitives->second).mTexcoord_1AccessorDerived = itAttr->second;
				if (itAttr->first == "COLOR_0")
					(itPrimitives->second).mColor_0AccessorDerived = itAttr->second;
				if (itAttr->first == "JOINTS_0")
					(itPrimitives->second).mJoints_0AccessorDerived = itAttr->second;
				if (itAttr->first == "WEIGHTS_0")
					(itPrimitives->second).mWeights_0AccessorDerived = itAttr->second;
			}

			// TODO: Progagate more...
		}
	}
		
	return true;
}

//---------------------------------------------------------------------
const gLTFImage& gLTFImportExecutor::getImageByTextureIndex (int index)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::getImageByTextureIndex\n";

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
	OUT << TABx4 << "Perform gLTFImportExecutor::getImageIndexByTextureIndex\n";

	if (index < 0 || index > mTexturesMap.size() - 1)
		return -1;

	gLTFTexture texture = mTexturesMap[index];
	int source = texture.mSource;
	
	return source;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::getImageUriByTextureIndex (int index)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::getImageUriByTextureIndex\n";

	gLTFImage image = getImageByTextureIndex(index);
	mHelperString = image.mUri; // The Uri is empty if the image could not be found
	return mHelperString;
}

//---------------------------------------------------------------------
int gLTFImportExecutor::getSamplerByTextureIndex (int index)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::getSamplerByTextureIndex\n";

	if (index < 0 || index > mTexturesMap.size() - 1)
		return 0;

	gLTFTexture texture = mTexturesMap[index];
	return texture.mSampler;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::getMaterialNameByIndex (int index)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::getMaterialNameByIndex\n";
	mHelperMaterialNameString = "BaseWhite";
	std::map<std::string, gLTFMaterial>::iterator itMaterials;

	int count = 0;
	for (itMaterials = mMaterialsMap.begin(); itMaterials != mMaterialsMap.end(); itMaterials++)
	{
		if (count == index)
		{
			mHelperMaterialNameString = (itMaterials->second).mName;
			return mHelperMaterialNameString;
		}
		count++;
	}

	return mHelperMaterialNameString;
}

//---------------------------------------------------------------------
const std::string&  gLTFImportExecutor::copyImageFile (const std::string& textureName,
	Ogre::HlmsEditorPluginData* data, 
	const std::string& materialName,
	const std::string& uriImage)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::copyImageFile\n";
	mHelperString = "";

	// Do not copy if the uriImage is empty
	if (uriImage == "")
		return mHelperString;

	std::string fileNameSource;
	if (isFilePathAbsolute(uriImage))
	{
		// It is a fully qualified filename
		fileNameSource = uriImage;
		OUT << TABx4 << "Copy fully qualified image fileName = " << fileNameSource << "\n";
	}
	else
	{
		// It is a relative filename
		fileNameSource = data->mInFileDialogPath + uriImage;
		OUT << TABx4 << "Copy relative image fileName = " << fileNameSource << "\n";
	}
	
	std::string baseName = getBaseFileNameWithExtension(uriImage);
	std::string extension = "." + getFileExtension(baseName);
	mHelperString = data->mInImportPath +
		data->mInFileDialogBaseName +
		"/" +
		materialName +
		"_" +
		textureName +
		extension;

	OUT << TABx4 << "Copy to image fileName = " << mHelperString << "\n";

	copyFile(fileNameSource, mHelperString);
	return mHelperString;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::extractAndCreateImageFromBufferByTextureIndex (const std::string& textureName,
	Ogre::HlmsEditorPluginData* data, 
	const std::string& materialName, 
	int index,
	int startBinaryBuffer)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::extractAndCreateImageFromBufferByTextureIndex\n";
	mHelperString = "";

	// Get the image; return if it doesn't have a bufferIndex
	gLTFImage image = getImageByTextureIndex(index);
	if (image.mBufferView < 0)
		return mHelperString;

	// Get the buffer file (uri)
	gLTFBufferView bufferView = mBufferViewsMap[image.mBufferView];
	std::string fileName;
	if (isFilePathAbsolute(bufferView.mUriDerived))
		fileName = bufferView.mUriDerived;
	else
		fileName = data->mInFileDialogPath + bufferView.mUriDerived;

	// Read the binary data
	std::streampos begin, end;
	std::ifstream ifs(fileName, std::ios::binary);

	// Set the stream to the beginning of the binary chunk
	ifs.seekg(startBinaryBuffer + bufferView.mByteOffset, std::ios::beg);

	// Read chunkLength
	uint32_t chunkLength;
	ifs.read((char*)&chunkLength, sizeof(chunkLength));

	// Read chunkType
	uint32_t chunkType;
	ifs.read((char*)&chunkType, sizeof(chunkType)); // Don't validate on type, because it is not always reliable

	char* imageBlock = new char[bufferView.mByteLength];
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

	mHelperString = data->mInImportPath +
		data->mInFileDialogBaseName +
		"/" +
		materialName +
		"_" +
		textureName +
		extension;
	std::ofstream ofs(mHelperString, std::ios::binary);
	ofs.write(imageBlock, bufferView.mByteLength);
	OUT << TABx4 << "Written image file " << mHelperString << "\n";
	ofs.close();

	delete[] imageBlock;
	return mHelperString;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::convertTexture (const std::string& fileName, TextureTransformation transformation)
{
#ifdef TEXTURE_TRANSFORMATION
	OUT << TABx4 << "Perform gLTFImportExecutor::convertTexture\n";
	if (fileName == "")
		return false; // File does not exist

	OUT << fileName << "\n";
	Ogre::Image image;
	image.load(fileName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::PixelBox pixelbox = image.getPixelBox(0, 0);
	Ogre::ColourValue col;
	size_t width = pixelbox.getWidth();
	size_t height = pixelbox.getHeight();

	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			col = pixelbox.getColourAt(x, y, 0);
			switch (transformation)
			{
				case TTF_R_2_GB:
				{
					col.b = col.r;
					col.g = col.r;
				}
				break;
				case TTF_G_2_RB:
				{
					col.r = col.g;
					col.b = col.g;
				}
				break;
				case TTF_B_2_RG:
				{
					col.r = col.b;
					col.g = col.b;
				}
				break;
				case TTF_R_2_GB_INV:
				{
					col.b = 1 - col.r;
					col.g = 1 - col.r;
					col.r = 1 - col.r;
				}
				break;
				case TTF_G_2_RB_INV:
				{
					col.r = 1 - col.g;
					col.b = 1 - col.g;
					col.g = 1 - col.g;
				}
				break;
				case TTF_B_2_RG_INV:
				{
					col.r = 1 - col.b;
					col.g = 1 - col.b;
					col.b = 1 - col.b;
				}
				break;
				case TTF_R_2_GBA:
				{
					col.b = col.r;
					col.g = col.r;
					col.a = col.r;
				}
				break;
				case TTF_G_2_RBA:
				{
					col.r = col.g;
					col.b = col.g;
					col.a = col.g;
				}
				break;
				case TTF_B_2_RGA:
				{
					col.r = col.b;
					col.g = col.b;
					col.a = col.b;
				}
				break;
			}
			pixelbox.setColourAt(col, x, y, 0);
		}
	}
	image.save(fileName);
#endif // DEBUG

	return true;
}
