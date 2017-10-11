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

	int index = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through Nodes array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		gLTFNode node;
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
					if (matrixArray[j].IsFloat())
					{
						node.mMatrix[j] = matrixArray[j].GetFloat();
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
					if (rotationArray[j].IsFloat())
					{
						node.mRotation[j] = rotationArray[j].GetFloat();
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
					if (scaleArray[j].IsFloat())
					{
						node.mScale[j] = scaleArray[j].GetFloat();
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
					if (translationArray[j].IsFloat())
					{
						node.mTranslation[j] = translationArray[j].GetFloat();
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
				node.mName = it->value.GetString();
				OUT << TABx2 << "value ==> " << node.mName << "\n";
			}
		}
		
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
