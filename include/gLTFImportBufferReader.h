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
#ifndef __gLTFImportBufferReader_H__
#define __gLTFImportBufferReader_H__

#include <map>
#include "gLTFImportConstants.h"
#include "gLTFAccessor.h"

/** Class responsible for reading a gLTF (binary) Buffer */
class gLTFImportBufferReader
{
	public:
		gLTFImportBufferReader(void) {};
		virtual ~gLTFImportBufferReader(void) {};

		// Read scalar
		unsigned char readFromUnsignedByteBuffer (char* buffer, int count, gLTFAccessor accessor);
		unsigned short readFromUnsignedShortBuffer (char* buffer, int count, gLTFAccessor accessor);
		unsigned int readFromUnsignedIntBuffer (char* buffer, int count, gLTFAccessor accessor);

		// Read Vec2
		const Vec2Struct& readVec2FromUnsignedByteBuffer (char* buffer, int count, gLTFAccessor accessor);
		const Vec2Struct& readVec2FromUnsignedShortBuffer (char* buffer, int count, gLTFAccessor accessor);
		const Vec2Struct& readVec2FromFloatBuffer (char* buffer, int count, gLTFAccessor accessor);

		// Read Vec3
		const Vec3Struct& readVec3FromUnsignedByteBuffer (char* buffer, int count, gLTFAccessor accessor);
		const Vec3Struct& readVec3FromUnsignedShortBuffer (char* buffer, int count, gLTFAccessor accessor);
		const Vec3Struct& readVec3FromFloatBuffer (char* buffer, int count, gLTFAccessor accessor);

		// Read Vec4
		const Vec4Struct& readVec4FromUnsignedByteBuffer (char* buffer, int count, gLTFAccessor accessor);
		const Vec4Struct& readVec4FromUnsignedShortBuffer (char* buffer, int count, gLTFAccessor accessor);
		const Vec4Struct& readVec4FromFloatBuffer (char* buffer, int count, gLTFAccessor accessor);

		// Min/Max corrections
		void correctVec2StructWithMinMax (gLTFAccessor accessor, Vec2Struct* vec2Struct);
		void correctVec3StructWithMinMax (gLTFAccessor accessor, Vec3Struct* vec3Struct);
		void correctVec4StructWithMinMax (gLTFAccessor accessor, Vec4Struct* vec4Struct);

	//protected:

	private:
		Vec4Struct mHelperVec4Struct;
		Vec3Struct mHelperVec3Struct;
		Vec2Struct mHelperVec2Struct;
};

#endif
