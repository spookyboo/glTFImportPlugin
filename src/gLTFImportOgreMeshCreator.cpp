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
#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreMatrix4.h"
#include "base64.h"
#include "OgreRoot.h"

//---------------------------------------------------------------------
gLTFImportOgreMeshCreator::gLTFImportOgreMeshCreator (void)
{
	mHelperString = "";
	fileNameBufferHelper = "";
	mPositionsMap.clear();
	mNormalsMap.clear();
	mTangentsMap.clear();
	mColor_0AccessorMap.clear();
	mTexcoords_0Map.clear();
	mTexcoords_1Map.clear();
	mIndicesMap.clear();
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createOgreMeshFiles (Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFNode> nodesMap,
	std::map<int, gLTFMesh> meshesMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer,
	bool hasAnimations)
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
		{
			createCombinedOgreSkeletonFile(data, nodesMap, accessorMap, startBinaryBuffer, hasAnimations);
			return createCombinedOgreMeshFile(data, nodesMap, accessorMap, startBinaryBuffer, hasAnimations);
		}
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
	std::string ogreFullyQualifiedMeshMeshFileName;
	for (it = meshesMap.begin(); it != meshesMap.end(); it++)
	{
		mesh = it->second;
		ogreFullyQualifiedMeshXmlFileName = fullyQualifiedImportPath + mesh.mName + ".xml";
		ogreFullyQualifiedMeshMeshFileName = fullyQualifiedImportPath + mesh.mName + ".mesh";
		OUT << "Create: mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

		// Create the file
		std::ofstream dst(ogreFullyQualifiedMeshXmlFileName);

		// Add xml content
		dst << "<mesh>\n";
		dst << TAB << "<submeshes>\n";

		writeSubmeshToMesh(dst, mesh, data, accessorMap, startBinaryBuffer); // Do not perform any transformation
		
		dst << TAB << "</submeshes>\n";
		dst << "</mesh>\n";

		dst.close();
		OUT << "Written mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";
	}

	convertXmlFileToMesh(data, ogreFullyQualifiedMeshXmlFileName, ogreFullyQualifiedMeshMeshFileName);
	setMeshFileNamePropertyValue(data, ogreFullyQualifiedMeshMeshFileName);
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createCombinedOgreMeshFile (Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFNode> nodesMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer,
	bool hasAnimations)
{
	// ********************************************* MESH FILE *********************************************
	OUT << "\nPerform gLTFImportOgreMeshCreator::createCombinedOgreMeshFile\n";
	
	// Create the Ogre mesh xml files (*.xml)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Create one combined Ogre mesh file (.xml)
	std::map<int, gLTFNode>::iterator it;
	gLTFNode node;
	gLTFMesh mesh;

	std::string ogreFullyQualifiedMeshXmlFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".xml";
	std::string ogreFullyQualifiedMeshMeshFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".mesh";
	OUT << "Create: mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

	// Create the file
	std::ofstream dst(ogreFullyQualifiedMeshXmlFileName);

	// Add xml content
	dst << "<mesh>\n";
	dst << TAB << "<submeshes>\n";

	// Iterate through all nodes and write the geometry data (vertices) of the related meshes
	for (it = nodesMap.begin(); it != nodesMap.end(); it++)
	{
		node = it->second;
		if (node.mMesh > -1)
		{
			mesh = node.mMeshDerived;
			Ogre::Matrix4 matrix = node.mCalculatedTransformation;
			writeSubmeshToMesh(dst,
				mesh, 
				data, 
				accessorMap, 
				startBinaryBuffer, 
				matrix,
				hasAnimations);
		}
	}

	dst << TAB << "</submeshes>\n";
	if (hasAnimations)
	{
		dst << TAB <<
			"<skeletonlink name = \"" <<
			data->mInFileDialogBaseName <<
			".skeleton\" />\n";
		//std::string ogreFullyQualifiedSkeletonFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".skeleton";
		//dst << TAB <<
			//"<skeletonlink name = \"" <<
			//ogreFullyQualifiedSkeletonFileName <<
			//"\" />\n";
	}

	dst << "</mesh>\n";
	dst.close();
	OUT << "Written mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";
	convertXmlFileToMesh(data, ogreFullyQualifiedMeshXmlFileName, ogreFullyQualifiedMeshMeshFileName);
	setMeshFileNamePropertyValue(data, ogreFullyQualifiedMeshMeshFileName);

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createCombinedOgreSkeletonFile (Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFNode> nodesMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer,
	bool hasAnimations)
{
	// ********************************************* SKELETON FILE *********************************************
	OUT << "\nPerform gLTFImportOgreMeshCreator::createCombinedOgreSkeletonFile\n";

	if (!hasAnimations)
		return false;
	
	// Create the Ogre skeleton files (*.skeleton.xml/.skeleton)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Create one combined Ogre mesh file (.xml)
	std::map<int, gLTFNode>::iterator it;
	gLTFNode node;

	std::string ogreFullyQualifiedSkeletonXmlFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".skeleton.xml";
	std::string ogreFullyQualifiedSkeletonFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".skeleton";
	OUT << "Create: skeleton.xml file " << ogreFullyQualifiedSkeletonXmlFileName << "\n";

	// Create the file
	std::ofstream dst(ogreFullyQualifiedSkeletonXmlFileName);

	// Add xml content
	dst << "<skeleton blendmode=\"average\">\n";
	
	// 1. Add bones (=nodes, =joints)
	dst << TAB << "<bones>\n";
	
	/* Iterate through all nodes and write the bone info. Each joint is a bone. Each joint refers to a node.
	 */
	unsigned int index = 0;
	for (it = nodesMap.begin(); it != nodesMap.end(); it++)
	{
		node = it->second;
		writeBoneToSkeleton(dst,
			index,
			&node,
			data,
			accessorMap,
			startBinaryBuffer);
		++index;
	}
	dst << TAB << "</bones>\n";

	// 2. Add bones hierarchy
	dst << TAB << "<bonehierarchy>\n";
	for (it = nodesMap.begin(); it != nodesMap.end(); it++)
	{
		node = it->second;
		writeBoneHierarchyToSkeleton(dst, &node);
	}
	dst << TAB << "</bonehierarchy>\n";

	// 3. Add animations
	dst << TAB << "<animations>\n";
	// TODO: Add animations
	dst << TAB << "</animations>\n";

	dst << "</skeleton>\n";
	dst.close();
	OUT << "Written skeleton.xml file " << ogreFullyQualifiedSkeletonXmlFileName << "\n";
	convertXmlFileToSkeleton(data, ogreFullyQualifiedSkeletonXmlFileName, ogreFullyQualifiedSkeletonFileName);
	setSkeletonFileNamePropertyValue(data, ogreFullyQualifiedSkeletonFileName);

#ifdef USE_OGRE_IN_PLUGIN
	// Load the skeletonfile in Ogre before the mesh .xml is converted
	//Ogre::Root* root = Ogre::Root::getSingletonPtr();
	//root->addResourceLocation(fullyQualifiedImportPath, "FileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
#endif // DEBUG

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeSubmeshToMesh(std::ofstream& dst,
	gLTFMesh mesh,
	Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer,
	Ogre::Matrix4 matrix,
	bool hasAnimations)
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
		dst << TABx2 << "<submesh material = \"" + materialName + "\"";
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
			writeFacesToMesh(dst, primitive, accessorMap, data, startBinaryBuffer);
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
			writeVerticesToMesh(dst,
				primitive, 
				accessorMap, 
				data, 
				startBinaryBuffer, 
				matrix);

			// Closing tags
			dst << TABx4 << "</vertexbuffer>\n";
			dst << TABx3 << "</geometry>\n";

			/* Add none assignment in case there are animations
			 * Only in case there is an animation, these entries are written, otherwise it does not
			 * make sense.
			 */
			if (hasAnimations)
			{
				dst << TABx3 << "<boneassignments>\n";

				// Write bone assignments
				writeBoneAssignmentsToMesh(dst, primitive, accessorMap, data, startBinaryBuffer);

				dst << TABx3 << "</boneassignments>\n";
			}
		}

		dst << TABx2 << "</submesh>\n";
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeFacesToMesh(std::ofstream& dst,
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

		// For anticlockwise, swap around indices; convert ccw to cw for front face
		/*
		dst << TABx4 << "<face v1 = \"" << mIndicesMap[i] <<
			"\" v2 = \"" << mIndicesMap[i + 2] <<
			"\" v3 = \"" << mIndicesMap[i + 1] << "\" />\n";
		*/
	}

	// Closing tag
	dst << TABx3 << "</faces>\n";

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeVerticesToMesh(std::ofstream& dst,
	const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer,
	Ogre::Matrix4 matrix)
{
	// Read positions, normals, tangents,... etc.
	readPositionsFromUriOrFile(primitive, 
		accessorMap, 
		data, 
		startBinaryBuffer, 
		matrix); // Read the positions
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
		Ogre::Vector3 vec3 = mPositionsMap[i];
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
			Ogre::Vector4 vec4 = mTangentsMap[i];
			// Take value 'a' (= w) into account for handedness
			dst << TABx6 << "<tangent x=\"" << vec4.x << 
				"\" y=\"" << vec4.y << 
				"\" z=\"" << vec4.z << 
				"\" w=\"" << vec4.w <<
				"\" />\n";
		}

		// Diffuse color
		if (mColor_0AccessorMap.size() > 0)
		{
			Ogre::Vector4 vec4 = mColor_0AccessorMap[i];
			dst << TABx6 << "<colour_diffuse value=\"" << vec4.x << " " <<
				vec4.y << " " <<
				vec4.z << " " <<
				vec4.w << "\" />\n";
		}

		// Texcoord 0
		if (mTexcoords_0Map.size() > 0)
		{
			Ogre::Vector2 vec2 = mTexcoords_0Map[i];
			dst << TABx6 << "<texcoord u=\"" << vec2.x << "\" v=\"" << vec2.y << "\" />\n";
		}

		// Texcoord 1
		if (mTexcoords_1Map.size() > 0)
		{
			Ogre::Vector2 vec2 = mTexcoords_1Map[i];
			dst << TABx6 << "<texcoord u=\"" << vec2.x << "\" v=\"" << vec2.y << "\" />\n";
		}

		// Close vertex
		dst << TABx5 << "</vertex>\n";
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeBoneAssignmentsToMesh(std::ofstream& dst,
	const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	gLTFAccessor  positionAccessor = accessorMap[primitive.mPositionAccessorDerived]; // To determine number of vertices
	gLTFAccessor  jointAccessor = accessorMap[primitive.mJoints_0AccessorDerived];
	gLTFAccessor  weightAccessor = accessorMap[primitive.mWeights_0AccessorDerived];
	Ogre::Vector4 joint;
	Ogre::Vector4 weight;
	unsigned short fCount;

	// Get the buffer
	char* jointBuffer = getBufferChunk(weightAccessor.mUriDerived, data, jointAccessor, startBinaryBuffer);
	char* weightBuffer = getBufferChunk(weightAccessor.mUriDerived, data, weightAccessor, startBinaryBuffer);

	// Iterate through all vertices
	for (int i = 0; i < positionAccessor.mCount; i++)
	{
		joint = Ogre::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		weight = Ogre::Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		// Get the joint (= bone). A joint must be a VEC4/UNSIGNED_BYTE/UNSIGNED_SHORT, otherwise it doesn't get read
		if (jointAccessor.mType == "VEC4" && jointAccessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
		{
			// Get the joint
			joint = mBufferReader.readVec4FromUnsignedByteBuffer(jointBuffer,
				i,
				jointAccessor,
				getCorrectForMinMaxPropertyValue(data));
		}
		else if (jointAccessor.mType == "VEC4" && jointAccessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
		{
			// Get the joint
			joint = mBufferReader.readVec4FromUnsignedShortBuffer(jointBuffer,
				i,
				jointAccessor,
				getCorrectForMinMaxPropertyValue(data));
		}

		// Get the weight. A weight must be a VEC4/FLOAT/UNSIGNED_BYTE/UNSIGNED_SHORT, otherwise it doesn't get read
		if (weightAccessor.mType == "VEC4" && weightAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			// Get the weight
			weight = mBufferReader.readVec4FromFloatBuffer(weightBuffer,
				i,
				weightAccessor,
				getCorrectForMinMaxPropertyValue(data));
		}
		else if (weightAccessor.mType == "VEC4" && weightAccessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
		{
			// Get the weight
			weight = mBufferReader.readVec4FromUnsignedByteBuffer(weightBuffer,
				i,
				weightAccessor,
				getCorrectForMinMaxPropertyValue(data));
		}
		else if (weightAccessor.mType == "VEC4" && weightAccessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
		{
			// Get the weight
			weight = mBufferReader.readVec4FromUnsignedShortBuffer(weightBuffer,
				i,
				weightAccessor,
				getCorrectForMinMaxPropertyValue(data));
		}

		// Write the xml entry. A vertex can be influenced by max. 4 bones/joints
		float joints[4];
		float weights[4];
		joints[0] = joint.x;
		joints[1] = joint.y;
		joints[2] = joint.z;
		joints[3] = joint.w;
		weights[0] = weight.x;
		weights[1] = weight.y;
		weights[2] = weight.z;
		weights[3] = weight.w;
		fCount = 0;
		while (fCount < 4)
		{
			weights[fCount] = weights[fCount] < 0.00000001f ? 1.0f : weights[fCount];
			dst << TABx4 <<
				"<vertexboneassignment vertexindex=\"" <<
				i <<
				"\" boneindex = \"" <<
				joints[fCount] <<
				"\" weight=\"" <<
				weights[fCount] <<
				"\" />\n";
			++fCount;
		}
	}

	delete[] weightBuffer;
	delete[] jointBuffer;
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeBoneToSkeleton(std::ofstream& dst,
	unsigned int index,
	gLTFNode* node,
	Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer)
{
	if (node->mName != "")
	{
		dst << TABx2 << "<bone id=\"" <<
			index <<
			"\" name=\"" <<
			node->mName <<
			"\">\n";
		dst << TABx2 << "</bone>\n";
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeBoneHierarchyToSkeleton(std::ofstream& dst, gLTFNode* node)
{
	std::string parentNodeName = "";
	if (node->mParentNode)
		parentNodeName = node->mParentNode->mName;

	if (parentNodeName != "" && node->mName != "")
	{
		dst << TABx2 << "<boneparent bone=\"" <<
			node->mName <<
			"\" parent=\"" <<
			parentNodeName <<
			"\" />\n";
	}

	return true;
}

//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readPositionsFromUriOrFile (const gLTFPrimitive& primitive,
	std::map<int, gLTFAccessor> accessorMap,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer,
	Ogre::Matrix4 matrix)
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
			// Perform the transformation; use Ogre's classes, because they are proven
			Ogre::Vector3 pos = mBufferReader.readVec3FromFloatBuffer(buffer,
				i, 
				positionAccessor, 
				getCorrectForMinMaxPropertyValue(data));
			pos = matrix * pos;
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
			Ogre::Vector3 pos = mBufferReader.readVec3FromFloatBuffer(buffer,
				i, 
				normalAccessor, 
				getCorrectForMinMaxPropertyValue(data));
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
			Ogre::Vector4 pos = mBufferReader.readVec4FromFloatBuffer(buffer,
				i, 
				tangentAccessor, 
				getCorrectForMinMaxPropertyValue(data));
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
			Ogre::Vector3 v3 = mBufferReader.readVec3FromFloatBuffer(buffer,
				i, 
				mColor_0Accessor, 
				getCorrectForMinMaxPropertyValue(data));
			Ogre::Vector4 col;
			col.x = v3.x;
			col.y = v3.y;
			col.z = v3.z;
			col.w = 1.0f;
			mColor_0AccessorMap[i] = col;
		}
		else if (mColor_0Accessor.mType == "VEC4")
		{
			Ogre::Vector4 col = mBufferReader.readVec4FromFloatBuffer(buffer,
				i, 
				mColor_0Accessor, 
				getCorrectForMinMaxPropertyValue(data));
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
			mIndicesMap[i] = mBufferReader.readFromUnsignedByteBuffer(buffer, 
				i, 
				indicesAccessor,
				getCorrectForMinMaxPropertyValue(data));
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = mBufferReader.readFromUnsignedShortBuffer(buffer, 
				i, 
				indicesAccessor,
				getCorrectForMinMaxPropertyValue(data));
	}
	else if (indicesAccessor.mType == "SCALAR" && indicesAccessor.mComponentType == gLTFAccessor::UNSIGNED_INT)
	{
		for (int i = 0; i < indicesAccessor.mCount; i++)
			mIndicesMap[i] = mBufferReader.readFromUnsignedIntBuffer(buffer, 
				i, 
				indicesAccessor,
				getCorrectForMinMaxPropertyValue(data));
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
			Ogre::Vector2 pos = mBufferReader.readVec2FromFloatBuffer(buffer,
				i, 
				mTexcoord_0Accessor, 
				getCorrectForMinMaxPropertyValue(data));
			mTexcoords_0Map[i] = pos;
		}
		else if (mTexcoord_0Accessor.mType == "VEC2" && mTexcoord_0Accessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
		{
			Ogre::Vector2 pos = mBufferReader.readVec2FromUnsignedByteBuffer(buffer,
				i, 
				mTexcoord_0Accessor, 
				getCorrectForMinMaxPropertyValue(data));
			mTexcoords_0Map[i] = pos;
		}
		else if (mTexcoord_0Accessor.mType == "VEC2" && mTexcoord_0Accessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
		{
			Ogre::Vector2 pos = mBufferReader.readVec2FromUnsignedShortBuffer(buffer,
				i, 
				mTexcoord_0Accessor, 
				getCorrectForMinMaxPropertyValue(data));
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
			Ogre::Vector2 pos = mBufferReader.readVec2FromFloatBuffer(buffer,
				i, 
				mTexcoord_1Accessor, 
				getCorrectForMinMaxPropertyValue(data));
			mTexcoords_1Map[i] = pos;
		}
		else if (mTexcoord_1Accessor.mType == "VEC2" && mTexcoord_1Accessor.mComponentType == gLTFAccessor::UNSIGNED_BYTE)
		{
			Ogre::Vector2 pos = mBufferReader.readVec2FromUnsignedByteBuffer(buffer,
				i, 
				mTexcoord_1Accessor, 
				getCorrectForMinMaxPropertyValue(data));
			mTexcoords_1Map[i] = pos;
		}
		else if (mTexcoord_1Accessor.mType == "VEC2" && mTexcoord_1Accessor.mComponentType == gLTFAccessor::UNSIGNED_SHORT)
		{
			Ogre::Vector2 pos = mBufferReader.readVec2FromUnsignedShortBuffer(buffer,
				i, 
				mTexcoord_1Accessor, 
				getCorrectForMinMaxPropertyValue(data));
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
bool gLTFImportOgreMeshCreator::convertXmlFileToMesh (Ogre::HlmsEditorPluginData* data, 
	const std::string& xmlFileName, 
	const std::string& meshFileName)
{
	std::string meshToolCmd= "OgreMeshTool -v2 ";
	std::string meshToolGenerateTangents = "";
	std::string meshToolGenerateEdgeLists = "-e ";
	std::string meshToolOptimize = "-O qs ";
	
	// First get the property values (from the HLMS Editor)
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY> properties = data->mInPropertiesMap;
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY>::iterator it = properties.find("generate_edge_lists");
	if (it != properties.end() && (it->second).boolValue)
		meshToolGenerateEdgeLists = "";

	it = properties.find("generate_tangents");
	if (it != properties.end() && (it->second).boolValue)
		meshToolGenerateTangents = "-t -ts 4 ";

	it = properties.find("optimize_for_desktop");
	if (it != properties.end() && (it->second).boolValue)
		meshToolOptimize = "-O puqs ";

	meshToolCmd += meshToolGenerateEdgeLists + meshToolGenerateTangents + meshToolOptimize;

	// Desktop with normals
	std::string runOgreMeshTool = meshToolCmd + "\"" + xmlFileName + "\" \"" + meshFileName + "\"";
	OUT << "Generating mesh: " << runOgreMeshTool << "\n";
	system(runOgreMeshTool.c_str());
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::convertXmlFileToSkeleton (Ogre::HlmsEditorPluginData* data,
	const std::string& xmlFileName,
	const std::string& skeletonFileName)
{
	std::string meshToolCmd = "OgreMeshTool -v2 ";

	std::string runOgreMeshTool = meshToolCmd + "\"" + xmlFileName + "\" \"" + skeletonFileName + "\"";
	OUT << "Generating skeleton: " << runOgreMeshTool << "\n";
	system(runOgreMeshTool.c_str());
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::getCorrectForMinMaxPropertyValue (Ogre::HlmsEditorPluginData* data)
{
	// First get the property value (from the HLMS Editor)
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY> properties = data->mInPropertiesMap;
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY>::iterator it = properties.find("correct_min_max");
	if (it != properties.end())
	{
		// Property found; determine its value
		return (it->second).boolValue;
	}

	return false;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::setMeshFileNamePropertyValue (Ogre::HlmsEditorPluginData* data, const std::string& fileName)
{
	Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY property;
	property.propertyName = "load_mesh";
	property.type = Ogre::HlmsEditorPluginData::STRING;
	property.stringValue = fileName;
	data->mOutReferencesMap[property.propertyName] = property;
	
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::setSkeletonFileNamePropertyValue(Ogre::HlmsEditorPluginData* data, const std::string& fileName)
{
	Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY property;
	property.propertyName = "load_skeleton";
	property.type = Ogre::HlmsEditorPluginData::STRING;
	property.stringValue = fileName;
	data->mOutReferencesMap[property.propertyName] = property;

	return true;
}
