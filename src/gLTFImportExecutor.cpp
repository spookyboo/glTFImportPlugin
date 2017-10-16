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

	// Determine filenames
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";
	std::string fullyQualifiedBaseName = fullyQualifiedImportPath + data->mInFileDialogBaseName;
	mHlmsProjectFileName = fullyQualifiedBaseName + ".hlmp";
	mMaterialsConfigFileName = fullyQualifiedBaseName + "_materials.cfg";
	mTexturesConfigFileName = fullyQualifiedBaseName + "_textures.cfg";

	// Determine type of file
	int startBinaryBuffer = 0;
	std::string fileName = data->mInFileDialogPath + data->mInFileDialogName;
	std::string extension = getFileExtension(fileName);
	if (extension == "glb")
		result = executeBinary(fileName, data, startBinaryBuffer);
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

		// In case of a full binary file the start of the buffer is +8 bytes
		if (extension == "glb")
			startBinaryBuffer += 8;
		
		// Create the Meshes
		result = mOgreMeshCreator.createOgreMeshFiles (data, 
			mNodesMap, 
			mMeshesMap, 
			mAccessorsMap, 
			startBinaryBuffer);
	}

	// Create the HLMSEditor project file
	std::ofstream dst(mHlmsProjectFileName);
	dst << "hlmsEditor v1.0\n";
	dst << mMaterialsConfigFileName << "\n";
	dst << mTexturesConfigFileName << "\n";
	dst.close();
	setProjectFileNamePropertyValue(data, mHlmsProjectFileName);

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
	char* jsonChar = new char[chunkLength+1];
	memset(jsonChar, 0, chunkLength+1);
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

		if (it->value.IsArray() && name == "nodes")
		{
			mNodesParser.parseNodes(it); // Parse the nodes
			mNodesMap = mNodesParser.getParsedNodes();
		}
		if (it->value.IsArray() && name == "accessors")
		{
			mAccessorsParser.parseAccessors(it); // Parse the accessors
			mAccessorsMap = mAccessorsParser.getParsedAccessors();
		}
		if (it->value.IsArray() && name == "meshes")
		{
			mMeshesParser.parseMeshes(it); // Parse the meshes
			mMeshesMap = mMeshesParser.getParsedMeshes();
		}
		if (it->value.IsArray() && name == "materials")
		{
			mMaterialsParser.parseMaterials(it); // Parse the materials
			mMaterialsMap = mMaterialsParser.getParsedMaterials();
		}
		if (it->value.IsArray() && name == "textures")
		{
			mTexturesParser.parseTextures(it); // Parse the textures
			mTexturesMap = mTexturesParser.getParsedTextures();
		}
		if (it->value.IsArray() && name == "images")
		{
			mImagesParser.parseImages(it); // Parse the images
			mImagesMap = mImagesParser.getParsedImages();
		}
		if (it->value.IsArray() && name == "samplers")
		{
			mSamplersParser.parseSamplers(it); // Parse the samplers
			mSamplersMap = mSamplersParser.getParsedSamplers();
		}
		if (it->value.IsArray() && name == "bufferViews")
		{
			mBufferViewsParser.parseBufferViews(it); // Parse the bufferViews
			mBufferViewsMap = mBufferViewsParser.getParsedBufferViews();
		}
		if (it->value.IsArray() && name == "buffers")
		{
			mBuffersParser.parseBuffers(fileName, it); // Parse the buffers
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
	//propagateMeshes(data, startBinaryBuffer);
	propagateMeshes(data);
	propagateNodes(data);
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
	int matCount = 4;
	int texCount = 4;
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";
	std::string ogreFullyQualifiedMaterialFileName;
	std::string ogreMaterialFileName;

	// Create the materials config file
	std::ofstream matFile(mMaterialsConfigFileName);
	matFile << "1	0	1	1	PBS	PBS\n";

	// Create the textures config file
	std::ofstream texFile(mTexturesConfigFileName);
	texFile << "3	0	3	1	Textures	Textures\n";

	for (itMaterials = mMaterialsMap.begin(); itMaterials != mMaterialsMap.end(); itMaterials++)
	{
		materialName = (itMaterials->second).mName;

		// Write and entry in the materials config file
		ogreFullyQualifiedMaterialFileName = fullyQualifiedImportPath + materialName + ".material.json";
		ogreMaterialFileName = materialName + ".material.json";
		matFile << "1	1	" << matCount << "	3	";
		matFile << ogreMaterialFileName << "	" << ogreFullyQualifiedMaterialFileName << "\n";

		/* For each texture index it must be determined whether the uri of the image refers to a binary file
		* containing an image, a link to an image file or to embeded base64 encoded data in the uri itself.
		* The uri names are changed (for convenience) before they are set in the gLTFMaterial.
		*/

		// 0. baseColorTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mIndex;
		uriImage = prepareUri("baseColorTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mBaseColorTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		/*
		if (uriImage != "")
		{
			texFile << "3	3	" << texCount << "	3	";
			texFile << uriImage << "	" << uriImage << "\n";
			texCount++;
		}
		*/

		// 1. emissiveTexture
		textureIndex = (itMaterials->second).mEmissiveTexture.mIndex;
		uriImage = prepareUri("emissiveTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mEmissiveTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		/*
		if (uriImage != "")
		{
			texFile << "3	3	" << texCount << "	3	";
			texFile << uriImage << "	" << uriImage << "\n";
			texCount++;
		}
		*/

		// 2. normalTexture
		textureIndex = (itMaterials->second).mNormalTexture.mIndex;
		uriImage = prepareUri("normalTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mNormalTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		/*
		if (uriImage != "")
		{
			texFile << "3	3	" << texCount << "	3	";
			texFile << uriImage << "	" << uriImage << "\n";
			texCount++;
		}
		*/

		// 3. mOcclusionTexture
		textureIndex = (itMaterials->second).mOcclusionTexture.mIndex;
		uriImage = prepareUri("occlusionTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mOcclusionTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		/*
		if (uriImage != "")
		{
			texFile << "3	3	" << texCount << "	3	";
			texFile << uriImage << "	" << uriImage << "\n";
			texCount++;
		}
		*/

		// Occlusion is represented by the R channel
		convertTexture(uriImage, TTF_R_2_GB_INV); // Convert the image file into a usable occlusion texture

		// 4. metallicRoughnessTexture
		// Although not used by Ogre3d materials, they are still copied/extracted and used for metallicTexture and roughnessTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex;
		uriImage = prepareUri("metallicRoughnessTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		/*
		if (uriImage != "")
		{
			texFile << "3	3	" << texCount << "	3	";
			texFile << uriImage << "	" << uriImage << "\n";
			texCount++;
		}
		*/

		// 5. metallicTexture (copy of metallicRoughnessTexture)
		// The metallicRoughnessTexture is used as if it was a metallicTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex;
		uriImage = prepareUri("metallicTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mMetallicTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		/*
		if (uriImage != "")
		{
			texFile << "3	3	" << texCount << "	3	";
			texFile << uriImage << "	" << uriImage << "\n";
			texCount++;
		}
		*/

		// Convert to Metallic texture. Metallic is represented by the B channel
		convertTexture(uriImage, TTF_B_2_RGA); // Convert the image file into a usable metallic texture

		// 6. roughnessTexture (copy of metallicRoughnessTexture)
		// The metallicRoughnessTexture is used as if it was a roughnessTexture
		textureIndex = (itMaterials->second).mPbrMetallicRoughness.mMetallicRoughnessTexture.mIndex;
		uriImage = prepareUri("roughnessTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mPbrMetallicRoughness.mRoughnessTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		/*
		if (uriImage != "")
		{
			texFile << "3	3	" << texCount << "	3	";
			texFile << uriImage << "	" << uriImage << "\n";
			texCount++;
		}
		*/

		// Convert to Roughness texture. Roughness is represented by the G channel
		convertTexture(uriImage, TTF_G_2_RBA); // Convert the image file into a usable roughness texture

		// 7. Extension: KHR diffuseTexture
		textureIndex = (itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_DiffuseTexture.mIndex;
		uriImage = prepareUri("diffuseTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_DiffuseTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);

		// 8. Extension: KHR specularGlossinessTexture
		textureIndex = (itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularGlossinessTexture.mIndex;
		uriImage = prepareUri("specularGlossinessTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularGlossinessTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);

		// 9. Extension: glossinessTexture (roughness) (copy of KHR_specularGlossinessTexture)
		// The specularGlossinessTexture is used as if it was a glossiness texture/roughness texture
		textureIndex = (itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularGlossinessTexture.mIndex;
		uriImage = prepareUri("glossinessTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_GlossinessTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);
		
		// Convert. Glossiness is represented by the A channel
		convertTexture(uriImage, TTF_A_2_RGBA);

		// 10. Extension: KHR specularTexture (copy of KHR_specularGlossinessTexture)
		// The specularGlossinessTexture is used as if it was a specular texture
		textureIndex = (itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularGlossinessTexture.mIndex;
		uriImage = prepareUri("specularTexture", data, materialName, textureIndex, startBinaryBuffer);
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularTexture.mUri = uriImage;
		writeTextureEntryToConfig(texFile, uriImage, texCount);

		// Convert. Specular is represented by the RGB channel; just set Alpha to 1.0
		convertTexture(uriImage, TTF_0001);

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
		sampler = getSamplerByTextureIndex((itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_DiffuseTexture.mIndex);
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_DiffuseTexture.mSampler = sampler;
		sampler = getSamplerByTextureIndex((itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularGlossinessTexture.mIndex);
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularGlossinessTexture.mSampler = sampler;
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_GlossinessTexture.mSampler = sampler;
		(itMaterials->second).mKHR_PbrSpecularGlossiness.mKHR_SpecularTexture.mSampler = sampler;

		// TODO: Add sampler for KHR

		matCount++;
	}

	texFile.close();
	matFile << "2	0	2	1	Unlit	Unlit\n";
	matFile.close();

	return true;
}

//---------------------------------------------------------------------
void gLTFImportExecutor::writeTextureEntryToConfig (std::ofstream& texFile, const std::string& uriImage, int& texCount)
{
	if (uriImage != "")
	{
		texFile << "3	3	" << texCount << "	3	";
		texFile << uriImage << "	" << uriImage << "\n";
		texCount++;
	}
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
bool gLTFImportExecutor::propagateMeshes (Ogre::HlmsEditorPluginData* data)
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
bool gLTFImportExecutor::propagateNodes(Ogre::HlmsEditorPluginData* data)
{
	OUT << TABx3 << "Perform gLTFImportExecutor::propagateNodes\n";

	// Propagate the mesh to the node and the transformations to child nodes
	std::map<int, gLTFNode>::iterator itNodes;
	std::map<int, gLTFNode>::iterator itNodesEnd = mNodesMap.end();
	gLTFMesh mesh;

	// Iterate though the nodes, propagate transformation to childres and set the derived mesh object
	for (itNodes = mNodesMap.begin(); itNodes != itNodesEnd; itNodes++)
	{
		gLTFNode* node = &itNodes->second;
		propagateNodeTransformsToChildren(&(itNodes->second));
		mesh = mMeshesMap[itNodes->second.mMesh];
		itNodes->second.mMeshDerived = mesh;
	}

	return true;
}

//---------------------------------------------------------------------
void gLTFImportExecutor::propagateNodeTransformsToChildren (gLTFNode* node)
{
	std::vector<int>::iterator itChildNodes;
	std::vector<int>::iterator itChildNodesEnd;
	gLTFNode* childNode;
	int count;

	itChildNodesEnd = node->mChildren.end();
	count = 0;
	for (itChildNodes = node->mChildren.begin(); itChildNodes != itChildNodesEnd; itChildNodes++)
	{
		childNode = findNodeByIndex(node->mChildren[count]);
		if (childNode && !childNode->mTransformationDerived)
		{
			inheritTransforms(node, childNode);
			propagateNodeTransformsToChildren(childNode); // Propagate to the lower tree
		}
		count++;
	}
}

//---------------------------------------------------------------------
void gLTFImportExecutor::inheritTransforms (gLTFNode* parentNode, gLTFNode* childNode)
{
	//OUT << "inheritTransforms for parent " << parentNode->mName << " and child " << childNode->mName << "\n";

	OUT << "gLTFImportExecutor::inheritTransforms\n";
	Ogre::Matrix4 matrixParent;
	Ogre::Vector3 scaleParent = Ogre::Vector3(1.0f, 1.0f, 1.0f);
	Ogre::Quaternion rotationParent = Ogre::Quaternion::IDENTITY;
	Ogre::Vector3 translationParent = Ogre::Vector3::ZERO;
	Ogre::Matrix4 matrixChild;
	Ogre::Vector3 scaleChild = Ogre::Vector3(1.0f, 1.0f, 1.0f);
	Ogre::Quaternion rotationChild = Ogre::Quaternion::IDENTITY;
	Ogre::Vector3 translationChild = Ogre::Vector3::ZERO;

	// First translate everything to matrix4
	if (parentNode->mHasMatrix)
	{
		// Parent has a matrix4
		matrixParent = Ogre::Matrix4(
			parentNode->mMatrix[0], parentNode->mMatrix[4], parentNode->mMatrix[8], parentNode->mMatrix[12],
			parentNode->mMatrix[1], parentNode->mMatrix[5], parentNode->mMatrix[9], parentNode->mMatrix[13],
			parentNode->mMatrix[2], parentNode->mMatrix[6], parentNode->mMatrix[10], parentNode->mMatrix[14],
			parentNode->mMatrix[3], parentNode->mMatrix[7], parentNode->mMatrix[11], parentNode->mMatrix[15]);
	}
	else
	{
		// Parent has TRS
		if (parentNode->mHasScale)
			scaleParent = Ogre::Vector3(parentNode->mScale[0], parentNode->mScale[1], parentNode->mScale[2]);
		if (parentNode->mHasRotation)
			rotationParent = Ogre::Quaternion(parentNode->mRotation[3], parentNode->mRotation[0], parentNode->mRotation[1], parentNode->mRotation[2]);
		if (parentNode->mHasTranslation)
			translationParent = Ogre::Vector3(parentNode->mTranslation[0], parentNode->mTranslation[1], parentNode->mTranslation[2]);
		matrixParent.makeTransform(translationParent, scaleParent, rotationParent);
	}

	if (childNode->mHasMatrix)
	{
		// Child has a matrix4
		matrixChild = Ogre::Matrix4(
			childNode->mMatrix[0], childNode->mMatrix[4], childNode->mMatrix[8], childNode->mMatrix[12],
			childNode->mMatrix[1], childNode->mMatrix[5], childNode->mMatrix[9], childNode->mMatrix[13],
			childNode->mMatrix[2], childNode->mMatrix[6], childNode->mMatrix[10], childNode->mMatrix[14],
			childNode->mMatrix[3], childNode->mMatrix[7], childNode->mMatrix[11], childNode->mMatrix[15]);
	}
	else
	{
		// Child has TRS
		if (childNode->mHasScale)
			scaleChild = Ogre::Vector3(childNode->mScale[0], childNode->mScale[1], childNode->mScale[2]);
		if (childNode->mHasRotation)
			rotationChild = Ogre::Quaternion(childNode->mRotation[3], childNode->mRotation[0], childNode->mRotation[1], childNode->mRotation[2]);
		if (childNode->mHasTranslation)
			translationChild = Ogre::Vector3(childNode->mTranslation[0], childNode->mTranslation[1], childNode->mTranslation[2]);
		matrixChild.makeTransform(translationChild, scaleChild, rotationChild);
	}

	// Perform matrix transformation
	matrixChild = matrixParent * matrixChild;
	matrixChild.decomposition(translationChild, scaleChild, rotationChild);

	// Set result back to the childNode
	if (childNode->mHasMatrix)
	{
		childNode->mMatrix[0] = matrixChild[0][0];
		childNode->mMatrix[1] = matrixChild[1][0];
		childNode->mMatrix[2] = matrixChild[2][0];
		childNode->mMatrix[3] = matrixChild[3][0];
		childNode->mMatrix[4] = matrixChild[0][1];
		childNode->mMatrix[5] = matrixChild[1][1];
		childNode->mMatrix[6] = matrixChild[2][1];
		childNode->mMatrix[7] = matrixChild[3][1];
		childNode->mMatrix[8] = matrixChild[0][2];
		childNode->mMatrix[9] = matrixChild[1][2];
		childNode->mMatrix[10] = matrixChild[2][2];
		childNode->mMatrix[11] = matrixChild[3][2];
		childNode->mMatrix[12] = matrixChild[0][3];
		childNode->mMatrix[13] = matrixChild[1][3];
		childNode->mMatrix[14] = matrixChild[2][3];
		childNode->mMatrix[15] = matrixChild[3][3];
		childNode->mHasMatrix = true;
	}
	else
	{
		// Set all 3 TRS elements
		// Any unused elements have a 'neutral'  value (e.g. translation of Vector3::Zero or an Identity Quaternion)
		matrixChild.decomposition(translationChild, scaleChild, rotationChild);
		childNode->mScale[0] = scaleChild.x;
		childNode->mScale[1] = scaleChild.y;
		childNode->mScale[2] = scaleChild.z;
		childNode->mHasScale = true;

		childNode->mRotation[0] = rotationChild.x;
		childNode->mRotation[1] = rotationChild.y;
		childNode->mRotation[2] = rotationChild.z;
		childNode->mRotation[3] = rotationChild.w;
		childNode->mHasRotation = true;

		childNode->mTranslation[0] = translationChild.x;
		childNode->mTranslation[1] = translationChild.y;
		childNode->mTranslation[2] = translationChild.z;
		childNode->mHasTranslation = true;
	}

	childNode->mTransformationDerived = true;
}

//---------------------------------------------------------------------
gLTFNode* gLTFImportExecutor::findNodeByIndex (int nodeIndex)
{
	OUT << TABx4 << "gLTFImportExecutor::findNodeByIndex\n";

	std::map<int, gLTFNode>::iterator it = mNodesMap.find(nodeIndex);
	if (it != mNodesMap.end())
	{
		// Found the node
		return &(it->second);
	}
	
	return 0;
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
const std::string& gLTFImportExecutor::prepareUri (const std::string&  textureName,
	Ogre::HlmsEditorPluginData* data,
	const std::string&  materialName,
	int textureIndex, 
	int startBinaryBuffer)
{
	mHelperUri = "";
	if (getImageByTextureIndex(textureIndex).mBufferView == -1)
	{
		mHelperUri = getImageUriByTextureIndex(textureIndex);
		if (isUriEmbeddedBase64(mHelperUri))
			mHelperUri = extractAndCreateImageFromUriByTextureIndex(textureName, data, materialName, textureIndex);
		else
			mHelperUri = copyImageFile(textureName, data, materialName, mHelperUri);
	}
	else
		mHelperUri = extractAndCreateImageFromFileByTextureIndex(textureName, data, materialName, textureIndex, startBinaryBuffer);

	return mHelperUri;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::extractAndCreateImageFromUriByTextureIndex (const std::string& textureName,
	Ogre::HlmsEditorPluginData* data,
	const std::string& materialName,
	int index)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::extractAndCreateImageFromUriByTextureIndex\n";
	mHelperString = "";

	// Get the image
	gLTFImage image = getImageByTextureIndex(index);

	// Get the uri; it must be a base64 encoded string
	std::string uri = image.mUri; // Get the uri from image
	if (!isUriEmbeddedBase64(uri))
		return mHelperString;

	char* imageBlock;
	// Get the binary data from a base64 encoded string
	std::string imageBuffer = getEmbeddedBase64FromUri(uri); // Get the encoded data
	imageBuffer = base64_decode(imageBuffer); // Decode it

	// Read the data from the string
	std::istringstream iss(imageBuffer);

	// Set the stream to the beginning of the binary chunk
	iss.seekg(0, std::ios::beg);

	imageBlock = new char[imageBuffer.size()];
	iss.read(imageBlock, imageBuffer.size());

	// Write the image file
	mHelperString = writeImageFile(textureName, image.mMimeType, data, materialName, imageBlock, imageBuffer.size());
	delete[] imageBlock;
	return mHelperString;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::extractAndCreateImageFromFileByTextureIndex (const std::string& textureName,
	Ogre::HlmsEditorPluginData* data, 
	const std::string& materialName, 
	int index,
	int startBinaryBuffer)
{
	OUT << TABx4 << "Perform gLTFImportExecutor::extractAndCreateImageFromFileByTextureIndex\n";
	mHelperString = "";

	// Get the image; return if it doesn't have a bufferIndex
	gLTFImage image = getImageByTextureIndex(index);
	if (image.mBufferView < 0)
		return mHelperString;

	// Get the uri and determine whether it is a file or it contains the bufferdata as base64 encoded string
	gLTFBufferView bufferView = mBufferViewsMap[image.mBufferView];
	std::string uri = bufferView.mUriDerived; // The uri refers to the (binary) file

	// Uri may not be a base64 encoded one
	if (isUriEmbeddedBase64(uri))
		return mHelperString;

	char* imageBlock;

	// Get the binary data from file
	if (!isFilePathAbsolute(uri))
		uri = data->mInFileDialogPath + uri;

	// Read the binary data from file
	std::streampos begin, end;
	std::ifstream ifs(uri, std::ios::binary);

	// Set the stream to the beginning of the binary chunk
	ifs.seekg(startBinaryBuffer + bufferView.mByteOffset, std::ios::beg);

	// Read chunkLength
	uint32_t chunkLength;
	ifs.read((char*)&chunkLength, sizeof(chunkLength));

	// Read chunkType
	uint32_t chunkType;
	ifs.read((char*)&chunkType, sizeof(chunkType)); // Don't validate on type, because it is not always reliable

	imageBlock = new char[bufferView.mByteLength];
	ifs.read(imageBlock, bufferView.mByteLength);
	ifs.close();

	// Write the image file
	mHelperString = writeImageFile(textureName, image.mMimeType, data, materialName, imageBlock, bufferView.mByteLength);
	delete[] imageBlock;
	return mHelperString;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::writeImageFile (const std::string& textureName,
	const std::string& mimeType,
	Ogre::HlmsEditorPluginData* data,
	const std::string& materialName,
	char* imageBlock,
	size_t byteLength)
{
	// Write the image file
	// TODO: Check on existence of the image file or just overwrite?
	std::string extension = getExtensionFromMimeType(mimeType);
	mHelperOutputFile = data->mInImportPath +
		data->mInFileDialogBaseName +
		"/" +
		materialName +
		"_" +
		textureName +
		extension;
	std::ofstream ofs(mHelperOutputFile, std::ios::binary);
	ofs.write(imageBlock, byteLength);
	OUT << TABx4 << "Written image file " << mHelperOutputFile << "\n";
	ofs.close();
	return mHelperOutputFile;
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
				case TTF_R000:
				{
					col.g = 0.0f;
					col.b = 0.0f;
					col.a = 0.0f;
				}
				case TTF_0001:
				{
					col.a = 1.0f;
				}
				break;
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
				case TTF_A_2_RGBA:
				{
					col.r = col.a;
					col.g = col.a;
					col.b = col.a;
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

//---------------------------------------------------------------------
bool gLTFImportExecutor::setProjectFileNamePropertyValue (Ogre::HlmsEditorPluginData* data, const std::string& fileName)
{
	Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY property;
	property.propertyName = "load_project";
	property.type = Ogre::HlmsEditorPluginData::STRING;
	property.stringValue = fileName;
	data->mOutReferencesMap[property.propertyName] = property;
	return true;
}
