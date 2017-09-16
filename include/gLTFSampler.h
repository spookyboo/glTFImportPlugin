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
#ifndef __gLTFSampler_H__
#define __gLTFSampler_H__

#include <string>
#include <iostream>

/************************************************************************************************
This file contains the data struture of a gLFTSampler. This is an intermediate structure
used to generate an Ogre 3d material file.
/************************************************************************************************/

/************************************************/
/** Class responsible that represents a Sampler */
/************************************************/
class gLTFSampler
{
	public:
		gLTFSampler(void);
		virtual ~gLTFSampler(void) {};

		enum MAGFILTER
		{
			MAG_NEAREST = 9728,
			MAG_LINEAR = 9729
		};

		enum MINFILTER
		{
			MIN_NEAREST = 9728,
			MIN_LINEAR = 9729,
			MIN_NEAREST_MIPMAP_NEAREST = 9984,
			MIN_LINEAR_MIPMAP_NEAREST = 9985,
			MIN_NEAREST_MIPMAP_LINEAR = 9986,
			MIN_LINEAR_MIPMAP_LINEAR = 9987
		};

		enum S_WRAPPING_MODE
		{
			SW_CLAMP_TO_EDGE = 33071,
			SW_MIRRORED_REPEAT = 33648,
			SW_REPEAT = 10497
		};

		enum T_WRAPPING_MODE
		{
			TW_CLAMP_TO_EDGE = 33071,
			TW_MIRRORED_REPEAT = 33648,
			TW_REPEAT = 10497
		};

		void out (void); // prints the content of the gLTFSampler

		// Public members
		int mMagFilter;
		int mMinFilter;
		int mWrapS;
		int mWrapT;
		std::string mName;
};

#endif
