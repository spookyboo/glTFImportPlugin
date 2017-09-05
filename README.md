# Json Export Plugin

HLMS Editor plugins are used to import and/or export Hlms materials.  
The __Json Export Plugin__ is a plugin for the HLMS Editor and exports all materials in the material browser (all material per project) to two json files (one for pbs and one for unlit).
This __Json Export Plugin__ Github repository contains Visual Studio JsonExport.sln / JsonExport.vcxproj files for convenience (do not forget to change the properties to the correct include files, 
because it makes use of both HLMS Editor and Ogre3d include files).
The __Json Export Plugin__ makes use of the generic plugin mechanism of Ogre3D.

**Plugin creation:**  
You can use the __Json Export Plugin__ code to create your own plugins and import/export materials from/to different sources. It works as a generic Ogre3d plugin, which means that
you have to create a [_yourname_]Plugin.h / [_yourname_]Plugin.cpp to implement custom import/export code and a [_yourname_]Dll.cpp (Windows) that acts as an entrypoint.
Your custom plugin extends from HlmsEditorPlugin (defined in hlms_editor_plugin.h), which means that it not only has to implement the Ogre3d plugin methods, 
such as getName(), install(), initialise(), shutdown() and uninstall(), but also some functions that are needed to communicate with the HLMS Editor.
The HLMS Editor recognizes the plugin by its name. Function getName() must always return the value "HlmsEditorPlugin".  
  
The HLMS Editor and the plugin communicate by means of the HlmsEditorPluginData object, which is defined in hlms_editor_plugin.h

**Installation:**  
Just add the plugin to the plugins.cfg file (under HLMSEditor/bin) and that's all (eg. Plugin=JsonExport); the HLMS Editor recognizes it if you have followed the rules above.