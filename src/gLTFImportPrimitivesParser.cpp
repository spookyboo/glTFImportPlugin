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

#include "gLTFImportPrimitivesParser.h"

//---------------------------------------------------------------------
bool gLTFImportPrimitivesParser::parsePrimitives (rapidjson::Value::ConstMemberIterator jsonIterator)
{
	OUT << TABx3 << "Perform gLTFImportPrimitivesParser::parsePrimitives\n";

	int index = 0;
	mPrimitivesMap.clear();
	const rapidjson::Value& array = jsonIterator->value;

	OUT << TABx4 << "Loop through primitives array\n";
	for (rapidjson::SizeType i = 0; i < array.Size(); i++)
	{
		gLTFPrimitive primitive;
		rapidjson::Value::ConstMemberIterator it;
		rapidjson::Value::ConstMemberIterator itEnd = array[i].MemberEnd();

		for (it = array[i].MemberBegin(); it != itEnd; ++it)
		{
			std::string key = std::string(it->name.GetString());
			OUT << TABx4 << "key mesh ==> " << key << "\n";
			if (it->value.IsObject() && key == "attributes")
			{
				// ******** 1. attributes  ********
				OUT << TABx4 << "Loop through attributes\n";
				rapidjson::Value::ConstMemberIterator itAttr;
				rapidjson::Value::ConstMemberIterator itAttrEnd = it->value.MemberEnd();
				for (itAttr = it->value.MemberBegin(); itAttr != itAttrEnd; ++itAttr)
				{
					std::string keyAttr = std::string(itAttr->name.GetString());
					OUT << TABx4 << "key attributes ==> " << keyAttr << "\n";
					if (itAttr->value.IsInt() && keyAttr == "POSITION")
					{
						// ******** 1.1 POSITION ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
					if (itAttr->value.IsInt() && keyAttr == "NORMAL")
					{
						// ******** 1.2 NORMAL ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
					if (itAttr->value.IsInt() && keyAttr == "TANGENT")
					{
						// ******** 1.3 TANGENT ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
					if (itAttr->value.IsInt() && keyAttr == "TEXCOORD_0")
					{
						// ******** 1.4 TEXCOORD_0 ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
					if (itAttr->value.IsInt() && keyAttr == "TEXCOORD_1")
					{
						// ******** 1.5 TEXCOORD_1 ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
					if (itAttr->value.IsInt() && keyAttr == "COLOR_0")
					{
						// ******** 1.6 COLOR_0 ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
					if (itAttr->value.IsInt() && keyAttr == "JOINTS_0")
					{
						// ******** 1.7 JOINTS_0 ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
					if (itAttr->value.IsInt() && keyAttr == "WEIGHTS_0")
					{
						// ******** 1.8 WEIGHTS_0 ********
						primitive.mAttributes[keyAttr] = itAttr->value.GetInt();
						OUT << TABx4 << "value ==> " << primitive.mAttributes[keyAttr] << "\n";
					}
				}
			}
			if (it->value.IsInt() && key == "indices")
			{
				// ******** 2. indices ********
				primitive.mIndicesAccessor = it->value.GetInt();
				OUT << TABx4 << "value ==> " << primitive.mIndicesAccessor << "\n";
			}
			if (it->value.IsInt() && key == "material")
			{
				// ******** 3. material ********
				primitive.mMaterial = it->value.GetInt();
				OUT << TABx4 << "value ==> " << primitive.mMaterial << "\n";
			}
			if (it->value.IsInt() && key == "mode")
			{
				// ******** 4. mode ********
				primitive.mMode = it->value.GetInt();
				OUT << TABx4 << "value ==> " << primitive.mMode << "\n";
			}
			if (it->value.IsArray() && key == "targets")
			{
				// ******** 5. targets ********
				// TODO: Parse through 'targets' object
			}
		}
		
		mPrimitivesMap[index] = primitive;
		++index;
	}

	return true;
}

//---------------------------------------------------------------------
const std::map<int, gLTFPrimitive> gLTFImportPrimitivesParser::getParsedPrimitives (void) const
{
	return mPrimitivesMap;
}
