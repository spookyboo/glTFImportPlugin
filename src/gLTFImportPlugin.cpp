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

#include "OgreRoot.h"
#include "gLTFImportPlugin.h"
#include "gLTFImportExecutor.h"
#include <iostream>
#include <fstream>

namespace Ogre
{
	static const String gExportMenuText = "";
	static const String gImportMenuText = "Import gLTF";
	//---------------------------------------------------------------------
	gLTFImportPlugin::gLTFImportPlugin()
    {
		mProperties.clear();
    }
    //---------------------------------------------------------------------
    const String& gLTFImportPlugin::getName() const
    {
        return GENERAL_HLMS_PLUGIN_NAME;
    }
    //---------------------------------------------------------------------
    void gLTFImportPlugin::install()
    {
    }
    //---------------------------------------------------------------------
    void gLTFImportPlugin::initialise()
    {
        // nothing to do
	}
    //---------------------------------------------------------------------
    void gLTFImportPlugin::shutdown()
    {
        // nothing to do
    }
    //---------------------------------------------------------------------
    void gLTFImportPlugin::uninstall()
    {
    }
	//---------------------------------------------------------------------
	bool gLTFImportPlugin::isImport (void) const
	{
		return true;
	}
	//---------------------------------------------------------------------
	bool gLTFImportPlugin::isExport (void) const
	{
		return false;
	}
	//---------------------------------------------------------------------
	void gLTFImportPlugin::performPreImportActions(void)
	{
		// Nothing to do
	}
	//---------------------------------------------------------------------
	void gLTFImportPlugin::performPostImportActions(void)
	{
		// Nothing to do
	}
	//---------------------------------------------------------------------
	void gLTFImportPlugin::performPreExportActions(void)
	{
		// Nothing to do
	}
	//---------------------------------------------------------------------
	void gLTFImportPlugin::performPostExportActions(void)
	{
		// Nothing to do
	}
	//---------------------------------------------------------------------
	unsigned int gLTFImportPlugin::getActionFlag(void)
	{
		// 1. Open a dialog to read the gLTF file
		// 2. Create a directory in the import directory with the base name of the selected file
		// 3. Open a properties dialog to pass additional property data to the plugin
		// 4. Open a project after import; mOutReferencesMap contains the reference
		// 5. Open a mesh after import; mOutReferencesMap contains the reference
		return	PAF_PRE_IMPORT_OPEN_FILE_DIALOG |
			PAF_PRE_IMPORT_MK_DIR |
			PAF_PRE_ACTION_SETTINGS_DIALOG |
			PAF_POST_IMPORT_OPEN_PROJECT_MAPREF |
			PAF_POST_IMPORT_LOAD_MESH_MAPREF;
	}

	//---------------------------------------------------------------------
	std::map<std::string, HlmsEditorPluginData::PLUGIN_PROPERTY> gLTFImportPlugin::getProperties(void)
	{
		// Set the default property values

		// Combine meshes to one mesh
		HlmsEditorPluginData::PLUGIN_PROPERTY property;
		property.propertyName = "mesh_is_submesh";
		property.labelName = "Combine all meshes into one mesh";
		property.info = "gLTF distinguishes meshes and primitives. Meshes are transformed to Ogre meshes, while primitives are transformed into OgresubMeshes\n.\
						 If this property is set, each gLTF mesh is transformed to a submesh. This results in one Ogre mesh per import.";
		property.type = HlmsEditorPluginData::BOOL;
		property.boolValue = true;
		mProperties[property.propertyName] = property;

		// Generate tangents
		property.propertyName = "generate_tangents";
		property.labelName = "Generate tangents";
		property.info = "";
		property.type = HlmsEditorPluginData::BOOL;
		property.boolValue = false;
		mProperties[property.propertyName] = property;

		// Generate edge lists
		property.propertyName = "generate_edge_lists";
		property.labelName = "Generate edge lists";
		property.info = "This is needed for stencil shadows";
		property.type = HlmsEditorPluginData::BOOL;
		property.boolValue = false;
		mProperties[property.propertyName] = property;

		// Optimize for desktop
		property.propertyName = "optimize_for_desktop";
		property.labelName = "Optimize for desktop";
		property.info = "";
		property.type = HlmsEditorPluginData::BOOL;
		property.boolValue = true;
		mProperties[property.propertyName] = property;

		// Correct for min/max
		// Accessors have an optional min/max array, but for some models applying this results in misformed vertices
		property.propertyName = "correct_min_max";
		property.labelName = "Correct for min/max";
		property.info = "gLTF accessors may have min/max values defined, but some models become misformed when applied";
		property.type = HlmsEditorPluginData::BOOL;
		property.boolValue = false;
		mProperties[property.propertyName] = property;

		return mProperties;
	}

	//---------------------------------------------------------------------
	const String& gLTFImportPlugin::getImportMenuText (void) const
	{
		return gImportMenuText;
	}
	//---------------------------------------------------------------------
	const String& gLTFImportPlugin::getExportMenuText (void) const
	{
		return gExportMenuText;
	}
	//---------------------------------------------------------------------
	bool gLTFImportPlugin::executeImport (HlmsEditorPluginData* data)
	{
		gLTFImportExecutor executor;
		return executor.executeImport(data);
	}
	//---------------------------------------------------------------------
	bool gLTFImportPlugin::executeExport (HlmsEditorPluginData* data)
	{
		// nothing to do
		return true;
	}

}
