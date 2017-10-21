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
#include "gLTFImportConstants.h"
#include "gLTFMesh.h"
#include "gLTFNode.h"
#include "gLTFAccessor.h"
#include "hlms_editor_plugin.h"
#include "gLTFImportBufferReader.h"
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
			std::map<int, gLTFAccessor> accessorMap,
			int startBinaryBuffer); // Creates *.xml and .mesh files

		bool createIndividualOgreMeshFiles(Ogre::HlmsEditorPluginData* data,
			std::map<int, gLTFMesh> meshesMap,
			std::map<int, gLTFAccessor> accessorMap,
			int startBinaryBuffer); // Creates individual files

		bool createCombinedOgreMeshFile (Ogre::HlmsEditorPluginData* data,
			std::map<int, gLTFNode> nodesMap,
			std::map<int, gLTFAccessor> accessorMap,
			int startBinaryBuffer); // Combine into one file
	
	protected:
		// Write to mesh .xml file
		bool writeSubmesh(std::ofstream& dst,
			gLTFMesh mesh,
			Ogre::HlmsEditorPluginData* data,
			std::map<int, gLTFAccessor> accessorMap,
			int startBinaryBuffer,
			bool hasTrs = true,								// If true, the translation, rotation and scale args are used
			Ogre::Vector3 translation = Ogre::Vector3(),
			Ogre::Quaternion rotation = Ogre::Quaternion(),
			Ogre::Vector3 scale = Ogre::Vector3(),
			Ogre::Matrix4 matrix = Ogre::Matrix4());

		bool writeFaces (std::ofstream& dst,
			const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Write all faces

		bool writeVertices (std::ofstream& dst, 
			const gLTFPrimitive& primitive, 
			std::map<int, 
			gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer,
			bool hasTrs = true,
			Ogre::Vector3 translation = Ogre::Vector3(),
			Ogre::Quaternion rotation = Ogre::Quaternion(),
			Ogre::Vector3 scale = Ogre::Vector3(),
			Ogre::Matrix4 matrix = Ogre::Matrix4()); // Write all vertices
		
		// Read attributes from buffer
		void readPositionsFromUriOrFile (const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer,
			bool hasTrs = true,
			Ogre::Vector3 translation = Ogre::Vector3(),
			Ogre::Quaternion rotation = Ogre::Quaternion(),
			Ogre::Vector3 scale = Ogre::Vector3(),
			Ogre::Matrix4 matrix = Ogre::Matrix4()); // Read the positions

		void readNormalsFromUriOrFile (const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the normals

		void readTangentsFromUriOrFile (const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the tangents

		void readIndicesFromUriOrFile (const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the indices

		void readColorsFromUriOrFile (const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the diffuse colours

		void readTexCoords0FromUriOrFile (const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
			Ogre::HlmsEditorPluginData* data,
			int startBinaryBuffer); // Read the texcoords 0

		void readTexCoords1FromUriOrFile (const gLTFPrimitive& primitive,
			std::map<int, gLTFAccessor> accessorMap,
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

		bool getCorrectForMinMaxPropertyValue(Ogre::HlmsEditorPluginData* data);
		bool setMeshFileNamePropertyValue(Ogre::HlmsEditorPluginData* data, const std::string& fileName);
	
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
		Ogre::Vector4 mHelperVec4Struct;
		Ogre::Vector3 mHelperVec3Struct;
		Ogre::Vector2 mHelperVec2Struct;
		gLTFImportBufferReader mBufferReader;
};

#endif
