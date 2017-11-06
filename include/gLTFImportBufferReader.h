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
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreMatrix4.h"

/** Class responsible for reading a gLTF (binary) Buffer */
class gLTFImportBufferReader
{
	public:
		gLTFImportBufferReader(void) {};
		virtual ~gLTFImportBufferReader(void) {};

		// Read scalar
		unsigned char readFromUnsignedByteBuffer (char* buffer, 
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		unsigned short readFromUnsignedShortBuffer (char* buffer, 
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		unsigned int readFromUnsignedIntBuffer (char* buffer, 
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		float readFromFloatBuffer (char* buffer,
			int count,
			gLTFAccessor accessor,
			bool applyMinMax = false);

		// Read Vec2
		const Ogre::Vector2& readVec2FromUnsignedByteBuffer (char* buffer, 
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		const Ogre::Vector2& readVec2FromUnsignedShortBuffer (char* buffer,
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		const Ogre::Vector2& readVec2FromFloatBuffer (char* buffer,
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);

		// Read Vec3
		const Ogre::Vector3& readVec3FromUnsignedByteBuffer (char* buffer,
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		const Ogre::Vector3& readVec3FromUnsignedShortBuffer (char* buffer,
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		const Ogre::Vector3& readVec3FromFloatBuffer (char* buffer,
			int count, 
			gLTFAccessor accessor, 
			bool applyMinMax = false);
		const Ogre::Vector3& skipAndReadVec3FromFloatBuffer (char* buffer,
			unsigned int skipBytes,
			gLTFAccessor accessor,
			bool applyMinMax);

		// Read Vec4
		const Ogre::Vector4& readVec4FromUnsignedByteBuffer (char* buffer, int count, gLTFAccessor accessor, bool applyMinMax = false);
		const Ogre::Vector4& readVec4FromUnsignedShortBuffer (char* buffer, int count, gLTFAccessor accessor, bool applyMinMax = false);
		const Ogre::Vector4& readVec4FromFloatBuffer (char* buffer, int count, gLTFAccessor accessor, bool applyMinMax = false);
		const Ogre::Vector4& skipAndReadVec4FromFloatBuffer (char* buffer, unsigned int skipBytes, gLTFAccessor accessor, bool applyMinMax = false);

		// Read Matrix4
		const Ogre::Matrix4& readMatrix4FromFloatBuffer (char* buffer, int count, gLTFAccessor accessor, bool applyMinMax = false);

		// Min/Max corrections
		void correctVec2WithMinMax (gLTFAccessor accessor, Ogre::Vector2* vec2);
		void correctVec3WithMinMax (gLTFAccessor accessor, Ogre::Vector3* vec3);
		void correctVec4WithMinMax (gLTFAccessor accessor, Ogre::Vector4* vec4);

	//protected:

	private:
		Ogre::Matrix4 mHelperMat4;
		Ogre::Vector4 mHelperVec4;
		Ogre::Vector3 mHelperVec3;
		Ogre::Vector2 mHelperVec2;
};

#endif
