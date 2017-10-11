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
unsigned char gLTFImportBufferReader::readFromUnsignedByteBuffer(char* buffer, int count, gLTFAccessor accessor)
{
	unsigned char scalar;
	int unsignedCharSize = sizeof(unsigned char);
	memcpy(&scalar, &buffer[count * unsignedCharSize], unsignedCharSize);

	// Correct with min/max
	if (accessor.mMinAvailable)
		scalar = scalar < accessor.mMinInt[0] ? accessor.mMinInt[0] : scalar;
	if (accessor.mMaxAvailable)
		scalar = scalar > accessor.mMaxInt[0] ? accessor.mMaxInt[0] : scalar;
	return scalar;
}

//---------------------------------------------------------------------
unsigned short gLTFImportBufferReader::readFromUnsignedShortBuffer(char* buffer, int count, gLTFAccessor accessor)
{
	unsigned short scalar;
	int unsignedShortSize = sizeof(unsigned short);
	memcpy(&scalar, &buffer[count * unsignedShortSize], unsignedShortSize);

	// Correct with min/max
	if (accessor.mMinAvailable)
		scalar = scalar < (unsigned short)accessor.mMinInt[0] ? (unsigned short)accessor.mMinInt[0] : scalar;
	if (accessor.mMaxAvailable)
		scalar = scalar >(unsigned short)accessor.mMaxInt[0] ? (unsigned short)accessor.mMaxInt[0] : scalar;
	return scalar;
}

//---------------------------------------------------------------------
unsigned int gLTFImportBufferReader::readFromUnsignedIntBuffer(char* buffer, int count, gLTFAccessor accessor)
{
	unsigned int scalar;
	int unsignedIntSize = sizeof(unsigned int);
	memcpy(&scalar, &buffer[count * unsignedIntSize], unsignedIntSize);

	// Correct with min/max
	if (accessor.mMinAvailable)
		scalar = scalar < (unsigned int)accessor.mMinInt[0] ? (unsigned int)accessor.mMinInt[0] : scalar;
	if (accessor.mMaxAvailable)
		scalar = scalar >(unsigned int)accessor.mMaxInt[0] ? (unsigned int)accessor.mMaxInt[0] : scalar;
	return scalar;
}

//---------------------------------------------------------------------
const Vec2Struct& gLTFImportBufferReader::readVec2FromFloatBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	float fRaw;
	int floatSize = sizeof(float);
	int vec2Size = 2 * floatSize;
	memcpy(&fRaw, &buffer[count * vec2Size], floatSize);
	mHelperVec2Struct.u = fRaw;
	memcpy(&fRaw, &buffer[count * vec2Size + floatSize], floatSize);
	mHelperVec2Struct.v = fRaw;

	// Correct with min/max
	correctVec2StructWithMinMax(accessor, &mHelperVec2Struct);
	return mHelperVec2Struct;
}

//---------------------------------------------------------------------
const Vec2Struct& gLTFImportBufferReader::readVec2FromUnsignedByteBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	unsigned char raw;
	int unsignedCharSize = sizeof(unsigned char);
	int vec2Size = 2 * unsignedCharSize;
	memcpy(&raw, &buffer[count * vec2Size], unsignedCharSize);
	mHelperVec2Struct.u = raw;
	memcpy(&raw, &buffer[count * vec2Size + unsignedCharSize], unsignedCharSize);
	mHelperVec2Struct.v = raw;

	// Correct with min/max
	correctVec2StructWithMinMax(accessor, &mHelperVec2Struct);
	return mHelperVec2Struct;
}

//---------------------------------------------------------------------
const Vec2Struct& gLTFImportBufferReader::readVec2FromUnsignedShortBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	unsigned short raw;
	int unsignedShortSize = sizeof(unsigned short);
	int vec2Size = 2 * unsignedShortSize;
	memcpy(&raw, &buffer[count * vec2Size], unsignedShortSize);
	mHelperVec2Struct.u = raw;
	memcpy(&raw, &buffer[count * vec2Size + unsignedShortSize], unsignedShortSize);
	mHelperVec2Struct.v = raw;

	// Correct with min/max
	correctVec2StructWithMinMax(accessor, &mHelperVec2Struct);
	return mHelperVec2Struct;
}

