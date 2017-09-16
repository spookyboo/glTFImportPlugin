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

#include "gLTFImportPbsMaterialsCreator.h"
#include "rapidjson/document.h"

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createOgrePbsMaterialFiles(Ogre::HlmsEditorPluginData* data,
	std::map<std::string, gLTFMaterial> materialsMap,
	std::map<int, gLTFTexture> texturesMap,
	std::map<int, gLTFImage> imagesMap,
	std::map<int, gLTFSampler> samplersMap)
{
	OUT << "Perform gLTFImportPbsMaterialsCreator::createOgrePbsMaterialFiles\n";

	// Create the Ogre Pbs material files (*.material.json)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Iterate through materials and create for each material an Ogre Pbs .material.json file
	std::map<std::string, gLTFMaterial>::iterator it;
	for (it = materialsMap.begin(); it != materialsMap.end(); it++)
	{
		std::string ogreFullyQualifiedMaterialFileName = fullyQualifiedImportPath + it->first + ".material.json";
		OUT << "\nCreate: material file " << ogreFullyQualifiedMaterialFileName << "\n";

		// Create the file
		std::ofstream dst(ogreFullyQualifiedMaterialFileName);
		dst << "{\n";

		// ------------- SAMPLERS -------------
		dst << TAB << "\"samplers\" :\n";
		dst << TAB << "{\n";
		createSamplerJsonBlock(&dst, it->second, samplersMap);
		dst << TAB << "},\n";

		// ------------- MACROBLOCKS -------------
		dst << TAB << "\"macroblocks\" :\n";
		dst << TAB << "{\n";
		createMacroJsonBlock(&dst, it->second); // TODO: Pass extra arguments?
		dst << TAB << "},\n";

		// ------------- BLENDBLOCKS -------------
		dst << TAB << "\"blendblocks\" :\n";
		dst << TAB << "{\n";
		createBlendJsonBlock(&dst, it->second); // TODO: Pass extra arguments?
		dst << TAB << "},\n";

		// ------------- PBS -------------
		dst << TAB << "\"pbs\" :\n";
		dst << TAB << "{\n";
		dst << TABx2 << "\"" << it->first << "\" :\n";
		dst << TABx2 << "{\n";

		// General Pbs datablock properties
		const gLTFMaterial& material = it->second;
		dst << TABx3 << "\"shadow_const_bias\" : \"0.01\",\n"; // Default value
		dst << TABx3 << "\"workflow\" : \"metallic\",\n"; // Default workflow of gLTF is metallic

		dst << TABx3 << "\"macroblock\" : \"Macroblock_0\",\n"; // Use a default macroblock
		dst << TABx3 << "\"blendblock\" : \"Blendblock_0\",\n"; // Use a default blendblock

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

		dst << "\n";
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
	copyImageFiles(data, imagesMap);

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createSamplerJsonBlock (std::ofstream* dst, 
	const gLTFMaterial& material,
	std::map<int, gLTFSampler> samplersMap)
{
	if (samplersMap.size() > 0)
	{
		// TODO: Create Samplerblocks, based on the samplersMap
	}
	else
	{
		// There are no gLTF texture samplers; use a default
		*dst << TABx2 << "\"Sampler_0\" :\n";
		*dst << TABx2 << "{\n";
		*dst << TABx3 << "\"min\" : \"anisotropic\",\n";
		*dst << TABx3 << "\"mag\" : \"anisotropic\",\n";
		*dst << TABx3 << "\"mip\" : \"anisotropic\",\n";
		*dst << TABx3 << "\"u\" : \"wrap\",\n";
		*dst << TABx3 << "\"v\" : \"wrap\",\n";
		*dst << TABx3 << "\"w\" : \"wrap\",\n";
		*dst << TABx3 << "\"miplodbias\" : 0,\n";
		*dst << TABx3 << "\"max_anisotropic\" : 1,\n";
		*dst << TABx3 << "\"compare_function\" : \"disabled\",\n";
		*dst << TABx3 << "\"border\" : [1, 1, 1, 1],\n";
		*dst << TABx3 << "\"min_lod\" : -3.40282e+38,\n";
		*dst << TABx3 << "\"max_lod\" : 3.40282e+38\n";
		*dst << TABx2 << "}\n";
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createMacroJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// Use a default
	*dst << TABx2 << "\"Macroblock_0\" :\n";
	*dst << TABx2 << "{\n";
	*dst << TABx2 << "\"scissor_test\" : false,\n";
	*dst << TABx3 << "\"depth_check\" : true,\n";
	*dst << TABx3 << "\"depth_write\" : true,\n";
	*dst << TABx3 << "\"depth_function\" : \"less_equal\",\n";
	*dst << TABx3 << "\"depth_bias_constant\" : 0,\n";
	*dst << TABx3 << "\"depth_bias_slope_scale\" : 0,\n";
	*dst << TABx3 << "\"cull_mode\" : \"clockwise\",\n";
	*dst << TABx3 << "\"polygon_mode\" : \"solid\"\n";
	*dst << TABx2 << "}\n";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createBlendJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
{
	// Use a default
	*dst << TABx2 << "\"Blendblock_0\" :\n";
	*dst << TABx2 << "{\n";
	*dst << TABx3 << "\"alpha_to_coverage\" : false,\n";
	*dst << TABx3 << "\"blendmask\" : \"rgba\",\n";
	*dst << TABx3 << "\"separate_blend\" : false,\n";
	*dst << TABx3 << "\"src_blend_factor\" : \"one\",\n";
	*dst << TABx3 << "\"dst_blend_factor\" : \"zero\",\n";
	*dst << TABx3 << "\"blend_operation\" : \"add\"\n";
	*dst << TABx2 << "}\n";

	return true;
}
//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createTransparencyJsonBlock (std::ofstream* dst, const gLTFMaterial& material)
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
bool gLTFImportPbsMaterialsCreator::createDiffuseJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
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
		*dst << ",\n";
		*dst << TABx4 << "\"texture\" : \"" << material.mPbrMetallicRoughness.mBaseColorTexture.mUri << "\",\n";
		*dst << TABx4 << "\"sampler\" : \"Sampler_" << material.mPbrMetallicRoughness.mBaseColorTexture.mSampler << "\"\n";
		// TODO: "uv" is based on the texCoord of the gLTF texture
	}


	*dst << "\n";
	*dst << TABx3 << "}," << "\n";

	return true; // There is always a default "value"
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createSpecularJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
{
	// Because gTLF only uses metallic workflow, the specular block does not contain a texture or sampler
	*dst << TABx3 << "\"specular\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : [1, 1, 1]\n"; // Default value of specular color
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createMetalnessJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
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
bool gLTFImportPbsMaterialsCreator::createFresnelJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
{
	// Specular workflow is not supported in gLTF by default

	return false;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createNormalJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
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
	*dst << TABx4 << "\"texture\" : \"" << material.mNormalTexture.mUri << "\",\n";
	*dst << TABx4 << "\"sampler\" : \"Sampler_" << material.mNormalTexture.mSampler << "\"\n";
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createRoughnessJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
{
	if (!material.mPbrMetallicRoughness.mMetallicRoughnessTexture.isTextureAvailable())
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"roughness\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : " << material.mPbrMetallicRoughness.mRoughnessFactor << ",\n";
	*dst << TABx4 << "\"texture\" : \"" << material.mPbrMetallicRoughness.mMetallicRoughnessTexture.mUri << "\",\n";
	*dst << TABx4 << "\"sampler\" : \"Sampler_" << material.mPbrMetallicRoughness.mMetallicRoughnessTexture.mSampler << "\"\n";
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createReflectionJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
{
	// TODO

	return false;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createDetailDiffuseJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
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
	*dst << TABx4 << "\"value\" : " << material.mOcclusionTexture.mStrength << ",\n"; // Use strength of the occlusionTexture as weight
																					 // baseColorTexture does not support Scale and Offset in gLTF
	*dst << TABx4 << "\"texture\" : \"" << material.mPbrMetallicRoughness.mBaseColorTexture.mUri << "\",\n";
	*dst << TABx4 << "\"sampler\" : \"Sampler_" << material.mPbrMetallicRoughness.mBaseColorTexture.mSampler << "\"\n";
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";

	++mDetailedDiffuseMapCount;
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createDetailNormalJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
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
		"],\n";
	*dst << TABx4 << "\"texture\" : \"" << material.mNormalTexture.mUri << "\",\n";
	*dst << TABx4 << "\"sampler\" : \"Sampler_" << material.mNormalTexture.mSampler << "\"\n";
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createEmissiveJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
{
	/* This function is a placeholder for the future when Ogre does support emissive textures in PBS materials.
	* Currently it does not, so an emissiveTexture shows up as an detailed diffuse map.
	*/
	if (!material.mEmissiveTexture.isTextureAvailable())
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"detail_diffuse" << mDetailedDiffuseMapCount << "\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"value\" : 6,\n"; // Pump up the light a bit
	*dst << TABx4 << "\"mode\" : \"Add\",\n"; // Use additive blending
	*dst << TABx4 << "\"texture\" : \"" << material.mEmissiveTexture.mUri << "\",\n";
	*dst << TABx4 << "\"sampler\" : \"Sampler_" << material.mEmissiveTexture.mSampler << "\"\n";
	// TODO: "uv" is based on the texCoord of the gLTF texture
	*dst << TABx3 << "}";

	++mDetailedDiffuseMapCount;
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::createDetailWeightJsonBlock(std::ofstream* dst, const gLTFMaterial& material)
{
	// Return if there is no occlusion texture
	if (!material.mOcclusionTexture.isTextureAvailable())
		return false;

	*dst << "," << "\n";
	*dst << TABx3 << "\"detail_weight\" :\n";
	*dst << TABx3 << "{\n";
	*dst << TABx4 << "\"texture\" : \"" << material.mOcclusionTexture.mUri << "\",\n";
	*dst << TABx4 << "\"sampler\" : \"Sampler_" << material.mOcclusionTexture.mSampler << "\"\n";

	// TODO: Add "sampler"
	*dst << TABx3 << "}";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportPbsMaterialsCreator::copyImageFiles (Ogre::HlmsEditorPluginData* data, std::map<int, gLTFImage> imagesMap)
{
	std::map<int, gLTFImage>::iterator it;
	gLTFImage image;
	std::string fileNameSource;
	std::string fileNameDestination;
	for (it = imagesMap.begin(); it != imagesMap.end(); it++)
	{
		image = it->second;
		fileNameSource = data->mInFileDialogPath + image.mUri; // TODO: uri can be a full path
		fileNameDestination = data->mInImportPath + data->mInFileDialogBaseName + "/" + image.mUri; // TODO: uri can be a full path
		OUT << "DEBUG fileNameSource: " << fileNameSource << "\n";
		OUT << "DEBUG fileNameDestination: " << fileNameDestination << "\n";
		copyFile (fileNameSource, fileNameDestination);
	}
	return true;
}
