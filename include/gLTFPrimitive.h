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
#ifndef __gLTFPrimitive_H__
#define __gLTFPrimitive_H__

#include <string>
#include <map>
#include <iostream>

/************************************************************************************************
 This file contains the data struture of a gLFTPrimitive. This is an intermediate structure
 used to generate an Ogre 3d mesh file.
/************************************************************************************************/

/**************************************************/
/** Class responsible that represents a Primitive */
/**************************************************/
class gLTFPrimitive
{
	public:
		enum Mode
		{
			POINTS = 0,
			LINES = 1,
			LINE_LOOP = 2,
			LINE_STRIP = 3,
			TRIANGLES = 4,
			TRIANGLE_STRIP = 5,
			TRIANGLE_FAN = 6
		};
		gLTFPrimitive (void);
		virtual ~gLTFPrimitive (void) {};
		void out (void); // prints the content of the gLTFPrimitive

		// Public members
		int mIndicesAccessor;
		int mMaterial;
		int mMode;
		std::map<std::string, int> mAttributes;
		// TODO: mTargets

		// Derived members
		std::string mMaterialName;
		int mPositionAccessor;
		int mNormalAccessor;
		int mTangentAccessor;
		int mTexcoord_0Accessor;
		int mTexcoord_1Accessor;
		int mColor_0Accessor;
		int mJoints_0Accessor;
		int mWeights_0Accessor;
};

#endif
