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
#ifndef __gLTFAccessor_H__
#define __gLTFAccessor_H__

#include <string>
#include <iostream>

/************************************************************************************************
 This file contains the data struture of a gLFTAccessor.
/************************************************************************************************/

/**************************************************/
/** Class responsible that represents an Accessor */
/**************************************************/
class gLTFAccessor
{
	public:
		enum ComponentType
		{
			BYTE = 5120,
			UNSIGNED_BYTE = 5121,
			SHORT = 5122,
			UNSIGNED_SHORT = 5123,
			UNSIGNED_INT = 5125,
			FLOAT = 5126
		};
		gLTFAccessor (void);
		virtual ~gLTFAccessor (void) {};
		void out (void); // prints the content of the gLTFAccessor

		// Public members
		int mComponentType; // default 0
		int mBufferView;
		int mByteOffset;
		bool mNormalized; // default false
		int mCount; // default 1
		std::string mType;
		int mMin; // default 0
		int mMax; // default 0
		// TODO: map of Sparse classes
		std::string mName;

		// Derived data from BufferView
		int mByteOffsetBufferViewDerived;
		int mBufferIndexDerived;
		int mByteLengthDerived;
		int mByteStrideDerived;
		int mTargetDerived;
		std::string mUriDerived; // uri of the Buffer
};

#endif
