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

#include "gLTFImportImagesParser.h"

//---------------------------------------------------------------------
bool gLTFImportImagesParser::parseImages (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TAB << "Perform gLTFImportImagesParser::parseImages\n";

	int source = 0;
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TAB << "Loop through images array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		OUT << TAB << "Source index " << source << "\n";
		gLTFImage image;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();
		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			OUT << TABx2 << "key image ==> " << it->name.GetString() << "\n";
			std::string key = std::string(it->name.GetString());
			if (it->value.IsString() && key == "uri")
			{
				// ******** 1. uri ********
				image.mUri = it->value.GetString();
				OUT << TABx2 << "value ==> " << image.mUri << "\n";
			}
			if (it->value.IsString() && key == "mimeType")
			{
				// ******** 2. mimeType ********
				image.mMimeType = it->value.GetString();
				OUT << TABx2 << "value ==> " << image.mMimeType << "\n";
			}
			if (it->value.IsInt() && key == "bufferView")
			{
				// ******** 3. bufferView ********
				image.mBufferView = it->value.GetInt();
				OUT << TABx2 << "value ==> " << image.mBufferView << "\n";
			}
		}
		
		mImagesMap[source] = image;
		++source;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFImage> gLTFImportImagesParser::getParsedImages (void) const
{
	std::map<int, gLTFImage>::const_iterator itImages;
	for (itImages = mImagesMap.begin(); itImages != mImagesMap.end(); itImages++)
		OUT << "Image.bufferView: " << (itImages->second).mBufferView << "\n";

	return mImagesMap;
}



