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

#include "gLTFImportNodesParser.h"

//---------------------------------------------------------------------
bool gLTFImportNodesParser::parseNodes (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportNodesParser::parseNodes\n";

	std::string name;
	int index = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through Nodes array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		OUT << TAB << "node index ==> " << index << "\n";
		gLTFNode node;
		node.mName = "";
		name = "";
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();

		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key node ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			
			if (it->value.IsInt() && key == "camera")
			{
				// ******** 1. camera ********
				node.mCamera = it->value.GetInt();
				OUT << TABx2 << "value ==> " << node.mCamera << "\n";
			}
			if (it->value.IsArray() && key == "children")
			{
				// ******** 2. children ********
				const rapidjson::Value& childrenArray = it->value;
				for (rapidjson::SizeType j = 0; j < childrenArray.Size(); j++)
				{
					if (childrenArray[j].IsInt())
					{
						node.mChildren.push_back(childrenArray[j].GetInt());
						OUT << TABx2 << "value ==> " << node.mChildren[j] << "\n";
					}
				}
			}
			if (it->value.IsInt() && key == "skin")
			{
				// ******** 3. skin ********
				node.mSkin = it->value.GetInt();
				OUT << TABx2 << "value ==> " << node.mSkin << "\n";
			}
			if (it->value.IsArray() && key == "matrix")
			{
				// ******** 4. matrix ********
				const rapidjson::Value& matrixArray = it->value;
				for (rapidjson::SizeType j = 0; j < matrixArray.Size(); j++)
				{
					if (matrixArray[j].IsNumber())
					{
						node.mMatrix[j] = matrixArray[j].GetDouble();
						node.mHasMatrix = true;
						OUT << TABx2 << "value ==> " << node.mMatrix[j] << "\n";
					}
				}
			}
			if (it->value.IsInt() && key == "mesh")
			{
				// ******** 5. mesh ********
				node.mMesh = it->value.GetInt();
				OUT << TABx2 << "value ==> " << node.mMesh << "\n";
			}
			if (it->value.IsArray() && key == "rotation")
			{
				// ******** 6. rotation ********
				const rapidjson::Value& rotationArray = it->value;
				for (rapidjson::SizeType j = 0; j < rotationArray.Size(); j++)
				{
					if (rotationArray[j].IsNumber())
					{
						node.mRotation[j] = rotationArray[j].GetDouble();
						node.mHasRotation = true;
						OUT << TABx2 << "value ==> " << node.mRotation[j] << "\n";
					}
				}
			}
			if (it->value.IsArray() && key == "scale")
			{
				// ******** 7. scale ********
				const rapidjson::Value& scaleArray = it->value;
				for (rapidjson::SizeType j = 0; j < scaleArray.Size(); j++)
				{
					if (scaleArray[j].IsNumber())
					{
						node.mScale[j] = scaleArray[j].GetDouble();
						node.mHasScale = true;
						OUT << TABx2 << "value ==> " << node.mScale[j] << "\n";
					}
				}
			}
			if (it->value.IsArray() && key == "translation")
			{
				// ******** 8. translation ********
				const rapidjson::Value& translationArray = it->value;
				for (rapidjson::SizeType j = 0; j < translationArray.Size(); j++)
				{
					if (translationArray[j].IsNumber())
					{
						node.mTranslation[j] = translationArray[j].GetDouble();
						node.mHasTranslation = true;
						OUT << TABx2 << "value ==> " << node.mTranslation[j] << "\n";
					}
				}
			}
			if (it->value.IsArray() && key == "weights")
			{
				// ******** 9. weights ********
				const rapidjson::Value& weightsArray = it->value;
				for (rapidjson::SizeType j = 0; j < weightsArray.Size(); j++)
				{
					if (weightsArray[j].IsFloat())
					{
						node.mWeights.push_back(weightsArray[j].GetFloat());
						OUT << TABx2 << "value ==> " << node.mWeights[j] << "\n";
					}
				}
			}
			if (it->value.IsString() && key == "name")
			{
				// ******** 10. name ********
				name = it->value.GetString();
				node.mName = name;
				OUT << TABx2 << "value ==> " << node.mName << "\n";
			}
		}

		Ogre::Vector3 scale = Ogre::Vector3(1.0f, 1.0f, 1.0f);
		Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
		Ogre::Vector3 translation = Ogre::Vector3::ZERO;
		Ogre::Matrix4 matrix;
		if (node.mHasMatrix)
		{
			// Set the matrix as calculated matrix (will be overruled later in some cases)
			matrix[0][0] = node.mMatrix[0];
			matrix[1][0] = node.mMatrix[1];
			matrix[2][0] = node.mMatrix[2];
			matrix[3][0] = node.mMatrix[3];

			matrix[0][1] = node.mMatrix[4];
			matrix[1][1] = node.mMatrix[5];
			matrix[2][1] = node.mMatrix[6];
			matrix[3][1] = node.mMatrix[7];

			matrix[0][2] = node.mMatrix[8];
			matrix[1][2] = node.mMatrix[9];
			matrix[2][2] = node.mMatrix[10];
			matrix[3][2] = node.mMatrix[11];

			matrix[0][3] = node.mMatrix[12];
			matrix[1][3] = node.mMatrix[13];
			matrix[2][3] = node.mMatrix[14];
			matrix[3][3] = node.mMatrix[15];
		}
		else
		{
			// Set all 3 TRS elements as calculated matrix 
			if (node.mHasTranslation)
			{
				translation.x = node.mTranslation[0];
				translation.y = node.mTranslation[1];
				translation.z = node.mTranslation[2];
			}
			if (node.mHasRotation)
			{
				rotation.x = node.mRotation[0];
				rotation.y = node.mRotation[1];
				rotation.z = node.mRotation[2];
				rotation.w = node.mRotation[3];
			}
			if (node.mHasScale)
			{
				scale.x = node.mScale[0];
				scale.y = node.mScale[1];
				scale.z = node.mScale[2];
			}
			matrix.makeTransform(translation, scale, rotation);
		}
		
		if (name == "")
		{
			// Generate a name if not provided (name is optional in gLTF)
			std::stringstream ss;
			ss << "Node_" << index;
			name = ss.str();
			node.mName = name;
		}

		node.mCalculatedTransformation = matrix;
		node.mOwnTransformation = matrix;
		node.mNodeIndex = index;
		mNodesMap[index] = node;
		++index;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFNode> gLTFImportNodesParser::getParsedNodes(void) const
{
	return mNodesMap;
}
