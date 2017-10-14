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

#include "gLTFImportBufferReader.h"

//---------------------------------------------------------------------
unsigned char gLTFImportBufferReader::readFromUnsignedByteBuffer(char* buffer, int count, gLTFAccessor accessor, bool applyMinMax)
{
	unsigned char scalar;
	int unsignedCharSize = sizeof(unsigned char);
	memcpy(&scalar, &buffer[count * unsignedCharSize], unsignedCharSize);

	// Correct with min/max
	if (applyMinMax)
	{
		if (accessor.mMinAvailable)
			scalar = scalar < accessor.mMinInt[0] ? accessor.mMinInt[0] : scalar;
		if (accessor.mMaxAvailable)
			scalar = scalar > accessor.mMaxInt[0] ? accessor.mMaxInt[0] : scalar;
	}

	return scalar;
}

//---------------------------------------------------------------------
unsigned short gLTFImportBufferReader::readFromUnsignedShortBuffer(char* buffer, int count, gLTFAccessor accessor, bool applyMinMax)
{
	unsigned short scalar;
	int unsignedShortSize = sizeof(unsigned short);
	memcpy(&scalar, &buffer[count * unsignedShortSize], unsignedShortSize);

	// Correct with min/max
	if (applyMinMax)
	{
		if (accessor.mMinAvailable)
			scalar = scalar < (unsigned short)accessor.mMinInt[0] ? (unsigned short)accessor.mMinInt[0] : scalar;
		if (accessor.mMaxAvailable)
			scalar = scalar > (unsigned short)accessor.mMaxInt[0] ? (unsigned short)accessor.mMaxInt[0] : scalar;
	}
	
	return scalar;
}

//---------------------------------------------------------------------
unsigned int gLTFImportBufferReader::readFromUnsignedIntBuffer(char* buffer, int count, gLTFAccessor accessor, bool applyMinMax)
{
	unsigned int scalar;
	int unsignedIntSize = sizeof(unsigned int);
	memcpy(&scalar, &buffer[count * unsignedIntSize], unsignedIntSize);

	// Correct with min/max
	if (applyMinMax)
	{
		if (accessor.mMinAvailable)
			scalar = scalar < (unsigned int)accessor.mMinInt[0] ? (unsigned int)accessor.mMinInt[0] : scalar;
		if (accessor.mMaxAvailable)
			scalar = scalar > (unsigned int)accessor.mMaxInt[0] ? (unsigned int)accessor.mMaxInt[0] : scalar;
	}
	
	return scalar;
}

//---------------------------------------------------------------------
const Ogre::Vector2& gLTFImportBufferReader::readVec2FromFloatBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	float raw;
	int floatSize = sizeof(float);
	int vec2Size = 2 * floatSize;
	memcpy(&raw, &buffer[count * vec2Size], floatSize);
	mHelperVec2.x = raw;
	memcpy(&raw, &buffer[count * vec2Size + floatSize], floatSize);
	mHelperVec2.y = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec2WithMinMax(accessor, &mHelperVec2);
	
	return mHelperVec2;
}

//---------------------------------------------------------------------
const Ogre::Vector2& gLTFImportBufferReader::readVec2FromUnsignedByteBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	unsigned char raw;
	int unsignedCharSize = sizeof(unsigned char);
	int vec2Size = 2 * unsignedCharSize;
	memcpy(&raw, &buffer[count * vec2Size], unsignedCharSize);
	mHelperVec2.x = raw;
	memcpy(&raw, &buffer[count * vec2Size + unsignedCharSize], unsignedCharSize);
	mHelperVec2.y = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec2WithMinMax(accessor, &mHelperVec2);
	
	return mHelperVec2;
}

