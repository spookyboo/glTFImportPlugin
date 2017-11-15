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

#include "gLTFImportConstants.h"
#include "gLTFNode.h"

//---------------------------------------------------------------------
gLTFNode::gLTFNode(void) :
	mCamera (-1),
	mSkin (-1), 
	mMesh (-1),
	mHasMatrix(false),
	mHasRotation(false),
	mHasScale(false),
	mHasTranslation(false),
	mTransformationCalculated(false),
	mWorldTransformationCalculated(false),
	mParentNode(0),
	mNodeIndex(-1)
{
	mChildren.clear();
	mWeights.clear();
	mName = "";
	for (int i = 0; i < 16; ++i)
		mMatrix[i] = 0.0f;

	mRotation[0] = 0.0f;
	mRotation[1] = 0.0f;
	mRotation[2] = 0.0f;
	mRotation[3] = 1.0f;

	mScale[0] = 1.0f;
	mScale[1] = 1.0f;
	mScale[2] = 1.0f;

	mTranslation[0] = 0.0f;
	mTranslation[1] = 0.0f;
	mTranslation[2] = 0.0f;
	
	mAnimationVector.clear();
}

//---------------------------------------------------------------------
void gLTFNode::out (void)
{
	OUT << "***************** Debug: gLTFNode *****************\n";
}

//---------------------------------------------------------------------
bool gLTFNode::hasAnimationName (const std::string& animationName)
{
	std::vector<gLTFAnimation>::iterator it;
	std::vector<gLTFAnimation>::iterator itEnd = mAnimationVector.end();
	for (it = mAnimationVector.begin(); it != itEnd; ++it)
	{
		if ((*it).mName == animationName)
			return true;
	}
	return false;
}
