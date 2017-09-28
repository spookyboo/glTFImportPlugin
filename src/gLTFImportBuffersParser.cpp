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

#include "gLTFImportBuffersParser.h"

//---------------------------------------------------------------------
bool gLTFImportBuffersParser::parseBuffers (const std::string& fileName, rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportBuffersParser::parseBuffers\n";

	int source = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through Buffers array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		OUT << TAB << "Source index " << source << "\n";
		gLTFBuffer buffer;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();
		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key buffer ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsString() && key == "uri")
			{
				// ******** 1. uri ********
				buffer.mUri = it->value.GetString();
				OUT << TABx2 << "value ==> " << buffer.mUri << "\n";
			}
			if (it->value.IsInt() && key == "byteLength")
			{
				// ******** 2. byteLength ********
				buffer.mByteLength = it->value.GetInt();
				OUT << TABx2 << "value ==> " << buffer.mByteLength << "\n";
			}
			if (it->value.IsString() && key == "name")
			{
				// ******** 3. name ********
				buffer.mName = it->value.GetString();
				OUT << TABx2 << "value ==> " << buffer.mName << "\n";
			}
		}
		
		if (buffer.mUri == "")
		{
			/* The uri is not defined, so assign the file in which the json is defined. This may be the case when
			   both the json and the buffer are part of the same file (so, the glb file itself)
			*/
			buffer.mUri = fileName;
		}
		mBuffersMap[source] = buffer;
		++source;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFBuffer> gLTFImportBuffersParser::getParsedBuffers (void) const
{
	return mBuffersMap;
}
