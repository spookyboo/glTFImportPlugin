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
	OUT << "Perform gLTFImportExecutor::executeImport\n";

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
		result = createOgrePbsMaterialFiles(data);

	return result;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::executeBinary (const std::string& fileName, Ogre::HlmsEditorPluginData* data)
{
	OUT << "Perform gLTFImportExecutor::executeBinary\n";

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
	OUT << "Perform gLTFImportExecutor::executeText\n";

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
		OUT << "key ==> " << it->name.GetString() << "\n";
		std::string name(it->name.GetString());
		if (it->value.IsArray() && name == "materials")
		{
			parseMaterials(it);
		}
		if (it->value.IsArray() && name == "textures")
		{
			parseTextures(it);
		}
		if (it->value.IsArray() && name == "images")
		{
			parseImages(it);
		}
	}

	// Debug
	std::map<std::string, gLTFMaterial>::iterator it;
	for (it = mMaterialMap.begin(); it != mMaterialMap.end(); it++)
	{
		OUT << "\n***************** Debug: Material name = " << it->first << " *****************\n";
		(it->second).out();
	}

	data->mOutSuccessText = "gLTF file succesfully imported";

	return true;
}

//---------------------------------------------------------------------
//bool gLTFImportExecutor::readGlb()
//{
	//return true;
//}

//---------------------------------------------------------------------
bool gLTFImportExecutor::parseMaterials (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseMaterials\n";

	gLTFMaterial material;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << "Loop through materials array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();
		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << "key ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsString() && key == "name")
			{
				// ******** 1. name ********
				mMaterialMap[it->value.GetString()] = material;
			}
			if (it->value.IsObject() && key == "pbrMetallicRoughness")
			{
				// ******** 2. pbrMetallicRoughness ********
				PbrMetallicRoughness pbrMetallicRoughness = parsePbrMetallicRoughness(it);
				material.mPbrMetallicRoughness = pbrMetallicRoughness;
			}
			if (it->value.IsObject() && key == "normalTexture")
			{
				// ******** 3. normalTexture ********
				NormalTexture normalTexture = parseNormalTexture(it);
				material.mNormalTexture = normalTexture;
			}
			if (it->value.IsObject() && key == "occlusionTexture")
			{
				// ******** 4. occlusionTexture ********
				OcclusionTexture occlusionTexture = parseOcclusionTexture(it);
				material.mOcclusionTexture = occlusionTexture;
			}
			if (it->value.IsObject() && key == "emissiveTexture")
			{
				// ******** 5. emissiveTexture ********
				EmissiveTexture emissiveTexture = parseEmissiveTexture(it);
				material.mEmissiveTexture = emissiveTexture;
			}
			if (it->value.IsArray() && key == "emissiveFactor")
			{
				// ******** 6. emissiveFactor ********
				Color3 emissiveFactor = parseColor3(it);
				material.mEmissiveFactor = emissiveFactor;
			}
			if (it->value.IsString() && key == "alphaMode")
			{
				// ******** 7. alphaMode ********
				material.mAlphaMode = it->value.GetString();
			}
			if (it->value.IsFloat() && key == "alphaCutoff")
			{
				// ******** 8. alphaCutoff ********
				material.mAlphaCutoff = it->value.GetFloat();
			}
			if (it->value.IsBool() && key == "doubleSided")
			{
				// ******** 9. doubleSided ********
				material.mDoubleSided = it->value.GetBool();
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::parseTextures (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseTextures\n";
	// TODO
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::parseImages (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseImages\n";
	// TODO
	return true;
}

//---------------------------------------------------------------------
PbrMetallicRoughness gLTFImportExecutor::parsePbrMetallicRoughness (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parsePbrMetallicRoughness\n";

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		OUT << "key ==> " << it->name.GetString() << "\n";
		std::string key = std::string(it->name.GetString());
		if (it->value.IsArray() && key == "baseColorFactor")
		{
			// ******** 2.1 baseColorFactor ********
			Color4 baseColorFactor = parseColor4(it);
			mPbrMetallicRoughness.mBaseColorFactor = baseColorFactor;
		}
		if (it->value.IsObject() && key == "baseColorTexture")
		{
			// ******** 2.2 baseColorTexture ********
			MaterialGenericTexture texture = parseMaterialGenericTexture(it);
			mPbrMetallicRoughness.mBaseColorTexture = texture;
		}
		if (it->value.IsFloat() && key == "metallicFactor")
		{
			// ******** 2.3 metallicFactor ********
			mPbrMetallicRoughness.mMetallicFactor = it->value.GetFloat();
		}
		if (it->value.IsFloat() && key == "roughnessFactor")
		{
			// ******** 2.4 roughnessFactor ********
			mPbrMetallicRoughness.mRoughnessFactor = it->value.GetFloat();
		}
		if (it->value.IsObject() && key == "metallicRoughnessTexture")
		{
			// ******** 2.5 metallicRoughnessTexture ********
			MaterialGenericTexture texture = parseMaterialGenericTexture(it);
			mPbrMetallicRoughness.mMetallicRoughnessTexture = texture;
		}
	}

	return mPbrMetallicRoughness;
}

//---------------------------------------------------------------------
NormalTexture gLTFImportExecutor::parseNormalTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseNormalTexture\n";

	MaterialGenericTexture texture = parseMaterialGenericTexture(jsonIterator);
	mNormalTexture.mIndex = texture.mIndex;
	mNormalTexture.mTextCoord = texture.mTextCoord;

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		std::string key = std::string(it->name.GetString());
		if (it->value.IsFloat() && key == "scale")
		{
			// ******** 3.3 scale ********
			OUT << "key ==> " << it->name.GetString() << "\n";
			mNormalTexture.mScale = it->value.GetFloat();
			OUT << "value ==> " << mNormalTexture.mScale << "\n";
		}
	}

	return mNormalTexture;
}

