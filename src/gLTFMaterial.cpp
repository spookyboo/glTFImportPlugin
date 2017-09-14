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

#include "gLTFImportConstants.h"
#include "gLTFMaterial.h"

//---------------------------------------------------------------------
gLTFMaterial::gLTFMaterial(void) :
	mAlphaCutoff(0.5f),
	mDoubleSided(false)
{
	mAlphaMode = "OPAQUE";
	mEmissiveFactor.mRed = 0.0f;
	mEmissiveFactor.mGreen = 0.0f;
	mEmissiveFactor.mBlue = 0.0f;
}

//---------------------------------------------------------------------
void gLTFMaterial::out (void)
{
	OUT << "***************** Debug: gLTFMaterial *****************\n";
	mPbrMetallicRoughness.out();
	mNormalTexture.out();
	mOcclusionTexture.out();
	mEmissiveTexture.out();
	OUT << "***************** Debug: EmissiveFactor *****************\n";
	mEmissiveFactor.out();
	OUT << "mAlphaMode = " << mAlphaMode << "\n";
	OUT << "mAlphaCutoff = " << mAlphaCutoff << "\n";
	OUT << "mDoubleSided = " << mDoubleSided << "\n";
}

//---------------------------------------------------------------------
PbrMetallicRoughness::PbrMetallicRoughness (void) :
	mMetallicFactor (1.0f),
	mRoughnessFactor (1.0f)
{
}

//---------------------------------------------------------------------
void PbrMetallicRoughness::out (void)
{
	OUT << "***************** Debug: PbrMetallicRoughness *****************\n";
	OUT << "***************** Debug: BaseColorFactor *****************\n";
	mBaseColorFactor.out();
	OUT << "***************** Debug: BaseColorTexture *****************\n";
	mBaseColorTexture.out();
	OUT << "mMetallicFactor = " << mMetallicFactor << "\n";
	OUT << "mRoughnessFactor = " << mRoughnessFactor << "\n";
	OUT << "***************** Debug: MetallicRoughnessTexture *****************\n";
	mMetallicRoughnessTexture.out();
}

//---------------------------------------------------------------------
NormalTexture::NormalTexture (void) :
	MaterialGenericTexture (),
	mScale (1.0f)
{
}

//---------------------------------------------------------------------
void NormalTexture::out (void)
{
	OUT << "***************** Debug: NormalTexture *****************\n";
	MaterialGenericTexture::out();
	OUT << "mScale = " << mScale << "\n";
}

//---------------------------------------------------------------------
OcclusionTexture::OcclusionTexture (void) :
	MaterialGenericTexture (),
	mStrength (1.0f)
{
}

//---------------------------------------------------------------------
void OcclusionTexture::out (void)
{
	OUT << "***************** Debug: OcclusionTexture *****************\n";
	MaterialGenericTexture::out();
	OUT << "mStrength = " << mStrength << "\n";
}

//---------------------------------------------------------------------
EmissiveTexture::EmissiveTexture (void) :
	MaterialGenericTexture ()
{
}

//---------------------------------------------------------------------
void EmissiveTexture::out (void)
{
	OUT << "***************** Debug: EmissiveTexture *****************\n";
	MaterialGenericTexture::out();
}

//---------------------------------------------------------------------
void Color3::out (void)
{
	OUT << "mRed = " << mRed << "\n";
	OUT << "mGreen = " << mGreen << "\n";
	OUT << "mBlue = " << mBlue << "\n";
}

//---------------------------------------------------------------------
Color3::Color3 (void) :
	mRed (1.0f),
	mBlue (1.0f),
	mGreen (1.0f)
{
}

//---------------------------------------------------------------------
Color4::Color4 (void) :
	mRed (1.0f),
	mBlue (1.0f),
	mGreen (1.0f),
	mAlpha (1.0f)
{
}
	
//---------------------------------------------------------------------
void Color4::out (void)
{
	OUT << "mRed = " << mRed << "\n";
	OUT << "mGreen = " << mGreen << "\n";
	OUT << "mBlue = " << mBlue << "\n";
	OUT << "mAlpha = " << mAlpha << "\n";
}

//---------------------------------------------------------------------
MaterialGenericTexture::MaterialGenericTexture (void) :
	mIndex (-1),
	mTextCoord (0)
{
}

//---------------------------------------------------------------------
bool MaterialGenericTexture::isTextureAvailable (void) const
{
	return mIndex != -1;
}

//---------------------------------------------------------------------
void MaterialGenericTexture::out (void)
{
	OUT << "mIndex = " << mIndex << "\n";
	OUT << "mTextCoord = " << mTextCoord << "\n";
}
