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

	// First get the property value (from the HLMS Editor)
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY> properties = data->mInPropertiesMap;
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY>::iterator it = properties.find("mesh_is_submesh");
	if (it != properties.end())
	{
		// Property found; determine its value
		if ((it->second).boolValue)
			return createCombinedOgreMeshFile (data, meshesMap, accessorMap, startBinaryBuffer);
		else
			return createIndividualOgreMeshFiles (data, meshesMap, accessorMap, startBinaryBuffer);
	}
	else
	{
		// Property not found; the default way of processing is to create individual meshes
		return createIndividualOgreMeshFiles(data, meshesMap, accessorMap, startBinaryBuffer);
	}
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createIndividualOgreMeshFiles (Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFMesh> meshesMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer)
{
	OUT << "\nPerform gLTFImportOgreMeshCreator::createIndividualOgreMeshFiles\n";

	// Create the Ogre mesh xml files (*.xml)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Iterate through meshes and create for each mesh an Ogre .xml file
	std::map<int, gLTFMesh>::iterator it;
	gLTFMesh mesh;
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

		writeSubmesh(dst, mesh, data, meshesMap, accessorMap, startBinaryBuffer);
		
		dst << TAB << "</submeshes>\n";
		dst << "</mesh>\n";

		dst.close();
	}

	OUT << "Written mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createCombinedOgreMeshFile (Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFMesh> meshesMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer)
{
	OUT << "\nPerform gLTFImportOgreMeshCreator::createCombinedOgreMeshFile\n";

	// Create the Ogre mesh xml files (*.xml)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Create one combined Ogre mesh file (.xml)
	std::map<int, gLTFMesh>::iterator it;
	gLTFMesh mesh;
	std::string ogreFullyQualifiedMeshXmlFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".xml";
	OUT << "Create: mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

	// Create the file
	std::ofstream dst(ogreFullyQualifiedMeshXmlFileName);

	// Add xml content
	dst << "<mesh>\n";
	dst << TAB << "<submeshes>\n";

	// Iterate through meshes and add the geometry data
	for (it = meshesMap.begin(); it != meshesMap.end(); it++)
	{
		mesh = it->second;

		// Iterate through primitives (each gLTF mesh is a submesh)
		// TODO: Apply transformation to the submeshes, because they are now have the same origin/rotation !!!!!!!!!
		writeSubmesh(dst, mesh, data, meshesMap, accessorMap, startBinaryBuffer);
	}
	
	dst << TAB << "</submeshes>\n";
	dst << "</mesh>\n";
	dst.close();
	OUT << "Written mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeSubmesh (std::ofstream& dst, 
	gLTFMesh mesh,
	Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFMesh> meshesMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer)
{
	std::map<int, gLTFPrimitive>::iterator itPrimitives;
	gLTFPrimitive primitive;
	std::string materialName;

	// Iterate through primitives (each primitive is a submesh)
	for (itPrimitives = mesh.mPrimitiveMap.begin(); itPrimitives != mesh.mPrimitiveMap.end(); itPrimitives++)
	{
		primitive = itPrimitives->second;
		materialName = primitive.mMaterialNameDerived;
		if (materialName == "")
			materialName = "BaseWhite";

		/* Write submesh definition
		 * Do not share vertices. It is not clear whether gLTF shares geometry data, so use the default 'false' value,
		 * until proven otherwise.
		 * We always use 32 bit incices (no 16 bit), even if the gLTF file itself uses 16 bits
		 */
		dst << TABx2 << "<submesh material = \"" + primitive.mMaterialNameDerived + "\"";
		dst << " usesharedvertices = \"false\" use32bitindexes = \"false\" "; // TODO: Hardcoded?

		// Write operation type
		switch (primitive.mMode)
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
			writeFaces(dst, primitive, accessorMap, data, startBinaryBuffer);
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
			if (primitive.mTexcoord_0AccessorDerived > -1 && primitive.mTexcoord_1AccessorDerived > -1)
				numTextCoordsText = "\"2\"";

			dst << TABx4 << "<vertexbuffer positions = " << hasPositionsText <<
				" normals = " << hasNormalsText;

			// Colours
			if (primitive.mColor_0AccessorDerived > -1)
				dst << " colours_diffuse = \"true\"";

			// Texture coordinate dimensions (assume float2 for now)
			if (primitive.mTexcoord_0AccessorDerived > -1)
				dst << " texture_coord_dimensions_0 = \"float2\"";
			if (primitive.mTexcoord_0AccessorDerived > -1 && primitive.mTexcoord_1AccessorDerived > -1)
				dst << " texture_coord_dimensions_1 = \"float2\"";

			// Tangents
			dst << " tangents = " << hasTangentsText;
			if (primitive.mTangentAccessorDerived > -1)
				" tangent_dimensions = \"4\"";

			// Texcoords
			dst << " texture_coords = " << numTextCoordsText <<
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
			Vec4Struct vec4 = mTangentsMap[i];
			// Take value 'a' (= w) into account for handedness
			dst << TABx6 << "<tangent x=\"" << vec4.r << 
				"\" y=\"" << vec4.r << 
				"\" z=\"" << vec4.b << 
				"\" w=\"" << vec4.a <<
				"\" />\n";
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
			Vec3Struct pos = mBufferReader.readVec3FromFloatBuffer(buffer, i, positionAccessor);
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
	if (primitive.mNormalAccessorDerived < 0)
		return;

	// Get the buffer and read positions
	gLTFAccessor  normalAccessor = accessorMap[primitive.mNormalAccessorDerived];
	char* buffer = getBufferChunk(normalAccessor.mUriDerived, data, normalAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mNormalsMap.clear();
	for (int i = 0; i < normalAccessor.mCount; i++)
	{
		// A normal  must be a VEC3/Float, otherwise it doesn't get read
		if (normalAccessor.mType == "VEC3" && normalAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec3Struct pos = mBufferReader.readVec3FromFloatBuffer(buffer, i, normalAccessor);
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
	if (primitive.mTangentAccessorDerived < 0)
		return;

	// Get the buffer file and read positions
	gLTFAccessor  tangentAccessor = accessorMap[primitive.mTangentAccessorDerived];
	char* buffer = getBufferChunk(tangentAccessor.mUriDerived, data, tangentAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mTangentsMap.clear();
	for (int i = 0; i < tangentAccessor.mCount; i++)
	{
		// A tangent must be a VEC4/Float, otherwise it doesn't get read
		if (tangentAccessor.mType == "VEC4" && tangentAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec4Struct pos = mBufferReader.readVec4FromFloatBuffer(buffer, i, tangentAccessor);
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
	if (primitive.mColor_0AccessorDerived < 0)
		return;

	// Get the buffer file and read colours
	gLTFAccessor  mColor_0Accessor = accessorMap[primitive.mColor_0AccessorDerived];
	char* buffer = getBufferChunk(mColor_0Accessor.mUriDerived, data, mColor_0Accessor, startBinaryBuffer);

	// Iterate through the chunk
	mColor_0AccessorMap.clear();
	for (int i = 0; i < mColor_0Accessor.mCount; i++)
	{
		// A colour can be a VEC3 (Float)
		if (mColor_0Accessor.mType == "VEC3")
		{
			Vec3Struct v3 = mBufferReader.readVec3FromFloatBuffer(buffer, i, mColor_0Accessor);
			Vec4Struct col;
			col.r = v3.x;
			col.g = v3.y;
			col.b = v3.z;
			col.a = 1.0f;
			mColor_0AccessorMap[i] = col;
		}
		else if (mColor_0Accessor.mType == "VEC4")
		{
			Vec4Struct col = mBufferReader.readVec4FromFloatBuffer(buffer, i, mColor_0Accessor);
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
	if (primitive.mIndicesAccessor < 0)
		return;

	// Get the buffer file and read indices
	gLTFAccessor indicesAccessor = accessorMap[primitive.mIndicesAccessor];
	char* buffer = getBufferChunk(indicesAccessor.mUriDerived, data, indicesAccessor, startBinaryBuffer);

	// Iterate through the chunk
	mIndicesMap.clear();
	if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = mBufferReader.readFromUnsignedByteBuffer(buffer, i, indicesAccessor);
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = mBufferReader.readFromUnsignedShortBuffer(buffer, i, indicesAccessor);
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_INT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = mBufferReader.readFromUnsignedIntBuffer(buffer, i, indicesAccessor);
	}

	delete[] buffer;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readTexCoords0FromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	if (primitive.mTexcoord_0AccessorDerived < 0)
		return;

	// Get the buffer file and read positions
	gLTFAccessor  mTexcoord_0Accessor = accessorMap[primitive.mTexcoord_0AccessorDerived];
	char* buffer = getBufferChunk(mTexcoord_0Accessor.mUriDerived, data, mTexcoord_0Accessor, startBinaryBuffer);

	// Iterate through the chunk
	mTexcoords_0Map.clear();
	for (int i = 0; i < mTexcoord_0Accessor.mCount; i++)
	{
		// A position must be a VEC3/Float, otherwise it doesn't get read
		if (mTexcoord_0Accessor.mType == "VEC2" && mTexcoord_0Accessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec2Struct pos = mBufferReader.readVec2FromFloatBuffer(buffer, i, mTexcoord_0Accessor);
			mTexcoords_0Map[i] = pos;
		}
		else if (mTexcoord_0Accessor.mType == "VEC2" && mTexcoord_0Accessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
		{
			Vec2Struct pos = mBufferReader.readVec2FromUnsignedByteBuffer(buffer, i, mTexcoord_0Accessor);
			mTexcoords_0Map[i] = pos;
		}
		else if (mTexcoord_0Accessor.mType == "VEC2" && mTexcoord_0Accessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
		{
			Vec2Struct pos = mBufferReader.readVec2FromUnsignedShortBuffer(buffer, i, mTexcoord_0Accessor);
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
	if (primitive.mTexcoord_1AccessorDerived < 0)
		return;

	// Get the buffer file and read positions
	gLTFAccessor  mTexcoord_1Accessor = accessorMap[primitive.mTexcoord_1AccessorDerived];
	char* buffer = getBufferChunk(mTexcoord_1Accessor.mUriDerived, data, mTexcoord_1Accessor, startBinaryBuffer);

	// Iterate through the chunk
	mTexcoords_1Map.clear();
	for (int i = 0; i < mTexcoord_1Accessor.mCount; i++)
	{
		// A position must be a VEC3, otherwise it doesn't get read
		if (mTexcoord_1Accessor.mType == "VEC2" && mTexcoord_1Accessor.mComponentType == gLTFAccessor::FLOAT)
		{
			Vec2Struct pos = mBufferReader.readVec2FromFloatBuffer(buffer, i, mTexcoord_1Accessor);
			mTexcoords_1Map[i] = pos;
		}
		else if (mTexcoord_1Accessor.mType == "VEC2" && mTexcoord_1Accessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
		{
			Vec2Struct pos = mBufferReader.readVec2FromUnsignedByteBuffer(buffer, i, mTexcoord_1Accessor);
			mTexcoords_1Map[i] = pos;
		}
		else if (mTexcoord_1Accessor.mType == "VEC2" && mTexcoord_1Accessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
		{
			Vec2Struct pos = mBufferReader.readVec2FromUnsignedShortBuffer(buffer, i, mTexcoord_1Accessor);
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
