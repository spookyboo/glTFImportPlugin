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
			mMaterialsExecutor.parseMaterials(it); // Create a gLTFImportMaterialsExecutor object and parse the materials
			mMaterialsMap = mMaterialsExecutor.getParsedMaterials();
		}
		if (it->value.IsArray() && name == "textures")
		{
			mTexturesExecutor.parseTextures(it); // Create a gLTFImportTexturesExecutor object and parse the textures
			//mTexturesMap = mTexturesExecutor.getParsedTextures();
		}
		if (it->value.IsArray() && name == "images")
		{
			mImagesExecutor.parseImages(it); // Create a gLTFImportImagesExecutor object and parse the images
			//mImagesMap = mImagesExecutor.getParsedImages();
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

//---------------------------------------------------------------------
bool gLTFImportExecutor::createOgrePbsMaterialFiles(Ogre::HlmsEditorPluginData* data)
{
	OUT << "Perform gLTFImportExecutor::createOgrePbsMaterialFiles\n";

	// Create the Ogre Pbs material files (*.material.json)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Iterate through materials and create for each material an Ogre Pbs .material.json file
	std::map<std::string, gLTFMaterial>::iterator it;
	for (it = mMaterialsMap.begin(); it != mMaterialsMap.end(); it++)
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

		// General Pbs datablock properties
		const gLTFMaterial& material = it->second;
		dst << TABx3 << "\"shadow_const_bias\" : \"0.01\",\n"; // Default value
		dst << TABx3 << "\"workflow\" : \"metallic\",\n"; // Default workflow of gLTF is metallic
		std::string doubleSidedValue = "false";
		if (material.mDoubleSided)
			doubleSidedValue = "true";
		dst << TABx3 << "\"two_sided\" : " << doubleSidedValue << ",\n";
		// TODO: mAlphaMode == "MASK" is similar to alpha test ("alpha_test") / alpha threshold ["less_equal", mAlphaCutoff],

		// Transparency
		createTransparencyJsonBlock(&dst, it->second);
		
		// 'textures'
		createDiffuseJsonBlock(&dst, it->second);
		createSpecularJsonBlock(&dst, it->second);
		createMetalnessJsonBlock(&dst, it->second);
		createFresnelJsonBlock(&dst, it->second);
		createNormalJsonBlock(&dst, it->second);
		createRoughnessJsonBlock(&dst, it->second);
		createReflectionJsonBlock(&dst, it->second);
		createDetailDiffuseJsonBlock(&dst, it->second);
		createDetailNormalJsonBlock(&dst, it->second);
		createDetailWeightJsonBlock(&dst, it->second);
		createEmissiveJsonBlock(&dst, it->second);
		
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

	// Copy all gLTF files (images)

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createTransparencyJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	return false;
	if (material.mAlphaMode != "BLEND")
		return false;
		
	*dst << TABx3 << "\"transparency\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : " << material.mAlphaCutoff << "\n"; // TODO: No incorrect!!! mAlphaCutoff only used in case mAlphaMode == MASK
	// TODO: How to make use "use_alpha_from_textures"
	*dst << TABx3 << "}," << "\n";
	
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
		"]";

	// Background colour (TODO: Check if supported in gLTF)
	*dst << "," << "\n";
	*dst << TABx4 << "\"background\" : [1, 1, 1, 1]";

	/* The baseColorTexture becomes a diffuse texture in case there is NO occlusionTexture
	*  present in the gLTF material.
	*/
	if (material.mPbrMetallicRoughness.mBaseColorTexture.isTextureAvailable() &&
		!material.mOcclusionTexture.isTextureAvailable())
	{
		// TODO: Add "texture" + add "sampler"
		// TODO: "uv" is based on the texCoord of the gLTF texture
	}


	*dst << "\n";
	*dst << TABx3 << "}," << "\n";
	
	return true; // There is always a default "value"
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createSpecularJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// Because gTLF only uses metallic workflow, the specular block does not contain a texture or sampler
	*dst << TABx3 << "\"specular\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : [1, 1, 1]\n"; // Default value of specular color
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createMetalnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	*dst << "," << "\n";
	*dst << TABx3 << "\"metalness\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : " << material.mPbrMetallicRoughness.mMetallicFactor << "\n";

	// TODO: What is the texture?
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
	// Return if there is no normal texture
	if (!material.mNormalTexture.isTextureAvailable())
		return false;

	/* The normal texture becomes a detail normal texture in case the scale is <> 1
	 */
	if (material.mNormalTexture.mScale != 1.0f)
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"normal\" :\n";
	*dst << TABx3 << "{\n";
	// TODO: Add "texture" + add "sampler"
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createRoughnessJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	if (!material.mPbrMetallicRoughness.mMetallicRoughnessTexture.isTextureAvailable())
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"roughness\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : " << material.mPbrMetallicRoughness.mRoughnessFactor << "\n";
	// TODO: Add "texture" + add "sampler"
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createReflectionJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// TODO
	
	return false;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createDetailDiffuseJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// There must at least be a baseColorTexture
	if (!material.mPbrMetallicRoughness.mBaseColorTexture.isTextureAvailable())
		return false;

	/* The baseColorTexture becomes a detail diffuse texture in case there is also a occlusionTexture
	 * present in the gLTF material. This means in practice that only 1 detail diffuse texture can be present.
	 */
	if (!material.mOcclusionTexture.isTextureAvailable())
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"detail_diffuse" << mDetailedDiffuseMapCount << "\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : " << material.mOcclusionTexture.mStrength << "\n"; // Use strength of the occlusionTexture as weight
	// baseColorTexture does not support Scale and Offset in gLTF
	// TODO: Add "texture" + add "sampler"
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";
	
	++mDetailedDiffuseMapCount;
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createDetailNormalJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// Return if there is no normal texture
	if (!material.mNormalTexture.isTextureAvailable())
		return false;

	/* The normal texture becomes a detail normal texture in case the scale is <> 1. This means also that
	 * only 1 detail normal texture can be present.
	 * @remark:
	 * Unlike the the diffuse map, a normal mal will not become a detail normal map in case an occlusion texture
	 * is present. The gLTF specification does not specify whether an occlusion texture also affects a normal map.
	 * This means that property "value" (= weight) is always 1.0 (= the default, so it is omitted).
	 */
	if (material.mNormalTexture.mScale == 1.0f)
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"detail_normal0\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << 
		"\"scale\" : [" <<
		material.mNormalTexture.mScale <<
		", " << 
		material.mNormalTexture.mScale << 
		"]\n";
	// TODO: Add texture and sampler (take the , into account)
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createEmissiveJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	/* This function is a placeholder for the future when Ogre does support emissive textures in PBS materials.
	 * Currently it does not, so an emissiveTexture shows up as an detailed diffuse map.
	 */
	if (!material.mEmissiveTexture.isTextureAvailable())
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"detail_diffuse" << mDetailedDiffuseMapCount << "\" :\n";
	*dst << TABx3 << "{\n";
	// TODO: Add "texture" + add "sampler"
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";
	
	++mDetailedDiffuseMapCount;
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportExecutor::createDetailWeightJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// Return if there is no occlusion texture
	if (!material.mOcclusionTexture.isTextureAvailable())
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"detail_weight\" :\n";
	*dst << TABx3 << "{\n";
	// TODO: Add "texture" and "sampler"
	*dst << TABx3 << "}";

	return true;
}
