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

#include "gLTFMaterial.h"

//---------------------------------------------------------------------
void gLTFMaterial::out (void)
{
	OUT << "***************** gLTFMaterial *****************\n";
	mPbrMetallicRoughness.out();
	mNormalTexture.out();
	mOcclusionTexture.out();
	mEmissiveTexture.out();
	OUT << "***************** EmissiveFactor *****************\n";
	mEmissiveFactor.out();
	OUT << "mAlphaMode = " << mAlphaMode << "\n";
	OUT << "mAlphaCutoff = " << mAlphaCutoff << "\n";
	OUT << "mDoubleSided = " << mDoubleSided << "\n";
}

//---------------------------------------------------------------------
void PbrMetallicRoughness::out(void)
{
	OUT << "***************** PbrMetallicRoughness *****************\n";
	OUT << "***************** BaseColorFactor *****************\n";
	mBaseColorFactor.out();
	OUT << "***************** BaseColorTexture *****************\n";
	mBaseColorTexture.out();
	OUT << "mMetallicFactor = " << mMetallicFactor << "\n";
	OUT << "mRoughnessFactor = " << mRoughnessFactor << "\n";
	mMetallicRoughnessTexture.out();
}

//---------------------------------------------------------------------
void NormalTexture::out(void)
{
	OUT << "***************** NormalTexture *****************\n";
	MaterialGenericTexture::out();
	OUT << "mScale = " << mScale << "\n";
}

//---------------------------------------------------------------------
void OcclusionTexture::out(void)
{
	OUT << "***************** OcclusionTexture *****************\n";
	MaterialGenericTexture::out();
	OUT << "mStrength = " << mStrength << "\n";
}

//---------------------------------------------------------------------
void EmissiveTexture::out(void)
{
	OUT << "***************** EmissiveTexture *****************\n";
	MaterialGenericTexture::out();
}

//---------------------------------------------------------------------
void Color3::out(void)
{
	OUT << "mRed = " << mRed << "\n";
	OUT << "mGreen = " << mGreen << "\n";
	OUT << "mBlue = " << mBlue << "\n";
}

//---------------------------------------------------------------------
void Color4::out(void)
{
	OUT << "mRed = " << mRed << "\n";
	OUT << "mGreen = " << mGreen << "\n";
	OUT << "mBlue = " << mBlue << "\n";
	OUT << "mAlpha = " << mAlpha << "\n";
}

//---------------------------------------------------------------------
void MaterialGenericTexture::out(void)
{
	OUT << "mIndex = " << mIndex << "\n";
	OUT << "mTextCoord = " << mTextCoord << "\n";
}
