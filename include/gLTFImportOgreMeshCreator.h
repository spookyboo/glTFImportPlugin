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
#ifndef __gLTFImportOgreMeshCreator_H__
#define __gLTFImportOgreMeshCreator_H__

#include <map>
#include <vector>
#include "gLTFImportConstants.h"
#include "gLTFMesh.h"
#include "gLTFNode.h"
#include "gLTFAnimation.h"
#include "gLTFAccessor.h"
#include "gLTFSkin.h"
#include "hlms_editor_plugin.h"
#include "gLTFImportBufferReader.h"
#include "gLTFAnimationChannel.h" 
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreMatrix4.h"

/** Class responsible for creation of Ogre meshes */
class gLTFImportOgreMeshCreator
{
	public:
		gLTFImportOgreMeshCreator(void);
		virtual ~gLTFImportOgreMeshCreator (void) {};

		// Create the Ogre Mesh files
		bool createOgreMeshFiles (Ogre::HlmsEditorPluginData* data,
			std::map<int, gLTFNode> nodesMap,
			std::map<int, gLTFMesh> meshesMap,
			std::map<int, gLTFAnimation> animationsMap,
			std::map<int, gLTFSkin> skinsMap,
			std::map<int, gLTFAccessor> accessorMap,
			int startBinaryBuffer,
			bool hasAnimations); // Creates *.xml and .mesh files

