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

#include "gLTFImportBufferViewsParser.h"

//---------------------------------------------------------------------
bool gLTFImportBufferViewsParser::parseBufferViews (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportBufferViewsParser::parseBufferViews\n";

	int source = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through BufferViews array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		OUT << TAB << "Source index " << source << "\n";
		gLTFBufferView bufferView;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();
		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key bufferView ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsInt() && key == "buffer")
			{
				// ******** 1. buffer (index) ********
				bufferView.mBufferIndex = it->value.GetInt();
				OUT << TABx2 << "value ==> " << bufferView.mBufferIndex << "\n";
			}
			if (it->value.IsInt() && key == "byteOffset")
			{
				// ******** 2. byteOffset ********
				bufferView.mByteOffset = it->value.GetInt();
				OUT << TABx2 << "value ==> " << bufferView.mByteOffset << "\n";
			}
			if (it->value.IsInt() && key == "byteLength")
			{
				// ******** 3. byteLength ********
				bufferView.mByteLength = it->value.GetInt();
				OUT << TABx2 << "value ==> " << bufferView.mByteLength << "\n";
			}
			if (it->value.IsInt() && key == "byteStride")
			{
				// ******** 4. byteStride ********
				bufferView.mByteStride = it->value.GetInt();
				OUT << TABx2 << "value ==> " << bufferView.mByteStride << "\n";
			}
			if (it->value.IsInt() && key == "target")
			{
				// ******** 5. target ********
				bufferView.mTarget = it->value.GetInt();
				OUT << TABx2 << "value ==> " << bufferView.mTarget << "\n";
			}
			if (it->value.IsString() && key == "name")
			{
				// ******** 6. name ********
				bufferView.mName = it->value.GetString();
				OUT << TABx2 << "value ==> " << bufferView.mName << "\n";
			}
		}

		mBufferViewsMap[source] = bufferView;
		++source;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFBufferView> gLTFImportBufferViewsParser::getParsedBufferViews (void) const
{
	return mBufferViewsMap;
}
