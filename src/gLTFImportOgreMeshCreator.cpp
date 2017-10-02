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

			// Write submesh definition
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
				std::string numTextCoordsText = "\"1\""; // Assume there is at least one
				if (primitive.mNormalAccessorDerived < 0)
					hasNormalsText = "\"false\"";
				if (primitive.mTangentAccessorDerived < 0)
					hasTangentsText = "\"false\"";
				if (primitive.mTexcoord_1AccessorDerived > -1)
					numTextCoordsText = "\"2\"";

				dst << TABx4 << "<vertexbuffer positions = " << hasPositionsText <<
					" normals = " << hasNormalsText <<
					" texture_coord_dimensions_0 = \"float2\"" <<
					" tangents = " << hasTangentsText <<
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
	readIndicesFromFile (primitive, accessorMap, data, startBinaryBuffer);

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
	readPositionsFromFile (primitive, accessorMap, data, startBinaryBuffer); // Read the positions
	readNormalsFromFile (primitive, accessorMap, data, startBinaryBuffer); // Read the normals
	readTangentsFromFile (primitive, accessorMap, data, startBinaryBuffer); // Read the tangents
	readTexCoords0FromFile (primitive, accessorMap, data, startBinaryBuffer); // Read the uv's

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
		vec3 = mNormalsMap[i];
		dst << TABx6 << "<normal x=\"" << vec3.x << "\" y=\"" << vec3.y << "\" z=\"" << vec3.z << "\" />\n";

		// Tangent
		vec3 = mTangentsMap[i];
		dst << TABx6 << "<tangent x=\"" << vec3.x << "\" y=\"" << vec3.y << "\" z=\"" << vec3.z << "\" />\n";

		// Texcoord
		Vec2Struct vec2 = mTexcoords_0Map[i];
		dst << TABx6 << "<texcoord u=\"" << vec2.u << "\" v=\"" << vec2.v << "\" />\n";

		// Close vertex
		dst << TABx5 << "</vertex>\n";
	}

	return true;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readPositionsFromFile(const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	gLTFAccessor  positionAccessor = accessorMap[primitive.mPositionAccessorDerived];
	std::string fileName = getFileNameBufferFile(positionAccessor.mUriDerived, data);
	char* buffer = getBufferChunk(fileName, positionAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mPositionsMap.clear();
	for (int i = 0; i < positionAccessor.mCount; i++)
	{
		// A position must be a VEC3/Float, otherwise it doesn't get read
		if (positionAccessor.mType == "VEC3" && positionAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec3Struct pos = readVec3FromBuffer(buffer, i);
			mPositionsMap[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readNormalsFromFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	gLTFAccessor  normalAccessor = accessorMap[primitive.mNormalAccessorDerived];
	std::string fileName = getFileNameBufferFile(normalAccessor.mUriDerived, data);
	char* buffer = getBufferChunk(fileName, normalAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mNormalsMap.clear();
	for (int i = 0; i < normalAccessor.mCount; i++)
	{
		// A normal  must be a VEC3/Float, otherwise it doesn't get read
		if (normalAccessor.mType == "VEC3" && normalAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec3Struct pos = readVec3FromBuffer(buffer, i);
			mNormalsMap[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readTangentsFromFile(const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	gLTFAccessor  tangentAccessor = accessorMap[primitive.mTangentAccessorDerived];
	std::string fileName = getFileNameBufferFile(tangentAccessor.mUriDerived, data);
	char* buffer = getBufferChunk(fileName, tangentAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mTangentsMap.clear();
	for (int i = 0; i < tangentAccessor.mCount; i++)
	{
		// A tangent must be a VEC3/Float, otherwise it doesn't get read
		if (tangentAccessor.mType == "VEC3" && tangentAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec3Struct pos = readVec3FromBuffer(buffer, i);
			mTangentsMap[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readIndicesFromFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read indices
	gLTFAccessor indicesAccessor = accessorMap[primitive.mIndicesAccessor];
	std::string fileName = getFileNameBufferFile(indicesAccessor.mUriDerived, data);
	char* buffer = getBufferChunk(fileName, indicesAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mIndicesMap.clear();
	if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = readUnsignedByteFromBuffer(buffer, i);
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = readUnsignedShortFromBuffer(buffer, i);
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_INT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = readUnsignedIntFromBuffer(buffer, i);
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readTexCoords0FromFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	// Open the buffer file and read positions
	gLTFAccessor  mTexcoord_0Accessor = accessorMap[primitive.mTexcoord_0AccessorDerived];
	std::string fileName = getFileNameBufferFile(mTexcoord_0Accessor.mUriDerived, data);
	char* buffer = getBufferChunk(fileName, mTexcoord_0Accessor, startBinaryBuffer);

	// Iterate through the chunk
	mTexcoords_0Map.clear();
	for (int i = 0; i < mTexcoord_0Accessor.mCount; i++)
	{
		// A position must be a VEC3/Float, otherwise it doesn't get read
		if (mTexcoord_0Accessor.mType == "VEC2" && mTexcoord_0Accessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec2Struct pos = readVec2FromBuffer (buffer, i);
			mTexcoords_0Map[i] = pos;
		}
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
char* gLTFImportOgreMeshCreator::getBufferChunk (const std::string& fileName, gLTFAccessor accessor, int startBinaryBuffer)
{
	// Read the binary data
	std::ifstream ifs(fileName, std::ios::binary);

	// Set the stream to the beginning of the binary chunk
	ifs.seekg(startBinaryBuffer + accessor.mByteOffset + accessor.mByteOffsetBufferViewDerived, std::ios::beg);

	// Read the buffer
	char* buffer = new char[accessor.mByteLengthDerived]; // Don not forget to delete[] the buffer
	ifs.read(buffer, accessor.mByteLengthDerived);
	ifs.close();
	return buffer;
}

//---------------------------------------------------------------------
const std::string& gLTFImportOgreMeshCreator::getFileNameBufferFile (const std::string& uri, 
	Ogre::HlmsEditorPluginData* data)
{
	fileNameBufferHelper = "";
	if (isFilePathAbsolute(uri))
		fileNameBufferHelper = uri;
	else
		fileNameBufferHelper = data->mInFileDialogPath + uri;
	
	return fileNameBufferHelper;
}

//---------------------------------------------------------------------
unsigned char gLTFImportOgreMeshCreator::readUnsignedByteFromBuffer (char* buffer, int count)
{
	unsigned char scalar;
	int unsignedCharSize = sizeof(unsigned char);
	memcpy(&scalar, &buffer[count * unsignedCharSize], unsignedCharSize);
	return scalar;
}

//---------------------------------------------------------------------
unsigned short gLTFImportOgreMeshCreator::readUnsignedShortFromBuffer (char* buffer, int count)
{
	unsigned short scalar;
	int unsignedShortSize = sizeof(unsigned short);
	memcpy(&scalar, &buffer[count * unsignedShortSize], unsignedShortSize);
	return scalar;
}

//---------------------------------------------------------------------
unsigned int gLTFImportOgreMeshCreator::readUnsignedIntFromBuffer (char* buffer, int count)
{
	unsigned int scalar;
	int unsignedIntSize = sizeof(unsigned int);
	memcpy(&scalar, &buffer[count * unsignedIntSize], unsignedIntSize);
	return scalar;
}

//---------------------------------------------------------------------
const gLTFImportOgreMeshCreator::Vec2Struct& gLTFImportOgreMeshCreator::readVec2FromBuffer (char* buffer, int count)
{
	float fRaw;
	int floatSize = sizeof(float);
	int vec2Size = 2 * floatSize;
	memcpy(&fRaw, &buffer[count * vec2Size], floatSize);
	mHelperVec2Struct.u = fRaw;
	memcpy(&fRaw, &buffer[count * vec2Size + floatSize], floatSize);
	mHelperVec2Struct.v = fRaw;
	//OUT << mHelperVec2Struct.u << ", " << mHelperVec2Struct.v << "\n";
	return mHelperVec2Struct;
}

//---------------------------------------------------------------------
const gLTFImportOgreMeshCreator::Vec3Struct& gLTFImportOgreMeshCreator::readVec3FromBuffer (char* buffer, int count)
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
	//OUT << count * vec3Size + 2 * floatSize << "\n";
	return mHelperVec3Struct;
}