//---------------------------------------------------------------------
OcclusionTexture gLTFImportExecutor::parseOcclusionTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseOcclusionTexture\n";

	MaterialGenericTexture texture = parseMaterialGenericTexture(jsonIterator);
	mOcclusionTexture.mIndex = texture.mIndex;
	mOcclusionTexture.mTextCoord = texture.mTextCoord;

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		std::string key = std::string(it->name.GetString());
		if (it->value.IsFloat() && key == "strength")
		{
			// ******** 4.3 strength ********
			OUT << "key ==> " << it->name.GetString() << "\n";
			mOcclusionTexture.mStrength = it->value.GetFloat();
			OUT << "value ==> " << mOcclusionTexture.mStrength << "\n";
		}
	}

	return mOcclusionTexture;
}

//---------------------------------------------------------------------
EmissiveTexture gLTFImportExecutor::parseEmissiveTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseEmissiveTexture\n";
	MaterialGenericTexture texture = parseMaterialGenericTexture(jsonIterator);
	mEmissiveTexture.mIndex = texture.mIndex;
	mEmissiveTexture.mTextCoord = texture.mTextCoord;
	return mEmissiveTexture;
}

//---------------------------------------------------------------------
MaterialGenericTexture gLTFImportExecutor::parseMaterialGenericTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseMaterialGenericTexture\n";

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		std::string key = std::string(it->name.GetString());
		if (it->value.IsInt() && key == "index")
		{
			// ******** index ********
			OUT << "key ==> " << it->name.GetString() << "\n";
			mMaterialGenericTexture.mIndex = it->value.GetInt();
			OUT << "value ==> " << mMaterialGenericTexture.mIndex << "\n";
		}
		if (it->value.IsInt() && key == "texCoord")
		{
			// ******** texCoord ********
			OUT << "key ==> " << it->name.GetString() << "\n";
			mMaterialGenericTexture.mTextCoord = it->value.GetInt();
			OUT << "value ==> " << mMaterialGenericTexture.mTextCoord << "\n";
		}
	}

	return mMaterialGenericTexture;
}

//---------------------------------------------------------------------
Color3 gLTFImportExecutor::parseColor3 (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseColor3\n";

	if (jsonIterator->value.IsArray())
	{
		const rapidjson::Value& array = jsonIterator->value;
		mColor3.mRed = array[0].GetFloat();
		mColor3.mGreen = array[1].GetFloat();
		mColor3.mBlue = array[2].GetFloat();

		for (rapidjson::SizeType i = 0; i < array.Size(); i++)
		{
			OUT << "value ==> " << array[i].GetFloat() << "\n";
		}
	}

	return mColor3;
}

