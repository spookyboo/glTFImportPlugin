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
#ifndef __gLTFMaterial_H__
#define __gLTFMaterial_H__

#include <string>
#include <iostream>

/************************************************************************************************
 This file contains the data struture of a gLFTmaterial. This is an intermediate structure
 used to generate an Ogre 3d material file.
/************************************************************************************************/

/***********************************************/
/** Class responsible that represents a Color3 */
/***********************************************/
class Color3
{
public:
	Color3 (void);
	virtual ~Color3 (void) {};
	void out (void);

	// Public member attributes
	float mRed;
	float mGreen;
	float mBlue;
};

/***********************************************/
/** Class responsible that represents a Color4 */
/***********************************************/
class Color4
{
public:
	Color4 (void);
	virtual ~Color4 (void) {};
	void out (void);

	// Public member attributes
	float mRed;
	float mGreen;
	float mBlue;
	float mAlpha;
};

/***************************************************************/
/** Class responsible that represents a MaterialGenericTexture */
/***************************************************************/
class MaterialGenericTexture
{
public:
	MaterialGenericTexture (void);
	virtual ~MaterialGenericTexture (void) {};
	void out (void);
	bool isTextureAvailable (void) const;

	// Public member attributes
	int mIndex;
	int mTextCoord;
	std::string mUri;
	int mSampler;
};

/*************************************************************/
/** Class responsible that represents a PbrMetallicRoughness */
/*************************************************************/
class PbrMetallicRoughness
{
	public:
		PbrMetallicRoughness (void);
		virtual ~PbrMetallicRoughness (void) {};
		void out (void);

		// Public member attributes
		Color4 mBaseColorFactor;
		MaterialGenericTexture mBaseColorTexture;
		float mMetallicFactor;
		float mRoughnessFactor;
		MaterialGenericTexture mMetallicRoughnessTexture;
		MaterialGenericTexture mMetallicTexture;
		MaterialGenericTexture mRoughnessTexture;
};

/******************************************************/
/** Class responsible that represents a NormalTexture */
/******************************************************/
class NormalTexture : public MaterialGenericTexture
{
	public:
		NormalTexture (void);
		virtual ~NormalTexture (void) {};
		void out (void);

		// Public member attributes
		float mScale;
};

/*********************************************************/
/** Class responsible that represents a OcclusionTexture */
/*********************************************************/
class OcclusionTexture : public MaterialGenericTexture
{
	public:
		OcclusionTexture (void);
		virtual ~OcclusionTexture (void) {};
		void out (void);

		// Public member attributes
		float mStrength;
};

/********************************************************/
/** Class responsible that represents a EmissiveTexture */
/********************************************************/
class EmissiveTexture : public MaterialGenericTexture
{
	public:
		EmissiveTexture (void);
		virtual ~EmissiveTexture (void) {};
		void out (void);

		// Public member attributes
		// ?
};

/*************************************************/
/** Class responsible that represents a material */
/*************************************************/
class gLTFMaterial
{
	public:
		gLTFMaterial (void);
		virtual ~gLTFMaterial (void) {};
		void out (void); // prints the content of the gLTFMaterial

		// Public member attributes
		std::string mName;
		PbrMetallicRoughness mPbrMetallicRoughness;
		NormalTexture mNormalTexture;
		OcclusionTexture mOcclusionTexture;
		EmissiveTexture mEmissiveTexture;
		Color3 mEmissiveFactor;
		std::string mAlphaMode;
		float mAlphaCutoff;
		bool mDoubleSided;
};

#endif
