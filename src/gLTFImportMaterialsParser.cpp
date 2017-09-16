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

#include "gLTFImportMaterialsParser.h"

//---------------------------------------------------------------------
bool gLTFImportMaterialsParser::parseMaterials (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportMaterialsParser::parseMaterials\n";

	gLTFMaterial material;
	std::string name;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through materials array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();
		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key material ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsString() && key == "name")
			{
				// ******** 1. name ********
				name = it->value.GetString();
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
			if (it->value.IsNumber() && key == "alphaCutoff")
			{
				// ******** 8. alphaCutoff ********
				material.mAlphaCutoff = it->value.GetFloat();
			}
			if (it->value.IsBool() && key == "doubleSided")
			{
				// ******** 9. doubleSided ********
				material.mDoubleSided = it->value.GetBool();
			}

			if (name != "")
				mMaterialsMap[name] = material;
		}
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<std::string, gLTFMaterial> gLTFImportMaterialsParser::getParsedMaterials (void) const
{
	return mMaterialsMap;
}

//---------------------------------------------------------------------
PbrMetallicRoughness gLTFImportMaterialsParser::parsePbrMetallicRoughness (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx2 << "Perform gLTFImportMaterialsParser::parsePbrMetallicRoughness\n";

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		OUT << TABx3 << "key metallic roughness ==> " << it->name.GetString() << "\n";
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
		if (it->value.IsNumber() && key == "metallicFactor")
		{
			// ******** 2.3 metallicFactor ********
			mPbrMetallicRoughness.mMetallicFactor = it->value.GetFloat();
		}
		if (it->value.IsNumber() && key == "roughnessFactor")
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
NormalTexture gLTFImportMaterialsParser::parseNormalTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx2 << "Perform gLTFImportMaterialsParser::parseNormalTexture\n";

	MaterialGenericTexture texture = parseMaterialGenericTexture(jsonIterator);
	mNormalTexture.mIndex = texture.mIndex;
	mNormalTexture.mTextCoord = texture.mTextCoord;

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		std::string key = std::string(it->name.GetString());
		if (it->value.IsNumber() && key == "scale")
		{
			// ******** 3.3 scale ********
			OUT << TABx4 << "key normal texture ==> " << it->name.GetString() << "\n";
			mNormalTexture.mScale = it->value.GetFloat();
			OUT << TABx4 << "value ==> " << mNormalTexture.mScale << "\n";
		}
	}

	return mNormalTexture;
}

//---------------------------------------------------------------------
OcclusionTexture gLTFImportMaterialsParser::parseOcclusionTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx3 << "Perform gLTFImportMaterialsParser::parseOcclusionTexture\n";

	MaterialGenericTexture texture = parseMaterialGenericTexture(jsonIterator);
	mOcclusionTexture.mIndex = texture.mIndex;
	mOcclusionTexture.mTextCoord = texture.mTextCoord;

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		std::string key = std::string(it->name.GetString());
		if (it->value.IsNumber() && key == "strength")
		{
			// ******** 4.3 strength ********
			OUT << TABx4 << "key occlusion texture ==> " << it->name.GetString() << "\n";
			mOcclusionTexture.mStrength = it->value.GetFloat();
			OUT << TABx4 << "value ==> " << mOcclusionTexture.mStrength << "\n";
		}
	}

	return mOcclusionTexture;
}

//---------------------------------------------------------------------
EmissiveTexture gLTFImportMaterialsParser::parseEmissiveTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx3 << "Perform gLTFImportMaterialsParser::parseEmissiveTexture\n";
	MaterialGenericTexture texture = parseMaterialGenericTexture(jsonIterator);
	mEmissiveTexture.mIndex = texture.mIndex;
	mEmissiveTexture.mTextCoord = texture.mTextCoord;
	return mEmissiveTexture;
}

//---------------------------------------------------------------------
MaterialGenericTexture gLTFImportMaterialsParser::parseMaterialGenericTexture (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx3 << "Perform gLTFImportMaterialsParser::parseMaterialGenericTexture\n";

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		std::string key = std::string(it->name.GetString());
		if (it->value.IsInt() && key == "index")
		{
			// ******** index ********
			OUT << TABx4 << "key texture ==> " << it->name.GetString() << "\n";
			mMaterialGenericTexture.mIndex = it->value.GetInt();
			OUT << TABx4 << "value ==> " << mMaterialGenericTexture.mIndex << "\n";
		}
		if (it->value.IsInt() && key == "texCoord")
		{
			// ******** texCoord ********
			OUT << TABx4 << "key texture ==> " << it->name.GetString() << "\n";
			mMaterialGenericTexture.mTextCoord = it->value.GetInt();
			OUT << TABx4 << "value ==> " << mMaterialGenericTexture.mTextCoord << "\n";
		}
	}

	return mMaterialGenericTexture;
}

//---------------------------------------------------------------------
Color3 gLTFImportMaterialsParser::parseColor3 (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx3 << "Perform gLTFImportMaterialsParser::parseColor3\n";

	if (jsonIterator->value.IsArray())
	{
		const rapidjson::Value& array = jsonIterator->value;
		mColor3.mRed = array[0].GetFloat();
		mColor3.mGreen = array[1].GetFloat();
		mColor3.mBlue = array[2].GetFloat();

		for (rapidjson::SizeType i = 0; i < array.Size(); i++)
		{
			OUT << TABx4 << "value ==> " << array[i].GetFloat() << "\n";
		}
	}

	return mColor3;
}

//---------------------------------------------------------------------
Color4 gLTFImportMaterialsParser::parseColor4 (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx3 << "Perform gLTFImportMaterialsParser::parseColor4\n";

	if (jsonIterator->value.IsArray())
	{
		const rapidjson::Value& array = jsonIterator->value;
		mColor4.mRed = array[0].GetFloat();
		mColor4.mGreen = array[1].GetFloat();
		mColor4.mBlue = array[2].GetFloat();
		mColor4.mAlpha= array[3].GetFloat();

		for (rapidjson::SizeType i = 0; i < array.Size(); i++)
		{
			OUT << TABx4 << "value ==> " << array[i].GetFloat() << "\n";
		}
	}

	return mColor4;
}
