/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __gLTFImportNodessParser_H__
#define __gLTFImportNodessParser_H__

#include <map>
#include "gLTFImportConstants.h"
#include "gLTFNode.h"
#include "rapidjson/document.h"

/** Class responsible for executing the import and transformation of gLTF Nodes */
class gLTFImportNodesParser
{
	public:
		gLTFImportNodesParser (void) {};
		virtual ~gLTFImportNodesParser (void) {};

		// Parse the gLTF Nodes (level 1)
		bool parseNodes(rapidjson::Value::ConstMemberIterator jsonIterator);

		// Returns the Nodes structure
		const std::map<int, gLTFNode> getParsedNodes(void) const;

	protected:

	private:
		std::map<int, gLTFNode> mNodesMap;
};

#endif
