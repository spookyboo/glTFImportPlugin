/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __gLTFImportPbsMaterialsCreator_H__
#define __gLTFImportPbsMaterialsCreator_H__

#include <map>
#include "gLTFImportConstants.h"
#include "hlms_editor_plugin.h"
#include "gLTFMaterial.h"
#include "gLTFTexture.h"
#include "gLTFImage.h"
#include "gLTFSampler.h"

/** Class responsible for creation of Ogre PBS materials */
class gLTFImportPbsMaterialsCreator
{
	public:
		gLTFImportPbsMaterialsCreator(void) :
			mDetailedDiffuseMapCount(0)
		{
			mHelperString = "";
		};
		virtual ~gLTFImportPbsMaterialsCreator (void) {};

		// Create the Ogre Pbs Material files
		bool createOgrePbsMaterialFiles(Ogre::HlmsEditorPluginData* data,
			std::map<std::string, gLTFMaterial> materialsMap,
			std::map<int, gLTFTexture> texturesMap,
			std::map<int, gLTFImage> imagesMap,
			std::map<int, gLTFSampler> samplersMap); // Creates *.material.json files and copy images

	protected:
		// Create json blocks
		// Sampler
		bool createSamplerJsonBlock(std::ofstream* dst, 
			const gLTFMaterial& material, 
			std::map<int, gLTFSampler> samplersMap);

		// Macro
		bool createMacroJsonBlock(std::ofstream* dst, const gLTFMaterial& material);

		// Blend
		bool createBlendJsonBlock(std::ofstream* dst, const gLTFMaterial& material);

		// Pbs
		bool createTransparencyJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createDiffuseJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createSpecularJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createMetalnessJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createFresnelJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createNormalJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createRoughnessJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createReflectionJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createDetailDiffuseJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createDetailNormalJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createDetailWeightJsonBlock(std::ofstream* dst, const gLTFMaterial& material);
		bool createEmissiveJsonBlock(std::ofstream* dst, const gLTFMaterial& material);

		// Generic
		const std::string getUvString (int texCoord);
		bool copyImageFiles(Ogre::HlmsEditorPluginData* data, std::map<int, gLTFImage> imagesMap);
	
	private:
		std::string mHelperString;
		unsigned short mDetailedDiffuseMapCount;
};

#endif