//---------------------------------------------------------------------
const Ogre::Vector2& gLTFImportBufferReader::readVec2FromUnsignedShortBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	unsigned short raw;
	int unsignedShortSize = sizeof(unsigned short);
	int vec2Size = 2 * unsignedShortSize;
	memcpy(&raw, &buffer[count * vec2Size], unsignedShortSize);
	mHelperVec2.x = raw;
	memcpy(&raw, &buffer[count * vec2Size + unsignedShortSize], unsignedShortSize);
	mHelperVec2.y = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec2WithMinMax(accessor, &mHelperVec2);
	
	return mHelperVec2;
}

//---------------------------------------------------------------------
const Ogre::Vector3& gLTFImportBufferReader::readVec3FromFloatBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	float raw;
	int floatSize = sizeof(float);
	int vec3Size = 3 * floatSize;
	memcpy(&raw, &buffer[count * vec3Size], floatSize);
	mHelperVec3.x = raw;
	memcpy(&raw, &buffer[count * vec3Size + floatSize], floatSize);
	mHelperVec3.y = raw;
	memcpy(&raw, &buffer[count * vec3Size + 2 * floatSize], floatSize);
	mHelperVec3.z = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec3WithMinMax(accessor, &mHelperVec3);
	
	return mHelperVec3;
}

//---------------------------------------------------------------------
const Ogre::Vector3& gLTFImportBufferReader::readVec3FromUnsignedByteBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	unsigned char raw;
	int unsingedCharSize = sizeof(unsigned char);
	int vec3Size = 3 * unsingedCharSize;
	memcpy(&raw, &buffer[count * vec3Size], unsingedCharSize);
	mHelperVec3.x = raw;
	memcpy(&raw, &buffer[count * vec3Size + unsingedCharSize], unsingedCharSize);
	mHelperVec3.y = raw;
	memcpy(&raw, &buffer[count * vec3Size + 2 * unsingedCharSize], unsingedCharSize);
	mHelperVec3.z = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec3WithMinMax(accessor, &mHelperVec3);
	
	return mHelperVec3;
}

//---------------------------------------------------------------------
const Ogre::Vector3& gLTFImportBufferReader::readVec3FromUnsignedShortBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	unsigned short raw;
	int unsingedShortSize = sizeof(unsigned short);
	int vec3Size = 3 * unsingedShortSize;
	memcpy(&raw, &buffer[count * vec3Size], unsingedShortSize);
	mHelperVec3.x = raw;
	memcpy(&raw, &buffer[count * vec3Size + unsingedShortSize], unsingedShortSize);
	mHelperVec3.y = raw;
	memcpy(&raw, &buffer[count * vec3Size + 2 * unsingedShortSize], unsingedShortSize);
	mHelperVec3.z = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec3WithMinMax(accessor, &mHelperVec3);
	
	return mHelperVec3;
}

//---------------------------------------------------------------------
const Ogre::Vector4& gLTFImportBufferReader::readVec4FromFloatBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	float raw;
	int floatSize = sizeof(float);
	int vec4Size = 4 * floatSize;
	memcpy(&raw, &buffer[count * vec4Size], floatSize);
	mHelperVec4.x = raw;
	memcpy(&raw, &buffer[count * vec4Size + floatSize], floatSize);
	mHelperVec4.y = raw;
	memcpy(&raw, &buffer[count * vec4Size + 2 * floatSize], floatSize);
	mHelperVec4.z = raw;
	memcpy(&raw, &buffer[count * vec4Size + 3 * floatSize], floatSize);
	mHelperVec4.w = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec4WithMinMax(accessor, &mHelperVec4);
	
	return mHelperVec4;
}

