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

#include "gLTFImportSamplersParser.h"

//---------------------------------------------------------------------
bool gLTFImportSamplersParser::parseSamplers (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportSamplersParser::parseSamplers\n";

	int source = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through samplers array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		gLTFSampler sampler;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();

		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key sampler ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsInt() && key == "magFilter")
			{
				// ******** 1. magFilter ********
				sampler.mMagFilter = it->value.GetInt();
				OUT << TABx2 << "value ==> " << sampler.mMagFilter << "\n";
			}
			if (it->value.IsInt() && key == "minFilter")
			{
				// ******** 2. minFilter ********
				sampler.mMinFilter = it->value.GetInt();
				OUT << TABx2 << "value ==> " << sampler.mMinFilter << "\n";
			}
			if (it->value.IsInt() && key == "wrapS")
			{
				// ******** 3. wrapS ********
				sampler.mWrapS = it->value.GetInt();
				OUT << TABx2 << "value ==> " << sampler.mWrapS << "\n";
			}
			if (it->value.IsInt() && key == "wrapT")
			{
				// ******** 4. wrapT ********
				sampler.mWrapT = it->value.GetInt();
				OUT << TABx2 << "value ==> " << sampler.mWrapT << "\n";
			}
			if (it->value.IsString() && key == "name")
			{
				// ******** 5. name ********
				sampler.mName = it->value.GetString();
				OUT << TABx2 << "value ==> " << sampler.mName << "\n";
			}
		}
		
		mSamplersMap[source] = sampler;
		++source;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFSampler> gLTFImportSamplersParser::getParsedSamplers (void) const
{
	return mSamplersMap;
}
