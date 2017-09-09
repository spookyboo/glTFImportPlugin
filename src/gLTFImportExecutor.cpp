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
	std::ostringstream sstream;
	std::ifstream fs(fileName);
	sstream << fs.rdbuf();
	const std::string str(sstream.str());
	const char* jsonChar = str.c_str();
	OUT << "Json content: /n" << str << "/n";

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

	const rapidjson::Value& array = jsonIterator->value;
	gLTFMaterial material;

	OUT << "Loop through materials array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();
		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << "key ==> " << it->name.GetString() << "\n";
			if (it->value.IsString() && std::string(it->name.GetString()) == "name")
			{
				// ******** 1. name ********
				mMaterialMap[it->value.GetString()] = material;
			}
			if (it->value.IsObject() && std::string(it->name.GetString()) == "pbrMetallicRoughness")
			{
				// ******** 2. pbrMetallicRoughness ********
				PbrMetallicRoughness pbrMetallicRoughness = parsePbrMetallicRoughness(it);
				material.mPbrMetallicRoughness = pbrMetallicRoughness;
			}
			if (it->value.IsObject() && std::string(it->name.GetString()) == "normalTexture")
			{
				// ******** 3. normalTexture ********
				NormalTexture normalTexture = parseNormalTexture(it);
				material.mNormalTexture = normalTexture;
			}
			if (it->value.IsObject() && std::string(it->name.GetString()) == "occlusionTexture")
			{
				// ******** 4. occlusionTexture ********
				OcclusionTexture occlusionTexture = parseOcclusionTexture(it);
				material.mOcclusionTexture = occlusionTexture;
			}
			if (it->value.IsObject() && std::string(it->name.GetString()) == "emissiveTexture")
			{
				// ******** 5. emissiveTexture ********
				EmissiveTexture emissiveTexture = parseEmissiveTexture(it);
				material.mEmissiveTexture = emissiveTexture;
			}
			if (it->value.IsArray() && std::string(it->name.GetString()) == "emissiveFactor")
			{
				// ******** 6. emissiveFactor ********
				Color3 emissiveFactor = parseColor3(it);
				material.mEmissiveFactor = emissiveFactor;
			}
			if (it->value.IsString() && std::string(it->name.GetString()) == "alphaMode")
			{
				// ******** 7. alphaMode ********
				material.mAlphaMode = it->value.GetString();
			}
			if (it->value.IsFloat() && std::string(it->name.GetString()) == "alphaCutoff")
			{
				// ******** 8. alphaCutoff ********
				material.mAlphaCutoff = it->value.GetFloat();
			}
			if (it->value.IsBool() && std::string(it->name.GetString()) == "doubleSided")
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
		if (it->value.IsArray() && std::string(it->name.GetString()) == "baseColorFactor")
		{
			// ******** 2.1 baseColorFactor ********
			Color4 baseColorFactor = parseColor4(it);
			mPbrMetallicRoughness.mBaseColorFactor = baseColorFactor;
		}
		if (it->value.IsObject() && std::string(it->name.GetString()) == "baseColorTexture")
		{
			// ******** 2.2 baseColorTexture ********
			MaterialGenericTexture texture = parseMaterialGenericTexture(it);
			mPbrMetallicRoughness.mBaseColorTexture = texture;
		}
		if (it->value.IsFloat() && std::string(it->name.GetString()) == "metallicFactor")
		{
			// ******** 2.3 metallicFactor ********
			mPbrMetallicRoughness.mMetallicFactor = it->value.GetFloat();
		}
		if (it->value.IsFloat() && std::string(it->name.GetString()) == "roughnessFactor")
		{
			// ******** 2.4 roughnessFactor ********
			mPbrMetallicRoughness.mRoughnessFactor = it->value.GetFloat();
		}
		if (it->value.IsObject() && std::string(it->name.GetString()) == "metallicRoughnessTexture")
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
		if (it->value.IsFloat() && std::string(it->name.GetString()) == "scale")
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
		if (it->value.IsFloat() && std::string(it->name.GetString()) == "strength")
		{
			// ******** 4.3 scale ********
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
		if (it->value.IsInt() && std::string(it->name.GetString()) == "index")
		{
			// ******** index ********
			OUT << "key ==> " << it->name.GetString() << "\n";
			mMaterialGenericTexture.mIndex = it->value.GetInt();
			OUT << "value ==> " << mMaterialGenericTexture.mIndex << "\n";
		}
		if (it->value.IsInt() && std::string(it->name.GetString()) == "texCoord")
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

	mColor3.mRed = 0.0f;
	mColor3.mGreen = 0.0f;
	mColor3.mBlue = 0.0f;
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

	mColor4.mRed = 0.0f;
	mColor4.mGreen = 0.0f;
	mColor4.mBlue = 0.0f;
	mColor4.mAlpha = 0.0f;
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
