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

#include "gLTFImportAccessorsParser.h"

//---------------------------------------------------------------------
bool gLTFImportAccessorsParser::parseAccessors (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportAccessorsParser::parseAccessors\n";

	int index = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through accessors array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		gLTFAccessor accessor;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();

		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key accessor ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsInt() && key == "componentType")
			{
				// ******** 1. componentType ********
				accessor.mComponentType = it->value.GetInt();
				OUT << TABx2 << "value ==> " << accessor.mComponentType << "\n";
			}
			if (it->value.IsInt() && key == "bufferView")
			{
				// ******** 2. bufferView ********
				accessor.mBufferView = it->value.GetInt();
				OUT << TABx2 << "value ==> " << accessor.mBufferView << "\n";
			}
			if (it->value.IsInt() && key == "byteOffset")
			{
				// ******** 3. byteOffset ********
				accessor.mByteOffset = it->value.GetInt();
				OUT << TABx2 << "value ==> " << accessor.mByteOffset << "\n";
			}
			if (it->value.IsBool() && key == "normalized")
			{
				// ******** 4. normalized ********
				accessor.mNormalized = it->value.GetBool();
				OUT << TABx2 << "value ==> " << accessor.mNormalized << "\n";
			}
			if (it->value.IsInt() && key == "count")
			{
				// ******** 5. count ********
				accessor.mCount = it->value.GetInt();
				OUT << TABx2 << "value ==> " << accessor.mCount << "\n";
			}
			if (it->value.IsString() && key == "type")
			{
				// ******** 6. type ********
				accessor.mType = it->value.GetString();
				OUT << TABx2 << "value ==> " << accessor.mType << "\n";
			}
			if (it->value.IsInt() && key == "min")
			{
				// ******** 7. min ********
				accessor.mMin = it->value.GetInt();
				OUT << TABx2 << "value ==> " << accessor.mMin << "\n";
				// TODO: Min is an array
			}
			if (it->value.IsInt() && key == "max")
			{
				// ******** 8. max ********
				accessor.mMax = it->value.GetInt();
				OUT << TABx2 << "value ==> " << accessor.mMax << "\n";
				// TODO: Max is an array
			}
			if (it->value.IsArray() && key == "sparse")
			{
				// ******** 9. sparse ********
				// TODO: parse Sparse object
			}
			if (it->value.IsString() && key == "name")
			{
				// ******** 10. name ********
				accessor.mName = it->value.GetString();
				OUT << TABx2 << "value ==> " << accessor.mName << "\n";
			}
		}
		
		mAccessorsMap[index] = accessor;
		++index;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFAccessor> gLTFImportAccessorsParser::getParsedAccessors(void) const
{
	return mAccessorsMap;
}
