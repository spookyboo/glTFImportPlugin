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
	mNodesMap.clear();
	mMeshesMap.clear();
	mAnimationsMap.clear();
	mAccessorMap.clear();
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createOgreMeshFiles (Ogre::HlmsEditorPluginData* data,
	std::map<int, gLTFNode> nodesMap,
	std::map<int, gLTFMesh> meshesMap,
	std::map<int, gLTFAnimation> animationsMap,
	std::map<int, gLTFSkin> skinsMap,
	std::map<int, gLTFAccessor> accessorMap,
	int startBinaryBuffer,
	bool hasAnimations)
{
	OUT << "\nPerform gLTFImportOgreMeshCreator::createOgreMeshFiles\n";
	OUT << "------------------------------------------------------\n";

	mNodesMap = nodesMap;
	mMeshesMap = meshesMap;
	mAnimationsMap = animationsMap;
	mAccessorMap = accessorMap;
	mSkinsMap = skinsMap;

	// First get the property value (from the HLMS Editor)
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY> properties = data->mInPropertiesMap;
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY>::iterator it = properties.find("mesh_is_submesh");
	if (it != properties.end())
	{
		// Property found; determine its value
		if ((it->second).boolValue)
		{
			if (isGenerateAnimationProperty(data))
				createCombinedOgreSkeletonFile(data, startBinaryBuffer, hasAnimations);
			return createCombinedOgreMeshFile(data, startBinaryBuffer, hasAnimations);
		}
		else
			return createIndividualOgreMeshFiles (data, startBinaryBuffer);
	}
	else
	{
		// Property not found; the default way of processing is to create individual meshes
		return createIndividualOgreMeshFiles(data, startBinaryBuffer);
	}
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createIndividualOgreMeshFiles (Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	OUT << "Perform gLTFImportOgreMeshCreator::createIndividualOgreMeshFiles\n";

	// Create the Ogre mesh xml files (*.xml)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Iterate through meshes and create for each mesh an Ogre .xml file
	std::map<int, gLTFMesh>::iterator it;
	gLTFMesh mesh;
	std::string ogreFullyQualifiedMeshXmlFileName;
	std::string ogreFullyQualifiedMeshMeshFileName;
	unsigned int meshIndex = 0;
	for (it = mMeshesMap.begin(); it != mMeshesMap.end(); it++)
	{
		mesh = it->second;
		ogreFullyQualifiedMeshXmlFileName = fullyQualifiedImportPath + mesh.mName + ".xml";
		ogreFullyQualifiedMeshMeshFileName = fullyQualifiedImportPath + mesh.mName + ".mesh";
		OUT << TABx2 << "Create: mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

		// Create the file
		std::ofstream dst(ogreFullyQualifiedMeshXmlFileName);

		// Add xml content
		dst << "<mesh>\n";
		dst << TAB << "<submeshes>\n";

		gLTFNode dummyNode;
		writeSubmeshToMesh(dst, dummyNode, mesh, data, startBinaryBuffer); // Do not perform any transformation
		
		dst << TAB << "</submeshes>\n";
		dst << "</mesh>\n";

		dst.close();
		OUT << TABx2 << "Written mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

		++meshIndex;
	}

	convertXmlFileToMesh(data, ogreFullyQualifiedMeshXmlFileName, ogreFullyQualifiedMeshMeshFileName);
	setMeshFileNamePropertyValue(data, ogreFullyQualifiedMeshMeshFileName);
	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createCombinedOgreMeshFile (Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer,
	bool hasAnimations)
{
	// ********************************************* MESH FILE *********************************************
	OUT << "Perform gLTFImportOgreMeshCreator::createCombinedOgreMeshFile\n";
	
	// Create the Ogre mesh xml files (*.xml)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Create one combined Ogre mesh file (.xml)
	std::map<int, gLTFNode>::iterator it;
	gLTFNode node;
	gLTFMesh mesh;

	std::string ogreFullyQualifiedMeshXmlFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".xml";
	std::string ogreFullyQualifiedMeshMeshFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".mesh";
	OUT << TABx2 << "Create: mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";

	// Create the file
	std::ofstream dst(ogreFullyQualifiedMeshXmlFileName);

	// Add xml content
	dst << "<mesh>\n";
	dst << TAB << "<submeshes>\n";

	// Iterate through all nodes and write the geometry data (vertices) of the related meshes
	for (it = mNodesMap.begin(); it != mNodesMap.end(); it++)
	{
		node = it->second;
		if (node.mMesh > -1)
		{
			mesh = node.mMeshDerived;
			Ogre::Matrix4 matrix = node.mCalculatedTransformation;
			writeSubmeshToMesh(dst,
				node,
				mesh,
				data, 
				startBinaryBuffer, 
				matrix,
				hasAnimations);
		}
	}

	dst << TAB << "</submeshes>\n";
	if (hasAnimations && isGenerateAnimationProperty(data))
	{
		dst << TAB <<
			"<skeletonlink name = \"" <<
			data->mInFileDialogBaseName <<
			".skeleton\" />\n";
	}

	dst << "</mesh>\n";
	dst.close();
	OUT << TABx2 << "Written mesh .xml file " << ogreFullyQualifiedMeshXmlFileName << "\n";
	convertXmlFileToMesh(data, ogreFullyQualifiedMeshXmlFileName, ogreFullyQualifiedMeshMeshFileName);
	setMeshFileNamePropertyValue(data, ogreFullyQualifiedMeshMeshFileName);

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::createCombinedOgreSkeletonFile (Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer,
	bool hasAnimations)
{
	// ********************************************* SKELETON FILE *********************************************
	OUT << TAB << "Perform gLTFImportOgreMeshCreator::createCombinedOgreSkeletonFile\n";

	if (!hasAnimations)
		return false;
	
	// Create the Ogre skeleton files (*.skeleton.xml/.skeleton)
	std::string fullyQualifiedImportPath = data->mInImportPath + data->mInFileDialogBaseName + "/";

	// Create one combined Ogre mesh file (.xml)
	std::map<int, gLTFNode>::iterator it;
	gLTFNode node;

	std::string ogreFullyQualifiedSkeletonXmlFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".skeleton.xml";
	std::string ogreFullyQualifiedSkeletonFileName = fullyQualifiedImportPath + data->mInFileDialogBaseName + ".skeleton";
	OUT << TABx2 << "Create: skeleton.xml file " << ogreFullyQualifiedSkeletonXmlFileName << "\n";

	// Create the file
	std::ofstream dst(ogreFullyQualifiedSkeletonXmlFileName);

	// Add xml content
	dst << "<skeleton blendmode=\"average\">\n";
	
	// 1. Add bones (=joints; a joint is an index to a node)
	dst << TAB << "<bones>\n";
	
	/* Write the bone info. Each joint is a bone. Each joint refers to a node.
	 */
	writeBonesToSkeleton(dst,
		data,
		startBinaryBuffer);
	
		dst << TAB << "</bones>\n";

	// 2. Add bones hierarchy
	dst << TAB << "<bonehierarchy>\n";
	writeBoneHierarchyToSkeleton(dst);
	dst << TAB << "</bonehierarchy>\n";

	// 3. Add animations
	writeAnimationsToSkeleton(dst, data, startBinaryBuffer);

	dst << "</skeleton>\n";
	dst.close();
	OUT << TABx2 << "Written skeleton.xml file " << ogreFullyQualifiedSkeletonXmlFileName << "\n";
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
bool gLTFImportOgreMeshCreator::writeSubmeshToMesh (std::ofstream& dst,
	gLTFNode node,
	gLTFMesh mesh,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer,
	Ogre::Matrix4 matrix,
	bool hasAnimations)
{
	std::map<int, gLTFPrimitive>::iterator itPrimitives;
	gLTFPrimitive primitive;
	std::string materialName;

	// Iterate through primitives (each primitive is a submesh)
	unsigned int primitiveIndex = 0;
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
			writeFacesToMesh(dst, primitive, data, startBinaryBuffer);
		}

		// Write geometry
		if (primitive.mPositionAccessorDerived > -1)
		{
			gLTFAccessor  accessor = mAccessorMap[primitive.mPositionAccessorDerived];
			dst << TABx3 << "<geometry vertexcount=\"" << accessor.mCount << "\">\n";

			// Write vertexbuffer header
			std::string hasPositionsText = "\"true\""; // Assume there are always positions, right?
			std::string hasNormalsText = "\"true\"";
			std::string hasTangentsText = "\"true\"";
			std::string numTextCoordsText = "\"0\"";
			if (primitive.mNormalAccessorDerived < 0)
				hasNormalsText = "\"false\"";
			if (primitive.mTangentAccessorDerived < 0 || isGenerateTangentsProperty(data))
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
			if (primitive.mTangentAccessorDerived > -1 && !isGenerateTangentsProperty(data))
				" tangent_dimensions = \"4\"";

			// Texcoords
			dst << " texture_coords = " << numTextCoordsText <<
				">\n";

			// Write vertices
			writeVerticesToMesh(dst,
				primitive, 
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
			if (hasAnimations && isGenerateAnimationProperty(data))
			{
				dst << TABx3 << "<boneassignments>\n";

				// Write bone assignments
				writeBoneAssignmentsToMesh(dst, node, primitive, data, startBinaryBuffer);

				dst << TABx3 << "</boneassignments>\n";
			}
		}

		dst << TABx2 << "</submesh>\n";
		++primitiveIndex;
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeFacesToMesh(std::ofstream& dst,
	const gLTFPrimitive& primitive,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	gLTFAccessor  indicesAccessor = mAccessorMap[primitive.mIndicesAccessor];
	if (indicesAccessor.mCount == 0)
		return false;

	// Read indices
	readIndicesFromUriOrFile (primitive, data, startBinaryBuffer);

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
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer,
	Ogre::Matrix4 matrix)
{
	// Read positions, normals, tangents,... etc.
	readPositionsFromUriOrFile(primitive, 
		data, 
		startBinaryBuffer, 
		matrix); // Read the positions
	readNormalsFromUriOrFile(primitive, data, startBinaryBuffer); // Read the normals
	readTangentsFromUriOrFile(primitive, data, startBinaryBuffer); // Read the tangents
	readColorsFromUriOrFile(primitive, data, startBinaryBuffer); // Read the diffuse colours
	readTexCoords0FromUriOrFile(primitive, data, startBinaryBuffer); // Read the uv's set 0
	readTexCoords1FromUriOrFile(primitive, data, startBinaryBuffer); // Read the uv's set 1

	// Write vertices; Assume that count of positions, texcoords, etc. is the same
	gLTFAccessor  positionAccessor = mAccessorMap[primitive.mPositionAccessorDerived];
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
		if (mTangentsMap.size() > 0 && !isGenerateTangentsProperty(data))
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
bool gLTFImportOgreMeshCreator::writeBoneAssignmentsToMesh (std::ofstream& dst,
	gLTFNode node,
	const gLTFPrimitive& primitive,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	gLTFAccessor positionAccessor = mAccessorMap[primitive.mPositionAccessorDerived]; // To determine number of vertices
	gLTFAccessor jointAccessor = mAccessorMap[primitive.mJoints_0AccessorDerived];
	gLTFAccessor weightAccessor = mAccessorMap[primitive.mWeights_0AccessorDerived];
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

		// Write the xml entry. A vertex can be influenced by max. 4 bones/joints. Note that a gLTF file may contains
		// multiple skins. The relation between the joint and the skin may not dissapear
		unsigned int joints[4];
		float weights[4];
		joints[0] = getBoneId(joint.x, findSkinIndexByNodeIndex(node.mNodeIndex));
		joints[1] = getBoneId(joint.y, findSkinIndexByNodeIndex(node.mNodeIndex));
		joints[2] = getBoneId(joint.z, findSkinIndexByNodeIndex(node.mNodeIndex));
		joints[3] = getBoneId(joint.w, findSkinIndexByNodeIndex(node.mNodeIndex));
		weights[0] = weight.x;
		weights[1] = weight.y;
		weights[2] = weight.z;
		weights[3] = weight.w;
		fCount = 0;
		while (fCount < 4)
		{
			//weights[fCount] = weights[fCount] < 0.00000001f ? 1.0f : weights[fCount];
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
bool gLTFImportOgreMeshCreator::writeBonesToSkeleton (std::ofstream& dst,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	OUT << TABx3 << "Perform gLTFImportOgreMeshCreator::writeBonesToSkeleton\n";

	/* Each bone is a joint in the skin object, A joint refers to a node.
	 * Iterate through the joints and get the node.
	 */
	std::map<int, gLTFSkin>::iterator itSkin;
	std::map<int, gLTFSkin>::iterator itSkinEnd = mSkinsMap.end();
	std::map<int, gLTFNode>::iterator itNode;
	std::map<int, gLTFNode>::iterator itNodeEnd;
	gLTFSkin skin;
	gLTFNode node;

	// Iterate through all skins, get the nodes that correspond to the joints
	unsigned int skinIndex = 0;
	unsigned int jointIndex = 0;
	for (itSkin = mSkinsMap.begin(); itSkin != itSkinEnd; itSkin++)
	{
		skin = itSkin->second;
		itNodeEnd = skin.mNodesDerived.end();
		jointIndex = 0;
		for (itNode = skin.mNodesDerived.begin(); itNode != itNodeEnd; itNode++)
		{
			node = itNode->second;
			dst << TABx2 <<
				"<bone id=\"" <<
				getBoneId(jointIndex, skinIndex) <<
				"\" name=\"" <<
				getBoneName(jointIndex, skinIndex) <<
				"\">\n";

			Ogre::Matrix4 matrix4;
			Ogre::Vector3 position;
			Ogre::Quaternion orientation;
			Ogre::Vector3 axis;
			Ogre::Radian angle;
			Ogre::Vector3 scale;

			/* Determine the bone pose matrix.
			 * TODO: What is the math??? The line below isn'y correct, but it is unclear what it must be!!!
			 */
			matrix4 = node.mLocalTransformation;
			//TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 

			matrix4.decomposition(position, scale, orientation);

			// Position
			dst << TABx3 << "<position x= \"" <<
				position.x <<
				"\" y=\"" <<
				position.y <<
				"\" z=\"" <<
				position.z <<
				"\" />\n";

			// Orientation
			orientation.ToAngleAxis(angle, axis);
			dst << TABx3 << "<rotation angle=\"" <<
				angle.valueRadians() <<
				"\">\n";
			dst << TABx4 << "<axis x= \"" <<
				axis.x <<
				"\" y=\"" <<
				axis.y <<
				"\" z=\"" <<
				axis.z <<
				"\" />\n";
			dst << TABx3 << "</rotation>\n";

			// Scale
			dst << TABx3 << "<scale x= \"" <<
				scale.x <<
				"\" y=\"" <<
				scale.y <<
				"\" z=\"" <<
				scale.z <<
				"\" />\n";

			dst << TABx2 << "</bone>\n";
			++jointIndex;
		}
		
		++skinIndex;
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeBoneHierarchyToSkeleton (std::ofstream& dst)
{
	OUT << TABx3 << "Perform gLTFImportOgreMeshCreator::writeBoneHierarchyToSkeleton\n";

	/* Iterate trough all joints, search its node and determine the parentnode of the node.
	 * Search the joint index of the parent node.
	 */
	std::map<int, gLTFSkin>::iterator itSkin;
	std::map<int, gLTFSkin>::iterator itSkinEnd = mSkinsMap.end();
	std::map<int, gLTFNode>::iterator itNode;
	std::map<int, gLTFNode>::iterator itNodeEnd;
	std::map<int, gLTFNode>::iterator itNodeParent;
	std::map<int, gLTFNode>::iterator itNodeParentEnd;
	gLTFSkin skin;
	gLTFNode node;
	gLTFNode* nodeParent;

	// Iterate through all skins, get the nodes that correspond to the joints
	unsigned int skinIndex = 0;
	unsigned int jointIndexChild = 0;
	unsigned int jointIndexParent = 0;
	std::string boneNameChild;
	std::string boneNameParent;
	for (itSkin = mSkinsMap.begin(); itSkin != itSkinEnd; itSkin++)
	{
		skin = itSkin->second;
		itNodeEnd = skin.mNodesDerived.end();
		for (itNode = skin.mNodesDerived.begin(); itNode != itNodeEnd; itNode++)
		{
			node = itNode->second;

			// Determine the parent node and search it back in the node list
			nodeParent = node.mParentNode;
			itNodeParentEnd = skin.mNodesDerived.end();
			jointIndexParent = 0;
			for (itNodeParent = skin.mNodesDerived.begin(); itNodeParent != itNodeParentEnd; itNodeParent++)
			{
				// If node indices are equal, this must be the node
				if (nodeParent->mNodeIndex == (itNodeParent->second).mNodeIndex)
					break;
				++jointIndexParent;
			}
			
			if (jointIndexParent < skin.mNodesDerived.size())
			{
				// Use separate strings for the names and do not use getBoneName twice in the stream, because  it
				// does not return the correct parent bone name
				boneNameChild = getBoneName(jointIndexChild, skinIndex);
				boneNameParent = getBoneName(jointIndexParent, skinIndex);
				dst << TABx2 << "<boneparent bone=\"" <<
					boneNameChild <<
					"\" parent=\"" <<
					boneNameParent <<
					"\" />\n";
			}

			++jointIndexChild;
		}

		++skinIndex;
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeAnimationsToSkeleton(std::ofstream& dst,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	OUT << TABx3 << "Perform gLTFImportOgreMeshCreator::writeAnimationsToSkeleton\n";

	dst << TAB << "<animations>\n";


	/* Iterate through all skins and joints (use nodes instead of joints for convenience).
	 * Assume, that each gLTF skin represents an Ogre3d animation (not sure whether this is true, but it seems
	 * logical).
	 */
	std::map<int, gLTFSkin>::iterator itSkin;
	std::map<int, gLTFSkin>::iterator itSkinEnd = mSkinsMap.end();
	std::map<int, gLTFNode>::iterator itNode;
	std::map<int, gLTFNode>::iterator itNodeEnd;
	std::map<int, gLTFNode>::iterator itNodeJoint;
	std::map<int, gLTFAnimation>::iterator itAnimation;
	gLTFSkin skin;
	unsigned int skinIndex = 0;
	unsigned int jointIndex = 0;
	bool hasTrackAndKeyframesHeader;
	std::vector<gLTFAnimationChannel> animationChannelsForNode; // All the animation channels in this animation for this node
	std::map<int, gLTFAnimationChannel>::iterator itAnimationChannel;
	gLTFNode node;
	gLTFAnimation animation;
	gLTFAnimationChannel animationChannel;

	for (itSkin = mSkinsMap.begin(); itSkin != itSkinEnd; itSkin++)
	{
		skin = itSkin->second;
		dst << TABx2 << "<animation name=\"" <<
			skin.mName <<
			"\" length=\"" <<
			getMaxTimeOfAnimation(skin, data, startBinaryBuffer) <<
			"\">\n";
		dst << TABx3 << "<tracks>\n";

		jointIndex = 0;
		hasTrackAndKeyframesHeader = false;
		itNodeEnd = skin.mNodesDerived.end();
		
		for (itNode = skin.mNodesDerived.begin(); itNode != itNodeEnd; itNode++)
		{
			node = itNode->second;
			hasTrackAndKeyframesHeader = false;

			// Iterate through all gLTF animations
			for (itAnimation = mAnimationsMap.begin(); itAnimation != mAnimationsMap.end(); itAnimation++)
			{
				animation = itAnimation->second;
				animationChannelsForNode.clear();

				// Iterate through all animations channels within the animation
				for (itAnimationChannel = animation.mAnimationChannelsMap.begin();
					itAnimationChannel != animation.mAnimationChannelsMap.end();
					itAnimationChannel++)
				{
					animationChannel = itAnimationChannel->second;
					if (animationChannel.mTargetNode == node.mNodeIndex)
					{
						if (!hasTrackAndKeyframesHeader)
						{
							dst << TABx4 <<
								"<track bone=\"" <<
								getBoneName(jointIndex, skinIndex) <<
								"\">\n";
							dst << TABx5 <<
								"<keyframes>\n";
							hasTrackAndKeyframesHeader = true;
						}

						animationChannelsForNode.push_back(animationChannel);
					}
				}

				// Now we know which channels refer to this bone. Write the keyframes
				if (!animationChannelsForNode.empty())
					writeKeyframesToSkeleton(dst, &animationChannelsForNode, data, startBinaryBuffer);
					//writeKeyframesToSkeleton(dst, skin, jointIndex, &animationChannelsForNode, data, startBinaryBuffer);
			}

			if (hasTrackAndKeyframesHeader)
			{
				dst << TABx5 << "</keyframes>\n";
				dst << TABx4 << "</track>\n";
			}

			++jointIndex;
		}

		++skinIndex;
	
		dst << TABx3 << "</tracks>\n";
		dst << TABx2 << "</animation>\n";
	}
	
	dst << TAB << "</animations>\n";
	
	return true;
}

//---------------------------------------------------------------------
float gLTFImportOgreMeshCreator::getMaxTimeOfAnimation (const gLTFSkin& skin,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	OUT << TABx4 << "Perform gLTFImportOgreMeshCreator::getMaxTimeOfAnimation\n";

	float maxTime = 0.0f;
	float time = 0.0f;

	// Iterate through all bones (nodes) within this animation
	std::map<int, gLTFNode>::iterator itNode;
	std::map<int, gLTFAnimation>::iterator itAnimation;
	std::map<int, gLTFAnimationChannel>::iterator itAnimationChannel;
	gLTFNode node;
	gLTFAnimation animation;
	gLTFAnimationChannel animationChannel;
	unsigned int nodeIndex = 0;
	for (itNode = mNodesMap.begin(); itNode != mNodesMap.end(); itNode++)
	{
		node = itNode->second;
		if (isNodeAJointInThisSkin(skin, node))
		{
			// The node is a joint of this skin. Iterate through all animations
			for (itAnimation = mAnimationsMap.begin(); itAnimation != mAnimationsMap.end(); itAnimation++)
			{
				animation = itAnimation->second;

				// Iterate through all animations channels within the animation
				for (itAnimationChannel = animation.mAnimationChannelsMap.begin();
					itAnimationChannel != animation.mAnimationChannelsMap.end();
					itAnimationChannel++)
				{
					animationChannel = itAnimationChannel->second;
					if (animationChannel.mTargetNode == nodeIndex)
					{
						// The node (bone) is defined in this channel
						time = getMaxTimeOfKeyframes(&animationChannel, data, startBinaryBuffer);
						maxTime = std::max(time, maxTime);
					}
				}
			}
		}

		++nodeIndex;

	}

	return maxTime;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::isNodeAJointInThisSkin (const gLTFSkin& skin, const gLTFNode& node)
{
	std::map<int, gLTFNode>::const_iterator itNode;
	std::map<int, gLTFNode>::const_iterator itNodeEnd = skin.mNodesDerived.end();
	gLTFNode nodeAsJoint;

	for (itNode = skin.mNodesDerived.begin(); itNode != itNodeEnd; itNode++)
	{
		nodeAsJoint = itNode->second;
		if (node.mNodeIndex == nodeAsJoint.mNodeIndex)
			return true;
	}

	return false;
}

//---------------------------------------------------------------------
float gLTFImportOgreMeshCreator::getMaxTimeOfKeyframes (gLTFAnimationChannel* animationChannel,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	OUT << TABx4 << "Perform gLTFImportOgreMeshCreator::getMaxTimeOfKeyframes\n";

	float maxTime = 0.0f;
	float time = 0.0f;
	gLTFAccessor  animationChannelAccessor;
	animationChannelAccessor = mAccessorMap[animationChannel->mInputDerived];
	char* buffer = getBufferChunk(animationChannelAccessor.mUriDerived, data, animationChannelAccessor, startBinaryBuffer);

	// Iterate through the chunk
	// You may assume that keyframeAccessor.mCount represents the max. time, but better be safe and read them all
	for (int i = 0; i < animationChannelAccessor.mCount; i++)
	{
		// A position must be a SCALAR/Float, otherwise it doesn't get read
		if (animationChannelAccessor.mType == "SCALAR" && animationChannelAccessor.mComponentType == gLTFAccessor::FLOAT)
		{
			time = mBufferReader.readFromFloatBuffer(buffer,
				i,
				animationChannelAccessor,
				getCorrectForMinMaxPropertyValue(data));
			maxTime = std::max(time, maxTime);
		}
	}

	delete[] buffer;

	return maxTime;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::writeKeyframesToSkeleton (std::ofstream& dst,
	std::vector<gLTFAnimationChannel>* animationChannelsForNode,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	float time = 0.0f;
	gLTFAccessor  inputAccessor;
	gLTFAccessor  outputAccessor;
	gLTFAnimationChannel animationChannel;
	Ogre::Vector3 axis;
	Ogre::Radian angle;
	char* inputBuffer;
	char* outputBuffer;
	Ogre::Vector3 translation;
	Ogre::Vector4 vec4Rotation;
	Ogre::Quaternion rotation;
	Ogre::Vector3 scale;
	std::map<float, Keyframe> keyframes;
	std::map<int, char*> inputBufferMap;
	std::map<int, char*> outputBufferMap;


	// The animation channels are all part of the same animation and the same node, assume they all refer to the 
	// same accessor (otherwise it isn't part of same set of keyframes)

	// Iterate through the channels and create the in- and output buffers
	unsigned int index = 0;
	std::vector<gLTFAnimationChannel>::iterator it;
	for (it = animationChannelsForNode->begin(); it != animationChannelsForNode->end(); it++)
	{
		animationChannel = *it;
		inputAccessor = mAccessorMap[animationChannel.mInputDerived];
		outputAccessor = mAccessorMap[animationChannel.mOutputDerived];
		inputBuffer = getBufferChunk(inputAccessor.mUriDerived, data, inputAccessor, startBinaryBuffer);
		outputBuffer = getBufferChunk(outputAccessor.mUriDerived, data, outputAccessor, startBinaryBuffer);
		inputBufferMap[index] = inputBuffer;
		outputBufferMap[index] = outputBuffer;
		++index;
	}

	// Iterate through the buffers to get the time and TRS values. According to the specs, the number of entries in the
	// inputbuffer must match the number of entries in the outputbuffer
	index = 0;
	for (it = animationChannelsForNode->begin(); it != animationChannelsForNode->end(); it++)
	{
		animationChannel = *it;
		inputAccessor = mAccessorMap[animationChannel.mInputDerived];
		outputAccessor = mAccessorMap[animationChannel.mOutputDerived];

		for (int i = 0; i < inputAccessor.mCount; i++)
		{
			if (inputAccessor.mType == "SCALAR" && inputAccessor.mComponentType == gLTFAccessor::FLOAT)
			{
				time = mBufferReader.readFromFloatBuffer(inputBufferMap[index],
					i,
					inputAccessor,
					getCorrectForMinMaxPropertyValue(data));
				keyframes[time].time = time;
			}

			// TRS
			if (animationChannel.mTargetPath == "translation")
			{
				translation = mBufferReader.readVec3FromFloatBuffer(outputBufferMap[index],
					i,
					outputAccessor,
					getCorrectForMinMaxPropertyValue(data));
				keyframes[time].hasTranslation = true;
				keyframes[time].translation = translation;
			}
			else if (animationChannel.mTargetPath == "rotation")
			{
				vec4Rotation = mBufferReader.readVec4FromFloatBuffer(outputBufferMap[index],
					i,
					outputAccessor,
					getCorrectForMinMaxPropertyValue(data));
				rotation.x = vec4Rotation.x;
				rotation.y = vec4Rotation.y;
				rotation.z = vec4Rotation.z;
				rotation.w = vec4Rotation.w;
				keyframes[time].rotation = rotation;
				keyframes[time].hasRotation = true;
			}
			else if (animationChannel.mTargetPath == "scale")
			{
				scale = mBufferReader.readVec3FromFloatBuffer(outputBufferMap[index],
					i,
					outputAccessor,
					getCorrectForMinMaxPropertyValue(data));
				keyframes[time].scale = scale;
				keyframes[time].hasScale = true;
			}
			// Weights are not supported
		}

		++index;
	}

	// Iterate through the in/outputbuffers and delete them
	std::map<int, char*>::iterator itOutputBuffers;
	for (itOutputBuffers = outputBufferMap.begin(); itOutputBuffers != outputBufferMap.end(); itOutputBuffers++)
	{
		outputBuffer = itOutputBuffers->second;
		delete[] outputBuffer;
	}
	std::map<int, char*>::iterator itInputBuffers;
	for (itInputBuffers = inputBufferMap.begin(); itInputBuffers != inputBufferMap.end(); itInputBuffers++)
	{
		inputBuffer = itInputBuffers->second;
		delete[] inputBuffer;
	}
	
	// Iterate through the keyframes
	std::map<float, Keyframe>::iterator itKeyframes;
	for (itKeyframes = keyframes.begin(); itKeyframes != keyframes.end(); itKeyframes++)
	{
		Keyframe keyframe = itKeyframes->second;
		dst << TABx6 <<
			"<keyframe time=\"" <<
			keyframe.time <<
			"\">\n";

		if (keyframe.hasTranslation)
		{
			dst << TABx7 << "<translate x=\"" <<
				keyframe.translation.x <<
				"\" y=\"" <<
				keyframe.translation.y <<
				"\" z=\"" <<
				keyframe.translation.z <<
				"\" />\n";
		}
		if (keyframe.hasRotation)
		{
			keyframe.rotation.ToAngleAxis(angle, axis);
			dst << TABx7 << "<rotate angle=\"" <<
				angle.valueRadians() <<
				"\">\n";
			dst << TABx8 << "<axis x=\"" <<
				axis.x <<
				"\" y=\"" <<
				axis.y <<
				"\" z=\"" <<
				axis.z <<
				"\" />\n";
			dst << TABx7 << "</rotate>\n";
		}
		if (keyframe.hasScale)
		{
			dst << TABx7 << "<scale x=\"" <<
				keyframe.scale.x <<
				"\" y=\"" <<
				keyframe.scale.y <<
				"\" z=\"" <<
				keyframe.scale.z <<
				"\" />\n";
		}
		dst << TABx6 << "</keyframe>\n";
	}

	return true;
}


//---------------------------------------------------------------------
void gLTFImportOgreMeshCreator::readPositionsFromUriOrFile (const gLTFPrimitive& primitive,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer,
	Ogre::Matrix4 matrix)
{
	// Open the buffer file and read positions
	gLTFAccessor  positionAccessor = mAccessorMap[primitive.mPositionAccessorDerived];
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
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	if (primitive.mNormalAccessorDerived < 0)
		return;

	// Get the buffer and read positions
	gLTFAccessor  normalAccessor = mAccessorMap[primitive.mNormalAccessorDerived];
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
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	if (primitive.mTangentAccessorDerived < 0)
		return;

	// Get the buffer file and read positions
	gLTFAccessor  tangentAccessor = mAccessorMap[primitive.mTangentAccessorDerived];
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
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	if (primitive.mColor_0AccessorDerived < 0)
		return;

	// Get the buffer file and read colours
	gLTFAccessor  mColor_0Accessor = mAccessorMap[primitive.mColor_0AccessorDerived];
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
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	if (primitive.mIndicesAccessor < 0)
		return;

	// Get the buffer file and read indices
	gLTFAccessor indicesAccessor = mAccessorMap[primitive.mIndicesAccessor];
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
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	if (primitive.mTexcoord_0AccessorDerived < 0)
		return;

	// Get the buffer file and read positions
	gLTFAccessor  mTexcoord_0Accessor = mAccessorMap[primitive.mTexcoord_0AccessorDerived];
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
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	if (primitive.mTexcoord_1AccessorDerived < 0)
		return;

	// Get the buffer file and read positions
	gLTFAccessor  mTexcoord_1Accessor = mAccessorMap[primitive.mTexcoord_1AccessorDerived];
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

//---------------------------------------------------------------------
std::vector<gLTFAnimationChannel> gLTFImportOgreMeshCreator::getAnimationChannelsByNode (int nodeIndex)
{
	mHelperAnimationChannelVector.clear();
	std::map<int, gLTFAnimation>::iterator it;
	std::map<int, gLTFAnimation>::iterator itEnd = mAnimationsMap.end();
	std::map<int, gLTFAnimationChannel>::iterator itChannels;
	std::map<int, gLTFAnimationChannel>::iterator itChannelsEnd;
	gLTFAnimation animation;
	gLTFAnimationChannel animationChannel;
	for (it = mAnimationsMap.begin(); it != itEnd; it++)
	{
		animation = it->second;
		for (itChannels = animation.mAnimationChannelsMap.begin(); itChannels != animation.mAnimationChannelsMap.end(); itChannels++)
		{
			animationChannel = itChannels->second;
			if (animationChannel.mTargetNode == nodeIndex)
				mHelperAnimationChannelVector.push_back(animationChannel);
		}
	}

	return mHelperAnimationChannelVector;
}

//---------------------------------------------------------------------
gLTFAnimation gLTFImportOgreMeshCreator::getAnimationByIndex (int animationIndex)
{
	mHelperAnimation = gLTFAnimation();
	std::map<int, gLTFAnimation>::iterator it = mAnimationsMap.find(animationIndex);
	if (it != mAnimationsMap.end())
	{
		// Found the node
		mHelperAnimation = it->second;
		return mHelperAnimation;
	}

	return mHelperAnimation;
}

//---------------------------------------------------------------------
gLTFNode gLTFImportOgreMeshCreator::getNodeByIndex (int nodeIndex)
{
	mHelperNode = gLTFNode();
	std::map<int, gLTFNode>::iterator it = mNodesMap.find(nodeIndex);
	if (it != mNodesMap.end())
	{
		// Found the node
		mHelperNode = it->second;
		return mHelperNode;
	}

	return mHelperNode;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::isGenerateTangentsProperty (Ogre::HlmsEditorPluginData* data)
{
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY> properties = data->mInPropertiesMap;
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY>::iterator it = properties.find("generate_tangents");
	if (it != properties.end())
		return (it->second).boolValue;

	return false;
}

//---------------------------------------------------------------------
bool gLTFImportOgreMeshCreator::isGenerateAnimationProperty (Ogre::HlmsEditorPluginData* data)
{
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY> properties = data->mInPropertiesMap;
	std::map<std::string, Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY>::iterator it = properties.find("generate_animation");
	if (it != properties.end())
		return (it->second).boolValue;

	return false;
}

//---------------------------------------------------------------------
int gLTFImportOgreMeshCreator::findSkinIndexByNodeIndex (unsigned int nodeIndex)
{
	std::map<int, gLTFNode>::iterator itNode;
	gLTFNode node;
	for (itNode = mNodesMap.begin(); itNode != mNodesMap.end(); itNode++)
	{
		node = itNode->second;
		if (node.mNodeIndex == nodeIndex)
		{
			return node.mSkin;
		}
	}

	return 0; // Use default skin
}


//---------------------------------------------------------------------
unsigned int gLTFImportOgreMeshCreator::getBoneId (unsigned int jointIndex, unsigned int skinIndex)
{
	return jointIndex + MAX_JOINTS_PER_SKIN * skinIndex; // Must be a unique id over all the skins
}

//---------------------------------------------------------------------
const std::string& gLTFImportOgreMeshCreator::getBoneName (unsigned int jointIndex, unsigned int skinIndex)
{
	unsigned int indexForName = getBoneId(jointIndex, skinIndex);
	std::stringstream ss;
	ss << "Bone_" << indexForName;
	mHelperString = ss.str();
	return mHelperString;
}

//---------------------------------------------------------------------
const Ogre::Matrix4& gLTFImportOgreMeshCreator::getInverseBindMatrix (unsigned int inverseBindMatricesAccessorIndex,
	unsigned int jointIndex,
	Ogre::HlmsEditorPluginData* data,
	int startBinaryBuffer)
{
	mHelperMatrix4 = Ogre::Matrix4();
	gLTFAccessor  inverseBindMatricesAccessor;
	inverseBindMatricesAccessor = mAccessorMap[inverseBindMatricesAccessorIndex];
	char* buffer = getBufferChunk(inverseBindMatricesAccessor.mUriDerived, data, inverseBindMatricesAccessor, startBinaryBuffer);

	// Assume that jointIndex also represents the index of the matrix4 in this buffer
	mHelperMatrix4 = mBufferReader.readMatrix4FromFloatBuffer(buffer,
		jointIndex,
		inverseBindMatricesAccessor,
		getCorrectForMinMaxPropertyValue(data));
	
	delete[] buffer;
	return mHelperMatrix4;
}


//---------------------------------------------------------------------
/*
gLTFNode* gLTFImportOgreMeshCreator::findNodeByIndex (int nodeIndex)
{
	std::map<int, gLTFNode>::iterator it = mNodesMap.find(nodeIndex);
	if (it != mNodesMap.end())
	{
		// Found the node
		return &(it->second);
	}

	return 0;
}
*/