//---------------------------------------------------------------------
const Vec3Struct& gLTFImportBufferReader::readVec3FromFloatBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	float fRaw;
	int floatSize = sizeof(float);
	int vec3Size = 3 * floatSize;
	memcpy(&fRaw, &buffer[count * vec3Size], floatSize);
	mHelperVec3Struct.x = fRaw;
	memcpy(&fRaw, &buffer[count * vec3Size + floatSize], floatSize);
	mHelperVec3Struct.y = fRaw;
	memcpy(&fRaw, &buffer[count * vec3Size + 2 * floatSize], floatSize);
	mHelperVec3Struct.z = fRaw;

	// Correct with min/max
	correctVec3StructWithMinMax(accessor, &mHelperVec3Struct);
	return mHelperVec3Struct;
}

//---------------------------------------------------------------------
const Vec3Struct& gLTFImportBufferReader::readVec3FromUnsignedByteBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	unsigned char raw;
	int unsingedCharSize = sizeof(unsigned char);
	int vec3Size = 3 * unsingedCharSize;
	memcpy(&raw, &buffer[count * vec3Size], unsingedCharSize);
	mHelperVec3Struct.x = raw;
	memcpy(&raw, &buffer[count * vec3Size + unsingedCharSize], unsingedCharSize);
	mHelperVec3Struct.y = raw;
	memcpy(&raw, &buffer[count * vec3Size + 2 * unsingedCharSize], unsingedCharSize);
	mHelperVec3Struct.z = raw;

	// Correct with min/max
	correctVec3StructWithMinMax(accessor, &mHelperVec3Struct);
	return mHelperVec3Struct;
}

//---------------------------------------------------------------------
const Vec3Struct& gLTFImportBufferReader::readVec3FromUnsignedShortBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	unsigned short raw;
	int unsingedShortSize = sizeof(unsigned short);
	int vec3Size = 3 * unsingedShortSize;
	memcpy(&raw, &buffer[count * vec3Size], unsingedShortSize);
	mHelperVec3Struct.x = raw;
	memcpy(&raw, &buffer[count * vec3Size + unsingedShortSize], unsingedShortSize);
	mHelperVec3Struct.y = raw;
	memcpy(&raw, &buffer[count * vec3Size + 2 * unsingedShortSize], unsingedShortSize);
	mHelperVec3Struct.z = raw;

	// Correct with min/max
	correctVec3StructWithMinMax(accessor, &mHelperVec3Struct);
	return mHelperVec3Struct;
}

//---------------------------------------------------------------------
const Vec4Struct& gLTFImportBufferReader::readVec4FromFloatBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	float fRaw;
	int floatSize = sizeof(float);
	int vec4Size = 4 * floatSize;
	memcpy(&fRaw, &buffer[count * vec4Size], floatSize);
	mHelperVec4Struct.r = fRaw;
	memcpy(&fRaw, &buffer[count * vec4Size + floatSize], floatSize);
	mHelperVec4Struct.g = fRaw;
	memcpy(&fRaw, &buffer[count * vec4Size + 2 * floatSize], floatSize);
	mHelperVec4Struct.b = fRaw;
	memcpy(&fRaw, &buffer[count * vec4Size + 3 * floatSize], floatSize);
	mHelperVec4Struct.a = fRaw;

	// Correct with min/max
	correctVec4StructWithMinMax(accessor, &mHelperVec4Struct);
	return mHelperVec4Struct;
}

//---------------------------------------------------------------------
const Vec4Struct& gLTFImportBufferReader::readVec4FromUnsignedByteBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	unsigned char raw;
	int unsignedByteSize = sizeof(unsigned char);
	int vec4Size = 4 * unsignedByteSize;
	memcpy(&raw, &buffer[count * vec4Size], unsignedByteSize);
	mHelperVec4Struct.r = raw;
	memcpy(&raw, &buffer[count * vec4Size + unsignedByteSize], unsignedByteSize);
	mHelperVec4Struct.g = raw;
	memcpy(&raw, &buffer[count * vec4Size + 2 * unsignedByteSize], unsignedByteSize);
	mHelperVec4Struct.b = raw;
	memcpy(&raw, &buffer[count * vec4Size + 3 * unsignedByteSize], unsignedByteSize);
	mHelperVec4Struct.a = raw;

	// Correct with min/max
	correctVec4StructWithMinMax(accessor, &mHelperVec4Struct);
	return mHelperVec4Struct;
}

