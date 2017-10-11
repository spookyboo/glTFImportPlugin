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
#ifndef __gLTFImportPlugin_H__
#define __gLTFImportPlugin_H__

#include "gLTFImportPrerequisites.h"
#include "OgrePlugin.h"
#include "hlms_editor_plugin.h"

namespace Ogre
{
    /** Plugin instance for gLTFImport Plugin */
    class gLTFImportPlugin : public Plugin, public HlmsEditorPlugin
    {
		public:
			gLTFImportPlugin();
		
			/// @copydoc Plugin::getName
			const String& getName() const;

			/// @copydoc Plugin::install
			void install();

			/// @copydoc Plugin::initialise
			void initialise();

			/// @copydoc Plugin::shutdown
			void shutdown();

			/// @copydoc Plugin::uninstall
			void uninstall();

			// Implement HlmsEditorPlugin functions
			virtual bool isImport (void) const;
			virtual bool isExport (void) const;
			virtual const String& getImportMenuText (void) const;
			virtual const String& getExportMenuText (void) const;
			virtual bool executeImport(HlmsEditorPluginData* data);
			virtual bool executeExport(HlmsEditorPluginData* data);
			virtual void performPreImportActions(void);
			virtual void performPostImportActions(void);
			virtual void performPreExportActions(void);
			virtual void performPostExportActions(void);
			virtual unsigned int getActionFlag(void);
			virtual std::map<std::string, HlmsEditorPluginData::PLUGIN_PROPERTY> getProperties (void);

		protected:
			std::map<std::string, HlmsEditorPluginData::PLUGIN_PROPERTY> mProperties;
    };
}

#endif
