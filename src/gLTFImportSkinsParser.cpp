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

#include "gLTFImportSkinsParser.h"

//---------------------------------------------------------------------
bool gLTFImportSkinsParser::parseSkins (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportSkinsParser::parseSkins\n";

	int index = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through Skins array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		OUT << TAB << "skin index ==> " << index << "\n";
		gLTFSkin skin;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();

		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key skin ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			
			if (it->value.IsInt() && key == "inverseBindMatrices")
			{
				// ******** 1. inverseBindMatrices ********
				skin.mInverseBindMatrices = it->value.GetInt();
				OUT << TABx2 << "value ==> " << skin.mInverseBindMatrices << "\n";
			}
			if (it->value.IsInt() && key == "skeleton")
			{
				// ******** 2. skeleton ********
				skin.mSkeleton = it->value.GetInt();
				OUT << TABx2 << "value ==> " << skin.mSkeleton << "\n";
			}
			if (it->value.IsArray() && key == "joints")
			{
				// ******** 3. joints ********
				//std::map<int, int> mJoints;

				const rapidjson::Value& jointsArray = it->value;
				for (rapidjson::SizeType iJoint = 0; iJoint < jointsArray.Size(); iJoint++)
				{
					if (jointsArray[iJoint].IsInt())
					{
						skin.mJoints[iJoint] = jointsArray[iJoint].GetInt();
						OUT << TABx2 << "value ==> " << skin.mJoints[iJoint] << "\n";
					}
				}
			}
			if (it->value.IsString() && key == "name")
			{
				// ******** 4. name ********
				skin.mName = it->value.GetString();
				OUT << TABx2 << "value ==> " << skin.mName << "\n";
			}
		}

		if (skin.mName == "")
		{
			// Generate a name if not provided. For now, assume that each gLTF skin represents an Ogre3d animation
			std::stringstream ss;
			ss << "Animation_" << index;
			skin.mName = ss.str();
		}
		mSkinsMap[index] = skin;
		++index;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFSkin> gLTFImportSkinsParser::getParsedSkins(void) const
{
	return mSkinsMap;
}