//---------------------------------------------------------------------
const Ogre::Vector4& gLTFImportBufferReader::readVec4FromUnsignedByteBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	unsigned char raw;
	int unsignedByteSize = sizeof(unsigned char);
	int vec4Size = 4 * unsignedByteSize;
	memcpy(&raw, &buffer[count * vec4Size], unsignedByteSize);
	mHelperVec4.x = raw;
	memcpy(&raw, &buffer[count * vec4Size + unsignedByteSize], unsignedByteSize);
	mHelperVec4.y = raw;
	memcpy(&raw, &buffer[count * vec4Size + 2 * unsignedByteSize], unsignedByteSize);
	mHelperVec4.z = raw;
	memcpy(&raw, &buffer[count * vec4Size + 3 * unsignedByteSize], unsignedByteSize);
	mHelperVec4.w = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec4WithMinMax(accessor, &mHelperVec4);
	
	return mHelperVec4;
}

//---------------------------------------------------------------------
const Ogre::Vector4& gLTFImportBufferReader::readVec4FromUnsignedShortBuffer(char* buffer,
	int count,
	gLTFAccessor accessor, 
	bool applyMinMax)
{
	unsigned short raw;
	int unsignedShortSize = sizeof(unsigned short);
	int vec4Size = 4 * unsignedShortSize;
	memcpy(&raw, &buffer[count * vec4Size], unsignedShortSize);
	mHelperVec4.x = raw;
	memcpy(&raw, &buffer[count * vec4Size + unsignedShortSize], unsignedShortSize);
	mHelperVec4.y = raw;
	memcpy(&raw, &buffer[count * vec4Size + 2 * unsignedShortSize], unsignedShortSize);
	mHelperVec4.z = raw;
	memcpy(&raw, &buffer[count * vec4Size + 3 * unsignedShortSize], unsignedShortSize);
	mHelperVec4.w = raw;

	// Correct with min/max
	if (applyMinMax)
		correctVec4WithMinMax(accessor, &mHelperVec4);
	
	return mHelperVec4;
}

//---------------------------------------------------------------------
void gLTFImportBufferReader::correctVec2WithMinMax(gLTFAccessor accessor, Ogre::Vector2* vec2)
{
	// Correct with min/max
	if (accessor.mMinAvailable)
	{
		vec2->x = vec2->x < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : vec2->x;
		vec2->y = vec2->y < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : vec2->y;
	}
	if (accessor.mMaxAvailable)
	{
		vec2->x = vec2->x > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : vec2->x;
		vec2->y = vec2->y > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : vec2->y;
	}
}

//---------------------------------------------------------------------
void gLTFImportBufferReader::correctVec3WithMinMax(gLTFAccessor accessor, Ogre::Vector3* vec3)
{
	if (accessor.mMinAvailable)
	{
		vec3->x = vec3->x < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : vec3->x;
		vec3->y = vec3->y < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : vec3->y;
		vec3->z = vec3->z < accessor.mMinFloat[2] ? accessor.mMinFloat[2] : vec3->z;
	}
	if (accessor.mMaxAvailable)
	{
		vec3->x = vec3->x > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : vec3->x;
		vec3->y = vec3->y > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : vec3->y;
		vec3->z = vec3->z > accessor.mMaxFloat[2] ? accessor.mMaxFloat[2] : vec3->z;
	}
}

//---------------------------------------------------------------------
void gLTFImportBufferReader::correctVec4WithMinMax(gLTFAccessor accessor, Ogre::Vector4* vec4)
{
	if (accessor.mMinAvailable)
	{
		vec4->x = vec4->x < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : vec4->x;
		vec4->y = vec4->y < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : vec4->y;
		vec4->z = vec4->z < accessor.mMinFloat[2] ? accessor.mMinFloat[2] : vec4->z;
		vec4->w = vec4->w < accessor.mMinFloat[3] ? accessor.mMinFloat[3] : vec4->w;
	}
	if (accessor.mMaxAvailable)
	{
		vec4->x = vec4->x > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : vec4->x;
		vec4->y = vec4->y > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : vec4->y;
		vec4->z = vec4->z > accessor.mMaxFloat[2] ? accessor.mMaxFloat[2] : vec4->z;
		vec4->w = vec4->w > accessor.mMaxFloat[3] ? accessor.mMaxFloat[3] : vec4->w;
	}
}