//---------------------------------------------------------------------
Color4 gLTFImportExecutor::parseColor4 (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << "Perform gLTFImportExecutor::parseColor4\n";

	if (jsonIterator->value.IsArray())
	{
		const rapidjson::Value& array = jsonIterator->value;
		mColor4.mRed = array[0].GetFloat();
		mColor4.mGreen = array[1].GetFloat();
		mColor4.mBlue = array[2].GetFloat();
		mColor4.mAlpha= array[3].GetFloat();

		for (rapidjson::SizeType i = 0; i < array.Size(); i++)
		{
			OUT << "value ==> " << array[i].GetFloat() << "\n";
		}
	}

	return mColor4;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createOgrePbsMaterialFiles(Ogre::HlmsEditorPluginData* data)
{
	OUT << "Perform gLTFImportExecutor::createOgrePbsMaterialFiles\n";

	// Create the Ogre Pbs material files (*.material.json)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";


	// Iterate through materials and create for each material an Ogre Pbs .material.json file
	std::map<std::string, gLTFMaterial>::iterator it;
	for (it = mMaterialMap.begin(); it != mMaterialMap.end(); it++)
	{
		std::string ogreFullyQualifiedMaterialFileName = fullyQualifiedImportPath + it->first + ".material.json";
		OUT << "\nCreate: material file " << ogreFullyQualifiedMaterialFileName << "\n";

		// Create the file
		std::ofstream dst(ogreFullyQualifiedMaterialFileName);
		dst << "{\n";

		// ------------- SAMPLERS -------------
		// TODO

		// ------------- MACROBLOCKS -------------
		// TODO

		// ------------- BLENDBLOCKS -------------
		// TODO

		// ------------- PBS -------------
		dst << TAB << "\"pbs\" :\n";
		dst << TAB << "{\n";
		dst << TABx2 << "\"" << it->first << "\" :\n";
		dst << TABx2 << "{\n";
		createDiffuseJsonBlock(&dst, it->second);
		createSpecularJsonBlock(&dst, it->second);
		createMetalnessJsonBlock(&dst, it->second);
		createFresnelJsonBlock(&dst, it->second);
		createNormalJsonBlock(&dst, it->second);
		createRoughnessJsonBlock(&dst, it->second);
		createReflectionJsonBlock(&dst, it->second);
		dst << TABx3 << "\n";
		dst << TABx2 << "}\n";
		dst << TAB << "}\n";
		dst << "}\n";
		dst.close();

		// Validate the file to see whether it is valid Json
		std::string jsonString = getJsonAsString(ogreFullyQualifiedMaterialFileName);
		const char* jsonChar = jsonString.c_str();
		rapidjson::Document d;
		d.Parse(jsonChar);
		if (d.HasParseError())
			OUT << "Error, file " << ogreFullyQualifiedMaterialFileName << " is not valid Json\n";
		else
			OUT << "Ok, file " << ogreFullyQualifiedMaterialFileName << " is valid Json\n";
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createDiffuseJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	*dst << TABx3 << "\"diffuse\" :\n";
	*dst << TABx3 << "{\n";

	// Diffuse color
	*dst << TABx4 << "\"value\" : [" <<
		material.mPbrMetallicRoughness.mBaseColorFactor.mRed <<
		", " <<
		material.mPbrMetallicRoughness.mBaseColorFactor.mGreen <<
		", " <<
		material.mPbrMetallicRoughness.mBaseColorFactor.mBlue <<
		"]\n";

	// Background colour
	// TODO

	// Diffuse texture
	// TODO

	// Sampler
	// TODO

	*dst << TABx3 << "}," << "\n";
	
	return true; // There is always a default value
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createSpecularJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	std::string endWithChar = ",";
	*dst << TABx3 << "\"specular\" :\n";
	*dst << TABx3 << "{\n";
	// TODO
	*dst << TABx3 << "}";

	return true; // There is always a default value
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createMetalnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	*dst << "," << "\n";
	*dst << TABx3 << "\"metalness\" :\n";
	*dst << TABx3 << "{\n";
	// TODO
	*dst << TABx3 << "}";
	
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createFresnelJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// Specular workflow is not supported in gLTF by default
	
	return false;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createNormalJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	if (material.mNormalTexture.isTextureAvailable())
		return false;
	else
		*dst << "," << "\n";

	*dst << TABx3 << "\"normal\" :\n";
	*dst << TABx3 << "{\n";
	// TODO
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createRoughnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	if (material.mPbrMetallicRoughness.mMetallicRoughnessTexture.isTextureAvailable())
		return false;
	else
		*dst << "," << "\n";

	*dst << TABx3 << "\"roughness\" :\n";
	*dst << TABx3 << "{\n";
	// TODO
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createReflectionJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
{
	// TODO
	
	return false;
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::getFileExtension (const std::string& fileName)
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

//---------------------------------------------------------------------
void gLTFImportExecutor::copyFile (const std::string& fileNameSource, std::string& fileNameDestination)
{
	std::ifstream src(fileNameSource.c_str(), std::ios::binary);
	std::ofstream dst(fileNameDestination.c_str(), std::ios::binary);
	dst << src.rdbuf();
	dst.close();
	src.close();
}

//---------------------------------------------------------------------
const std::string& gLTFImportExecutor::getJsonAsString(const std::string& jsonFileName)
{
	jsonString = "";
	std::ostringstream sstream;
	std::ifstream fs(jsonFileName);
	sstream << fs.rdbuf();
	jsonString = sstream.str();
	return jsonString;
	//const std::string str(sstream.str());
	//return str.c_str();
}