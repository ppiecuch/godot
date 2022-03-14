# PolyVector
PolyVector is a proof-of-concept for importing SWF animations and triangulating them into polygons rendered with the GPU. It is implemented as a node module for Godot Game Engine 3.0.

Shape and path data is stored in memory using a Curve2D object, which are tessellated and triangulated as necessary when the quality level is changed. Animation works, but requires manual setup at the moment. Automatic LOD and other useful features are currently planned.

[A short feature demonstration is available on YouTube.](https://www.youtube.com/watch?v=9ozzZk03H44)


### Reference
 * http://www.doc.ic.ac.uk/lab/labman/swwf/SWFalexref.html#tag_showframe
 * https://stackoverflow.com/questions/26891599/qt-xml-viewer-like-notepad
 * https://github.com/lbellonda/qxmledit
 * http://3gfp.com/wp/2014/07/three-ways-to-parse-xml-in-qt/
 * https://github.com/pdw90/cppcheck/gui/projectfile.cpp
 * https://github.com/jsadelman/easyNetGUI/easyNet-app/xmlmodel.cpp
 * https://github.com/rdnvndr/treexmlmodel/src/treexmlmodel.h

 * http://www.herongyang.com/Flash/index.html
 * http://www.herongyang.com/Flash/SWF-File-Structure-and-Tag.html
 * http://www.herongyang.com/Flash/SWF-Process-Flow-SHOWFRAME-Synchronization.html

 * https://www.m2osw.com/mo_references_view/sswf_docs/SWFalexref

 * https://github.com/claus/as3swf/wiki/Shape-Export-to-Objective-C
 * https://blogs.msdn.microsoft.com/mswanson/2006/02/27/converting-flash-shapes-to-wpf/

 * https://github.com/jindrapetrik/jpexs-decompiler/blob/bf2a413725c09eecded4e8f42af4487ecd1842a5/libsrc/ffdec_lib/src/com/jpexs/decompiler/flash/importers/SwfXmlImporter.java
 * https://github.com/jindrapetrik/jpexs-decompiler/blob/bf2a413725c09eecded4e8f42af4487ecd1842a5/libsrc/ffdec_lib/src/com/jpexs/decompiler/flash/types/SHAPEWITHSTYLE.java
