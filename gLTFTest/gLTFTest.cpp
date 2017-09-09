// gLTFTest.cpp : Defines the entry point for the console application.
//

#include "hlms_editor_plugin.h"
#include "gLTFImportExecutor.h"
#include <iostream>

int main()
{
	Ogre::HlmsEditorPluginData pluginData;
	pluginData.mInProjectName = "project1";
	pluginData.mInProjectPath = "../project/"; // With end slash
	pluginData.mInMaterialFileName = "";
	pluginData.mInTextureFileName = "";
	pluginData.mInFileDialogName = "BoomBox.gltf"; // Json text file
	//pluginData.mInFileDialogName = "BoomBox.glb"; // Binary file
	pluginData.mInFileDialogBaseName = "BoomBox";
	pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/BoomBox/glTF/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/BoomBox/glTF-Binary/"; // With end slash
	pluginData.mInImportPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/"; // With end slash
	pluginData.mInExportPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/BoomBox/glTF/"; // Not used
	pluginData.mInItem = 0;
	pluginData.mInRenderWindow = 0;
	pluginData.mInSceneManager = 0;
	pluginData.mInOutCurrentDatablock = 0;
	gLTFImportExecutor executor;
	
	if (executor.executeImport(&pluginData))
		std::cout << pluginData.mOutSuccessText << "\n";
	else
		std::cout << pluginData.mOutErrorText << "\n";
    
	return 0;
}