//---------------------------------------------------------------------
const Vec4Struct& gLTFImportBufferReader::readVec4FromUnsignedShortBuffer(char* buffer,
	int count,
	gLTFAccessor accessor)
{
	unsigned short raw;
	int unsignedShortSize = sizeof(unsigned short);
	int vec4Size = 4 * unsignedShortSize;
	memcpy(&raw, &buffer[count * vec4Size], unsignedShortSize);
	mHelperVec4Struct.r = raw;
	memcpy(&raw, &buffer[count * vec4Size + unsignedShortSize], unsignedShortSize);
	mHelperVec4Struct.g = raw;
	memcpy(&raw, &buffer[count * vec4Size + 2 * unsignedShortSize], unsignedShortSize);
	mHelperVec4Struct.b = raw;
	memcpy(&raw, &buffer[count * vec4Size + 3 * unsignedShortSize], unsignedShortSize);
	mHelperVec4Struct.a = raw;

	// Correct with min/max
	correctVec4StructWithMinMax(accessor, &mHelperVec4Struct);
	return mHelperVec4Struct;
}

//---------------------------------------------------------------------
void gLTFImportBufferReader::correctVec2StructWithMinMax(gLTFAccessor accessor, Vec2Struct* vec2Struct)
{
	// Correct with min/max
	if (accessor.mMinAvailable)
	{
		vec2Struct->u = vec2Struct->u < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : vec2Struct->u;
		vec2Struct->v = vec2Struct->v < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : vec2Struct->v;
	}
	if (accessor.mMaxAvailable)
	{
		vec2Struct->u = vec2Struct->u > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : vec2Struct->u;
		vec2Struct->v = vec2Struct->v > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : vec2Struct->v;
	}
}

//---------------------------------------------------------------------
void gLTFImportBufferReader::correctVec3StructWithMinMax(gLTFAccessor accessor, Vec3Struct* vec3Struct)
{
	if (accessor.mMinAvailable)
	{
		vec3Struct->x = vec3Struct->x < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : vec3Struct->x;
		vec3Struct->y = vec3Struct->y < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : vec3Struct->y;
		vec3Struct->z = vec3Struct->z < accessor.mMinFloat[2] ? accessor.mMinFloat[2] : vec3Struct->z;
	}
	if (accessor.mMaxAvailable)
	{
		vec3Struct->x = vec3Struct->x > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : vec3Struct->x;
		vec3Struct->y = vec3Struct->y > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : vec3Struct->y;
		vec3Struct->z = vec3Struct->z > accessor.mMaxFloat[2] ? accessor.mMaxFloat[2] : vec3Struct->z;
	}
}

//---------------------------------------------------------------------
void gLTFImportBufferReader::correctVec4StructWithMinMax(gLTFAccessor accessor, Vec4Struct* vec4Struct)
{
	if (accessor.mMinAvailable)
	{
		vec4Struct->r = vec4Struct->r < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : vec4Struct->r;
		vec4Struct->g = vec4Struct->g < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : vec4Struct->g;
		vec4Struct->b = vec4Struct->b < accessor.mMinFloat[2] ? accessor.mMinFloat[2] : vec4Struct->b;
		vec4Struct->a = vec4Struct->a < accessor.mMinFloat[3] ? accessor.mMinFloat[3] : vec4Struct->a;
	}
	if (accessor.mMaxAvailable)
	{
		vec4Struct->r = vec4Struct->r > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : vec4Struct->r;
		vec4Struct->g = vec4Struct->g > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : vec4Struct->g;
		vec4Struct->b = vec4Struct->b > accessor.mMaxFloat[2] ? accessor.mMaxFloat[2] : vec4Struct->b;
		vec4Struct->a = vec4Struct->a > accessor.mMaxFloat[3] ? accessor.mMaxFloat[3] : vec4Struct->a;
	}
}