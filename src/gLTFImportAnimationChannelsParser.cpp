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

#include "gLTFAnimationChannel.h"
#include "gLTFImportAnimationChannelsParser.h"

//---------------------------------------------------------------------
bool gLTFImportAnimationChannelsParser::parseAnimationChannels (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportAnimationChannelsParser::parseAnimationChannels\n";

	int index = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through animation channels array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		OUT << TAB << "animation channel index ==> " << index << "\n";
		gLTFAnimationChannel animationChannel;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();

		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key animation channel ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsInt() && key == "sampler")
			{
				// ******** 1. sampler ********
				animationChannel.mSampler = it->value.GetInt();
				OUT << TABx2 << "value ==> " << animationChannel.mSampler << "\n";
			}
			if (it->value.IsObject() && key == "target")
			{
				// ******** 2. target ********
				parseTarget(it, &animationChannel);
			}
		}
		
		mAnimationChannelsMap[index] = animationChannel;
		++index;
	}

	return true;
}

//---------------------------------------------------------------------
bool gLTFImportAnimationChannelsParser::parseTarget (rapidjson::Value::ConstMemberIterator jsonIterator,
	gLTFAnimationChannel* animationChannel)
{
	OUT << TABx2 << "Perform gLTFImportAnimationChannelsParser::parseTarget\n";

	rapidjson::Value::ConstMemberIterator it;
	rapidjson::Value::ConstMemberIterator itEnd = jsonIterator->value.MemberEnd();
	for (it = jsonIterator->value.MemberBegin(); it != itEnd; ++it)
	{
		OUT << TABx3 << "key target ==> " << it->name.GetString() << "\n";
		std::string key = std::string(it->name.GetString());
		if (it->value.IsInt() && key == "node")
		{
			// ******** 1. node ********
			animationChannel->mTargetNode = it->value.GetInt();
			OUT << TABx3 << "value ==> " << animationChannel->mTargetNode << "\n";
		}
		if (it->value.IsString() && key == "path")
		{
			// ******** 2. path ********
			animationChannel->mTargetPath = std::string(it->value.GetString());
			OUT << TABx3 << "value ==> " << animationChannel->mTargetPath << "\n";
		}
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFAnimationChannel> gLTFImportAnimationChannelsParser::getParsedAnimationChannels (void) const
{
	return mAnimationChannelsMap;
}



