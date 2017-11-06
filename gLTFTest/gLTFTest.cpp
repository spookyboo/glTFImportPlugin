// gLTFTest.cpp : Defines the entry point for the console application.
//

#include "OgreRoot.h"
#include "OgreNULLRenderSystem.h"
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

	// Set the 'mesh_is_submesh' property
	Ogre::HlmsEditorPluginData::PLUGIN_PROPERTY property;
	property.propertyName = "mesh_is_submesh";
	property.labelName = "Combine all meshes into one mesh";
	property.info = "gLTF distinguishes meshes and primitives. Meshes are transformed to Ogre meshes, while primitives are transformed into OgresubMeshes\n.\
						 If this property is set, each gLTF mesh is transformed to a submesh. This results in one Ogre mesh per import.";
	property.type = Ogre::HlmsEditorPluginData::BOOL;
	property.boolValue = true; // Combine meshes to one mesh or make individual mesh files
	pluginData.mInPropertiesMap[property.propertyName] = property;
	
	property.propertyName = "optimize_for_desktop";
	property.labelName = "Optimized for Desktop";
	property.info = "";
	property.type = Ogre::HlmsEditorPluginData::BOOL;
	property.boolValue = true; // Combine meshes to one mesh or make individual mesh files
	pluginData.mInPropertiesMap[property.propertyName] = property;

	property.propertyName = "generate_animation";
	property.labelName = "Always generate animation";
	property.info = "";
	property.type = Ogre::HlmsEditorPluginData::BOOL;
	property.boolValue = true;
	pluginData.mInPropertiesMap[property.propertyName] = property;

	//pluginData.mInFileDialogName = "BoomBox.gltf"; // Json text file
	//pluginData.mInFileDialogName = "Suzanne.gltf"; // Json text file
	//pluginData.mInFileDialogName = "BoomBox.glb"; // Binary file
	//pluginData.mInFileDialogName = "adamHead.gltf";
	//pluginData.mInFileDialogName = "Buggy.gltf"; // Json text file
	//pluginData.mInFileDialogName = "Lantern.gltf"; // Json text file
	//pluginData.mInFileDialogName = "Box.gltf"; // Json text file
	//pluginData.mInFileDialogName = "TwoSidedPlane.gltf"; // Json text file
	//pluginData.mInFileDialogName = "Corset.glb"; // Binary file
	//pluginData.mInFileDialogName = "Monster.gltf"; // Embedded binary
	//pluginData.mInFileDialogName = "SmilingFace.gltf"; // Json text file
	//pluginData.mInFileDialogName = "scene.gltf"; // Json text file
	//pluginData.mInFileDialogName = "CesiumMilkTruck.gltf"; // Json text file
	pluginData.mInFileDialogName = "CesiumMan.gltf"; // Json text file
	//pluginData.mInFileDialogName = "AnimatedTriangle.gltf"; // Json text file
	//pluginData.mInFileDialogName = "RiggedSimple.gltf"; // Json text file
	

	//pluginData.mInFileDialogBaseName = "BoomBox";
	//pluginData.mInFileDialogBaseName = "Suzanne";
	//pluginData.mInFileDialogBaseName = "adamHead";
	//pluginData.mInFileDialogBaseName = "Buggy";
	//pluginData.mInFileDialogBaseName = "Lantern";
	//pluginData.mInFileDialogBaseName = "Box";
	//pluginData.mInFileDialogBaseName = "TwoSidedPlane";
	//pluginData.mInFileDialogBaseName = "Corset";
	//pluginData.mInFileDialogBaseName = "Monster";
	//pluginData.mInFileDialogBaseName = "SmilingFace";
	//pluginData.mInFileDialogBaseName = "scene";
	//pluginData.mInFileDialogBaseName = "CesiumMilkTruck";
	pluginData.mInFileDialogBaseName = "CesiumMan";
	//pluginData.mInFileDialogBaseName = "AnimatedTriangle";
	//pluginData.mInFileDialogBaseName = "RiggedSimple";

	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/BoomBox/glTF/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/Suzanne/glTF/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/BoomBox/glTF-Binary/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/AdamHead/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/Buggy/glTF/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/Lantern/glTF/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/Box/glTF/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/TwoSidedPlane/glTF/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/Corset/glTF-Binary/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/Monster/glTF-Embedded/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/SmilingFace/glTF/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/sexylady/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/CesiumMilkTruck/glTF/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/warehouse/";
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/cartoongirl/";
	pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/CesiumMan/glTF/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/AnimatedTriangle/glTF/"; // With end slash
	//pluginData.mInFileDialogPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/RiggedSimple/glTF/"; // With end slash
	
	pluginData.mInImportPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/"; // With end slash
	//pluginData.mInImportPath = "C:/Users/Henry/Downloads/AdamHead/import/"; // With end slash
	
	//pluginData.mInExportPath = "C:/Users/Henry/Downloads/glTF-Sample-Models-master/2.0/BoomBox/glTF/"; // Not used
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
