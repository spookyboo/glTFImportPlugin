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

#include "gLTFImportOgreMeshCreator.h"
#include "rapidjson/document.h"
#include "base64.h"

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createOgreMeshFiles (Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFMesh> meshesMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer)
{
	OUT << "\nPerform gLTFImportOgreMeshCreator::createOgreMeshFiles\n";
	OUT << "------------------------------------------------------\n";
	
	// Create the Ogre mesh xml files (*.xml)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Iterate through meshes and create for each mesh an Ogre .xml file
	std::map<int, gLTFMesh>::iterator it;
	std::map<int, gLTFPrimitive>::iterator itPrimitives;
	gLTFMesh mesh;
	gLTFPrimitive primitive;
	std::string materialName;
	std::string ogreFullyQualifiedMeshXmlFileName;
	for (it = meshesMap.begin(); it != meshesMap.end(); it++)
	{
		mesh = it->second;
		ogreFullyQualifiedMeshXmlFileName = fullyQualifiedImportPath + mesh.mName + ".xml";
		OUT << "Create: mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

		// Create the file
		std::ofstream dst(ogreFullyQualifiedMeshXmlFileName);
		
		// Add xml content
		dst << "<mesh>\n";
		dst << TAB << "<submeshes>\n";

		// Iterate through primitives (each primitive is a submesh)
		for (itPrimitives = mesh.mPrimitiveMap.begin(); itPrimitives != mesh.mPrimitiveMap.end(); itPrimitives++)
		{
			primitive = itPrimitives->second;
			materialName = primitive.mMaterialNameDerived;
			if (materialName == "")
				materialName = "BaseWhite";

			/* Write submesh definition
			 * Do not share vertices. It is not clear whether gLTF shares geometry data, so use the default 'false' value,
			 * until proved otherwise.
			 * We always use 32 bit incices (no 16 bit), even if the gLTF file itself uses 16 bits
			 */
			dst << TABx2 << "<submesh material = \"" + primitive.mMaterialNameDerived + "\"";
			dst << " usesharedvertices = \"false\" use32bitindexes = \"false\" "; // TODO: Hardcoded?

			// Write operation type
			switch(primitive.mMode)
			{
				case gLTFPrimitive::POINTS:
					dst << "operationtype = \"point_list\">\n";
					break;
				case gLTFPrimitive::LINES:
					dst << "operationtype = \"line_list\">\n";
					break;
				case gLTFPrimitive::LINE_LOOP:
					dst << "operationtype = \"line_list\">\n";
					break;
				case gLTFPrimitive::LINE_STRIP:
					dst << "operationtype = \"triangle_list\">\n";
					break;
				case gLTFPrimitive::TRIANGLES:
					dst << "operationtype = \"triangle_list\">\n";
					break;
				case gLTFPrimitive::TRIANGLE_STRIP:
					dst << "operationtype = \"triangle_strip\">\n";
					break;
				case gLTFPrimitive::TRIANGLE_FAN:
					dst << "operationtype = \"triangle_fan\">\n";
					break;
			}

			// Write faces
			if (primitive.mIndicesAccessor > -1)
			{
				writeFaces (dst, primitive, accessorMap, data, startBinaryBuffer);
			}

			// Write geometry
			if (primitive.mPositionAccessorDerived > -1)
			{
				gLTFAccessor  accessor = accessorMap[primitive.mPositionAccessorDerived];
				dst << TABx3 << "<geometry vertexcount=\"" << accessor.mCount << "\">\n";

				// Write vertexbuffer header
				std::string hasPositionsText = "\"true\""; // Assume there are always positions, right?
				std::string hasNormalsText = "\"true\"";
				std::string hasTangentsText = "\"true\"";
				//std::string numTextCoordsText = "\"1\""; // Assume there is at least one
				std::string numTextCoordsText = "\"0\"";
				if (primitive.mNormalAccessorDerived < 0)
					hasNormalsText = "\"false\"";
				if (primitive.mTangentAccessorDerived < 0)
					hasTangentsText = "\"false\"";
				if (primitive.mTexcoord_0AccessorDerived > -1)
					numTextCoordsText = "\"1\"";
				if (primitive.mTexcoord_1AccessorDerived > -1)
					numTextCoordsText = "\"2\"";

				dst << TABx4 << "<vertexbuffer positions = " << hasPositionsText <<
					" normals = " << hasNormalsText;

				// Colours
				if (primitive.mColor_0AccessorDerived > -1)
					dst << " colours_diffuse = \"true\"";

				// Texture coordinate dimensions (assume float2 for now)
				if (primitive.mTexcoord_0AccessorDerived > -1)
					dst << " texture_coord_dimensions_0 = \"float2\"";
				if (primitive.mTexcoord_1AccessorDerived > -1)
					dst << " texture_coord_dimensions_1 = \"float2\"";

				// Tangents
				dst << " tangents = " << hasTangentsText <<
				" texture_coords = " << numTextCoordsText <<
				">\n";

				// Write vertices
				writeVertices(dst, primitive, accessorMap, data, startBinaryBuffer);

				// Closing tags
				dst << TABx4 << "</vertexbuffer>\n";
				dst << TABx3 << "</geometry>\n";
			}
			// TODO: Finsih it !

			dst << TABx2 << "</submesh>\n";
		}

		dst << TAB << "</submeshes>\n";
		dst << "</mesh>\n";

		dst.close();
	}

	OUT << "Written mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeFaces (std::ofstream& dst,
	const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	gLTFAccessor  indicesAccessor = accessorMap[primitive.mIndicesAccessor];
	if (indicesAccessor.mCount == 0)
		return false;

	// Read indices
	readIndicesFromUriOrFile (primitive, accessorMap, data, startBinaryBuffer);

	// Write indices
	dst << TABx3 << "<faces count = \"" << indicesAccessor.mCount / 3 << "\">\n";
	
	for (int i = 0; i < indicesAccessor.mCount; i+=3)
	{
		dst << TABx4 << "<face v1 = \"" << mIndicesMap[i + 0] <<
			"\" v2 = \"" << mIndicesMap[i + 1] <<
			"\" v3 = \"" << mIndicesMap[i + 2] << "\" />\n";

		// TODO: swap around indices, convert ccw to cw for front face??????
		/*
		dst << TABx4 << "<face v1 = \"" << mIndicesMap[i] <<
			"\" v2 = \"" << mIndicesMap[i + 1] <<
			"\" v3 = \"" << mIndicesMap[i + 2] << "\" />\n";
		*/
	}

	// Closing tag
	dst << TABx3 << "</faces>\n";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeVertices (std::ofstream& dst, 
	const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Read positions, normals, tangents,... etc.
	readPositionsFromUriOrFile(primitive, accessorMap, data, startBinaryBuffer); // Read the positions
	readNormalsFromUriOrFile(primitive, accessorMap, data, startBinaryBuffer); // Read the normals
	readTangentsFromUriOrFile(primitive, accessorMap, data, startBinaryBuffer); // Read the tangents
	readColorsFromUriOrFile(primitive, accessorMap, data, startBinaryBuffer); // Read the diffuse colours
	readTexCoords0FromUriOrFile(primitive, accessorMap, data, startBinaryBuffer); // Read the uv's set 0
	readTexCoords1FromUriOrFile(primitive, accessorMap, data, startBinaryBuffer); // Read the uv's set 1

	// Write vertices; Assume that count of positions, texcoords, etc. is the same
	// TODO: Verify !!!!!!!!!!
	gLTFAccessor  positionAccessor = accessorMap[primitive.mPositionAccessorDerived];
	for (int i = 0; i < positionAccessor.mCount; i++)
	{
		// Open vertex
		dst << TABx5 << "<vertex>\n";
		
		// Position
		Vec3Struct vec3 = mPositionsMap[i];
		dst << TABx6 << "<position x=\"" << vec3.x << "\" y=\"" << vec3.y << "\" z=\"" << vec3.z << "\" />\n";

		// Normal
		if (mNormalsMap.size() > 0)
		{
			vec3 = mNormalsMap[i];
			dst << TABx6 << "<normal x=\"" << vec3.x << "\" y=\"" << vec3.y << "\" z=\"" << vec3.z << "\" />\n";
		}

		// Tangent
		if (mTangentsMap.size() > 0)
		{
			vec3 = mTangentsMap[i];
			dst << TABx6 << "<tangent x=\"" << vec3.x << "\" y=\"" << vec3.y << "\" z=\"" << vec3.z << "\" />\n";
		}

		// Diffuse color
		if (mColor_0AccessorMap.size() > 0)
		{
			Vec4Struct vec4 = mColor_0AccessorMap[i];
			dst << TABx6 << "<colour_diffuse value=\"" << vec4.r << " " <<
				vec4.g << " " <<
				vec4.b << " " <<
				vec4.a << "\" />\n";
		}

		// Texcoord 0
		if (mTexcoords_0Map.size() > 0)
		{
			Vec2Struct vec2 = mTexcoords_0Map[i];
			dst << TABx6 << "<texcoord u=\"" << vec2.u << "\" v=\"" << vec2.v << "\" />\n";
		}

		// Texcoord 1
		if (mTexcoords_1Map.size() > 0)
		{
			Vec2Struct vec2 = mTexcoords_1Map[i];
			dst << TABx6 << "<texcoord u=\"" << vec2.u << "\" v=\"" << vec2.v << "\" />\n";
		}

		// Close vertex
		dst << TABx5 << "</vertex>\n";
	}

	return true;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readPositionsFromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	gLTFAccessor  positionAccessor = accessorMap[primitive.mPositionAccessorDerived];
	char* buffer = getBufferChunk(positionAccessor.mUriDerived, data, positionAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mPositionsMap.clear();
	for (int i = 0; i < positionAccessor.mCount; i++)
	{
		// A position must be a VEC3/Float, otherwise it doesn't get read
		if (positionAccessor.mType == "VEC3" && positionAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec3Struct pos = readVec3FromBuffer(buffer, i, positionAccessor);
			mPositionsMap[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readNormalsFromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	if (primitive.mNormalAccessorDerived < 0)
		return;

	gLTFAccessor  normalAccessor = accessorMap[primitive.mNormalAccessorDerived];
	char* buffer = getBufferChunk(normalAccessor.mUriDerived, data, normalAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mNormalsMap.clear();
	for (int i = 0; i < normalAccessor.mCount; i++)
	{
		// A normal  must be a VEC3/Float, otherwise it doesn't get read
		if (normalAccessor.mType == "VEC3" && normalAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec3Struct pos = readVec3FromBuffer(buffer, i, normalAccessor);
			mNormalsMap[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readTangentsFromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	if (primitive.mTangentAccessorDerived < 0)
		return;

	gLTFAccessor  tangentAccessor = accessorMap[primitive.mTangentAccessorDerived];
	char* buffer = getBufferChunk(tangentAccessor.mUriDerived, data, tangentAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mTangentsMap.clear();
	for (int i = 0; i < tangentAccessor.mCount; i++)
	{
		// A tangent must be a VEC3/Float, otherwise it doesn't get read
		if (tangentAccessor.mType == "VEC3" && tangentAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec3Struct pos = readVec3FromBuffer(buffer, i, tangentAccessor);
			mTangentsMap[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readColorsFromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read colours
	if (primitive.mColor_0AccessorDerived < 0)
		return;

	gLTFAccessor  mColor_0Accessor = accessorMap[primitive.mColor_0AccessorDerived];
	char* buffer = getBufferChunk(mColor_0Accessor.mUriDerived, data, mColor_0Accessor, startBinaryBuffer);

	// Iterate through the chunk
	mColor_0AccessorMap.clear();
	for (int i = 0; i < mColor_0Accessor.mCount; i++)
	{
		// A colour can be a VEC3 (Float)
		if (mColor_0Accessor.mType == "VEC3")
		{
			Vec3Struct v3 = readVec3FromBuffer(buffer, i, mColor_0Accessor);
			Vec4Struct col;
			col.r = v3.x;
			col.g = v3.y;
			col.b = v3.z;
			col.a = 1.0f;
			mColor_0AccessorMap[i] = col;
		}
		else if (mColor_0Accessor.mType == "VEC4")
		{
			Vec4Struct col = readVec4FromBuffer(buffer, i, mColor_0Accessor);
			mColor_0AccessorMap[i] = col;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readIndicesFromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read indices
	if (primitive.mIndicesAccessor < 0)
		return;

	gLTFAccessor indicesAccessor = accessorMap[primitive.mIndicesAccessor];
	char* buffer = getBufferChunk(indicesAccessor.mUriDerived, data, indicesAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mIndicesMap.clear();
	if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = readUnsignedByteFromBuffer(buffer, i, indicesAccessor);
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = readUnsignedShortFromBuffer(buffer, i, indicesAccessor);
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_INT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = readUnsignedIntFromBuffer(buffer, i, indicesAccessor);
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readTexCoords0FromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	if (primitive.mTexcoord_0AccessorDerived < 0)
		return;

	gLTFAccessor  mTexcoord_0Accessor = accessorMap[primitive.mTexcoord_0AccessorDerived];
	char* buffer = getBufferChunk(mTexcoord_0Accessor.mUriDerived, data, mTexcoord_0Accessor, startBinaryBuffer);

	// Iterate through the chunk
	mTexcoords_0Map.clear();
	for (int i = 0; i < mTexcoord_0Accessor.mCount; i++)
	{
		// A position must be a VEC3/Float, otherwise it doesn't get read
		if (mTexcoord_0Accessor.mType == "VEC2" && mTexcoord_0Accessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec2Struct pos = readVec2FromBuffer (buffer, i, mTexcoord_0Accessor);
			mTexcoords_0Map[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readTexCoords1FromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	if (primitive.mTexcoord_1AccessorDerived < 0)
		return;

	gLTFAccessor  mTexcoord_1Accessor = accessorMap[primitive.mTexcoord_1AccessorDerived];
	char* buffer = getBufferChunk(mTexcoord_1Accessor.mUriDerived, data, mTexcoord_1Accessor, startBinaryBuffer);

	// Iterate through the chunk
	mTexcoords_1Map.clear();
	for (int i = 0; i < mTexcoord_1Accessor.mCount; i++)
	{
		// A position must be a VEC3/Float, otherwise it doesn't get read
		if (mTexcoord_1Accessor.mType == "VEC2" && mTexcoord_1Accessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec2Struct pos = readVec2FromBuffer(buffer, i, mTexcoord_1Accessor);
			mTexcoords_1Map[i] = pos;
		}
	}

	delete[] buffer;
}
//---------------------------------------------------------------------
char* gLTFImportOgreMeshCreator::getBufferChunk (const std::string& uri, 
	Ogre::HlmsEditorPluginData* data, 
	gLTFAccessor accessor, 
	int startBinaryBuffer)
{
	char* buffer;
	if (isUriEmbeddedBase64(uri))
	{
		// Get the binary data from a base64 encoded string
		std::string imageBuffer = getEmbeddedBase64FromUri(uri); // Get the encoded data
		imageBuffer = base64_decode(imageBuffer); // Decode it

		// Read the data from the string
		std::istringstream iss(imageBuffer);

		// Set the stream to the beginning of the binary chunk; ignore startBinaryBuffer (it should be 0 afterall)
		iss.seekg(accessor.mByteOffset + accessor.mByteOffsetBufferViewDerived, std::ios::beg);

		// Read the buffer
		buffer = new char[accessor.mByteLengthDerived]; // Don not forget to delete[] the buffer
		iss.read(buffer, accessor.mByteLengthDerived);
	}
	else
	{
		std::string fileNameBufferHelper = "";
		if (isFilePathAbsolute(uri))
			fileNameBufferHelper = uri;
		else
			fileNameBufferHelper = data->mInFileDialogPath + uri;

		// Read the binary data
		std::ifstream ifs(fileNameBufferHelper, std::ios::binary);

		// Set the stream to the beginning of the binary chunk
		ifs.seekg(startBinaryBuffer + accessor.mByteOffset + accessor.mByteOffsetBufferViewDerived, std::ios::beg);

		// Read the buffer
		buffer = new char[accessor.mByteLengthDerived]; // Don not forget to delete[] the buffer
		ifs.read(buffer, accessor.mByteLengthDerived);
		ifs.close();
	}
	
	return buffer;
}

//---------------------------------------------------------------------
unsigned char gLTFImportOgreMeshCreator::readUnsignedByteFromBuffer (char* buffer, int count, gLTFAccessor accessor)
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
unsigned short gLTFImportOgreMeshCreator::readUnsignedShortFromBuffer (char* buffer, int count, gLTFAccessor accessor)
{
	unsigned short scalar;
	int unsignedShortSize = sizeof(unsigned short);
	memcpy(&scalar, &buffer[count * unsignedShortSize], unsignedShortSize);

	// Correct with min/max
	if (accessor.mMinAvailable)
		scalar = scalar < accessor.mMinInt[0] ? accessor.mMinInt[0] : scalar;
	if (accessor.mMaxAvailable)
		scalar = scalar > accessor.mMaxInt[0] ? accessor.mMaxInt[0] : scalar;
	return scalar;
}

//---------------------------------------------------------------------
unsigned int gLTFImportOgreMeshCreator::readUnsignedIntFromBuffer (char* buffer, int count, gLTFAccessor accessor)
{
	unsigned int scalar;
	int unsignedIntSize = sizeof(unsigned int);
	memcpy(&scalar, &buffer[count * unsignedIntSize], unsignedIntSize);

	// Correct with min/max
	if (accessor.mMinAvailable)
		scalar = scalar < accessor.mMinInt[0] ? accessor.mMinInt[0] : scalar;
	if (accessor.mMaxAvailable)
		scalar = scalar > accessor.mMaxInt[0] ? accessor.mMaxInt[0] : scalar;
	return scalar;
}

//---------------------------------------------------------------------
const gLTFImportOgreMeshCreator::Vec2Struct& gLTFImportOgreMeshCreator::readVec2FromBuffer (char* buffer, 
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
	if (accessor.mMinAvailable)
	{
		mHelperVec2Struct.u = mHelperVec2Struct.u < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : mHelperVec2Struct.u;
		mHelperVec2Struct.v = mHelperVec2Struct.v < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : mHelperVec2Struct.v;
	}
	if (accessor.mMaxAvailable)
	{
		mHelperVec2Struct.u = mHelperVec2Struct.u > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : mHelperVec2Struct.u;
		mHelperVec2Struct.v = mHelperVec2Struct.v > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : mHelperVec2Struct.v;
	}
	return mHelperVec2Struct;
}

//---------------------------------------------------------------------
const gLTFImportOgreMeshCreator::Vec3Struct& gLTFImportOgreMeshCreator::readVec3FromBuffer (char* buffer, 
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
	if (accessor.mMinAvailable)
	{
		mHelperVec3Struct.x = mHelperVec3Struct.x < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : mHelperVec3Struct.x;
		mHelperVec3Struct.y = mHelperVec3Struct.y < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : mHelperVec3Struct.y;
		mHelperVec3Struct.z = mHelperVec3Struct.z < accessor.mMinFloat[2] ? accessor.mMinFloat[2] : mHelperVec3Struct.z;
	}
	if (accessor.mMaxAvailable)
	{
		mHelperVec3Struct.x = mHelperVec3Struct.x > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : mHelperVec3Struct.x;
		mHelperVec3Struct.y = mHelperVec3Struct.y > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : mHelperVec3Struct.y;
		mHelperVec3Struct.z = mHelperVec3Struct.z > accessor.mMaxFloat[2] ? accessor.mMaxFloat[2] : mHelperVec3Struct.z;
	}
	return mHelperVec3Struct;
}

//---------------------------------------------------------------------
const gLTFImportOgreMeshCreator::Vec4Struct& gLTFImportOgreMeshCreator::readVec4FromBuffer(char* buffer,
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
	if (accessor.mMinAvailable)
	{
		mHelperVec4Struct.r = mHelperVec4Struct.r < accessor.mMinFloat[0] ? accessor.mMinFloat[0] : mHelperVec4Struct.r;
		mHelperVec4Struct.g = mHelperVec4Struct.g < accessor.mMinFloat[1] ? accessor.mMinFloat[1] : mHelperVec4Struct.g;
		mHelperVec4Struct.b = mHelperVec4Struct.b < accessor.mMinFloat[2] ? accessor.mMinFloat[2] : mHelperVec4Struct.b;
		mHelperVec4Struct.a = mHelperVec4Struct.a < accessor.mMinFloat[3] ? accessor.mMinFloat[3] : mHelperVec4Struct.a;
	}
	if (accessor.mMaxAvailable)
	{
		mHelperVec4Struct.r = mHelperVec4Struct.r > accessor.mMaxFloat[0] ? accessor.mMaxFloat[0] : mHelperVec4Struct.r;
		mHelperVec4Struct.g = mHelperVec4Struct.g > accessor.mMaxFloat[1] ? accessor.mMaxFloat[1] : mHelperVec4Struct.g;
		mHelperVec4Struct.b = mHelperVec4Struct.b > accessor.mMaxFloat[2] ? accessor.mMaxFloat[2] : mHelperVec4Struct.b;
		mHelperVec4Struct.a = mHelperVec4Struct.a > accessor.mMaxFloat[3] ? accessor.mMaxFloat[3] : mHelperVec4Struct.a;
	}
	return mHelperVec4Struct;
}