		bool createIndividualOgreMeshFiles (Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Creates individual files

		bool createCombinedOgreMeshFile (Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer,
			bool hasAnimations); // Combine into one file

		bool createCombinedOgreSkeletonFile (Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer,
			bool hasAnimations);
	
	protected:
		struct Keyframe
		{
			float time = 0.0f;
			bool hasTranslation = false;
			Ogre::Vector3 translation;
			bool hasRotation = false;
			Ogre::Quaternion rotation;
			bool hasScale = false;
			Ogre::Vector3 scale;
		};

		// Write to mesh .xml file
		bool writeSubmeshToMesh (std::ofstream& dst,
			gLTFNode node,
			gLTFMesh mesh,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer,
			Ogre::Matrix4 matrix = Ogre::Matrix4(),
			bool hasAnimations = false);

		bool writeFacesToMesh(std::ofstream& dst,
			const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Write all faces

		bool writeVerticesToMesh(std::ofstream& dst,
			const gLTFPrimitive& primitive, 
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer,
			Ogre::Matrix4 matrix = Ogre::Matrix4()); // Write all vertices of a submesh

		bool writeBoneAssignmentsToMesh(std::ofstream& dst,
			gLTFNode node,
			const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data, 
			int startBinaryBuffer); // Write all bone assignments of a submesh

		// Write bones to skeleton.xml file
		bool writeBonesToSkeleton(std::ofstream& dst,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer);

		// Write bone hierarchy to skeleton.xml file
		bool writeBoneHierarchyToSkeleton (std::ofstream& dst);

		// Write animation to skeleton.xml file
		bool writeAnimationsToSkeleton (std::ofstream& dst,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer);

		// Write keyframe header to skeleton.xml file
		bool writeKeyframesToSkeleton (std::ofstream& dst,
			std::vector<gLTFAnimationChannel>* animationChannelsForNode,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer);

		// Read attributes from buffer
		void readPositionsFromUriOrFile (const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer,
			Ogre::Matrix4 matrix = Ogre::Matrix4()); // Read the positions

		void readNormalsFromUriOrFile (const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the normals

		void readTangentsFromUriOrFile (const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the tangents

		void readIndicesFromUriOrFile (const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the indices

		void readColorsFromUriOrFile (const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the diffuse colours

		void readTexCoords0FromUriOrFile (const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the texcoords 0

		void readTexCoords1FromUriOrFile (const gLTFPrimitive& primitive,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the texcoords 1

		// Utils
		char* getBufferChunk (const std::string& uri, 
			Ogre::HlmsEditorPluginData* data, 
			gLTFAccessor accessor, 
			int startBinaryBuffer);
		
		bool convertXmlFileToMesh (Ogre::HlmsEditorPluginData* data, 
			const std::string& xmlFileName, 
			const std::string& meshFileName);

		bool convertXmlFileToSkeleton (Ogre::HlmsEditorPluginData* data,
			const std::string& xmlFileName,
			const std::string& skeletonFileName);

		bool getCorrectForMinMaxPropertyValue (Ogre::HlmsEditorPluginData* data);
		bool setMeshFileNamePropertyValue (Ogre::HlmsEditorPluginData* data, const std::string& fileName);
		bool setSkeletonFileNamePropertyValue (Ogre::HlmsEditorPluginData* data, const std::string& fileName);
		std::vector<gLTFAnimationChannel> getAnimationChannelsByNode (int nodeIndex);
		gLTFAnimation getAnimationByIndex (int animationIndex);
		gLTFNode getNodeByIndex (int nodeIndex);
			
		// Returns the property value to generate tangents; returns false if the property isn't available
		bool isGenerateTangentsProperty (Ogre::HlmsEditorPluginData* data);

		// Returns the property value to generate animations; returns false if the property isn't available
		bool isGenerateAnimationProperty (Ogre::HlmsEditorPluginData* data);

		// Determine the time of an animation
		float getMaxTimeOfAnimation (const std::string& animationName,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer);

		// Determine the largest keyframe time
		float getMaxTimeOfKeyframes (gLTFAnimationChannel* animationChannel,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer);

		/* gLTF's animation system does not map clearly to Ogre's animation system. One animation in gLTF concerns 
		 * the movement of one part of the model (one or more bones), e.q. moving a leg for walking, while another gLTF 
		 * animation is involved with moving the other leg. They are individual animations, but a regular (Ogre) animation 
		 * is a combination of multiple gLTF animations (e.g. "walking").
		 * The gLTF specs do not make clear which gLTF animations belong to each other, an which do not.
		 * Current implementation in this plugin assumes that there is only 1 Ogre animation in the skeleton file and
		 * combines all gLTF animations.
		 */
		bool findAllUniqueOgreSkeletonAnimations (void);

		/* The primitives of a mesh refer to a joint index. To not forget the relation between the joint index and the 
		 * skin, the skin must be determined by the node that also refers to the mesh
		 */
		int findSkinIndexByNodeIndex (unsigned int nodeIndex);

		/* Because a gLTF model can have multiple skins, the model therefore has joints with the same index. This may give
		 * an issue when the relation between a vertex and a joint (=bone) is made. The index of a joint alone cannot be used.
		 * To identify a joint, a unique id must be given. This id is determined based on skin index and joint index.
		 */
		unsigned int getBoneId (unsigned int jointIndex, unsigned int skinIndex);
		const std::string& getBoneName (unsigned int jointIndex, unsigned int skinIndex);

		// Returns the InverseBindMatrix of a joint
		const Ogre::Matrix4& getInverseBindMatrix (unsigned int inverseBindMatricesAccessorIndex,
			unsigned int jointIndex,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer);

	private:
		std::string mHelperString;
		std::string fileNameBufferHelper;
		std::map <int, Ogre::Vector3> mPositionsMap;
		std::map <int, Ogre::Vector3> mNormalsMap;
		std::map <int, Ogre::Vector4> mTangentsMap;
		std::map <int, Ogre::Vector4> mColor_0AccessorMap;
		std::map <int, Ogre::Vector2> mTexcoords_0Map;
		std::map <int, Ogre::Vector2> mTexcoords_1Map;
		std::map <int, unsigned int> mIndicesMap;
		std::map<int, gLTFNode> mNodesMap;
		std::map<int, gLTFMesh> mMeshesMap;
		std::map<int, gLTFAnimation> mAnimationsMap;
		std::map<int, gLTFSkin> mSkinsMap;
		std::map<int, gLTFAccessor> mAccessorMap;
		std::map<int, std::string> mOgreSkeletonAnimationsMap; // Contains a list of all unique animations in Ogre
		Ogre::Vector4 mHelperVec4Struct;
		Ogre::Vector3 mHelperVec3Struct;
		Ogre::Vector2 mHelperVec2Struct;
		Ogre::Matrix4 mHelperMatrix4;
		gLTFImportBufferReader mBufferReader;
		gLTFAnimation mHelperAnimation;
		gLTFAnimationChannel mHelperAnimationChannel;
		gLTFAnimationSampler mHelperAnimationSampler;
		std::vector<gLTFAnimationChannel> mHelperAnimationChannelVector;
		gLTFNode mHelperNode;
};

#endif
