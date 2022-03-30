#ifdef _MSC_VER
#pragma once
#endif

#ifndef _TLFX_PUGIXMLLOADER_H
#define _TLFX_PUGIXMLLOADER_H

#include "TLFXXMLLoader.h"
#include "misc/pugixml.h"

namespace TLFX
{

    struct AttributeNode;

    class PugiXMLLoader : public XMLLoader
    {
    public:
        PugiXMLLoader(int shapes, const char *libraryfile = 0) : XMLLoader(shapes), _library(libraryfile) {}
        PugiXMLLoader(const char *libraryfile) : XMLLoader(0), _library(libraryfile) {}

        virtual bool        Open(const char *filename);
        virtual bool        GetNextShape(AnimImage *shape);
        virtual Effect*     GetNextEffect(const std::list<AnimImage*>& sprites);
        virtual Effect*     GetNextSuperEffect(const std::list<AnimImage*>& sprites);
 
        virtual void        LocateEffect();
        virtual void        LocateSuperEffect();

        virtual const char* GetLastError() const;

    protected:
        const char *_library;

        char _error[128];
        pugi::xml_document _doc;
        pugi::xml_node _currentShape;
        pugi::xml_node _currentEffect;              // can be in root or in a folder
        pugi::xml_node _currentFolder;

        Effect*    LoadEffect       (pugi::xml_node& node, const std::list<AnimImage*>& sprites, Emitter *parent = NULL, const char *folderPath = "");
        Effect*    LoadSuperEffect  (pugi::xml_node& node, const std::list<AnimImage*>& sprites, Emitter *parent = NULL, const char *folderPath = "");
        void       LoadAttributeNode(pugi::xml_node& node, AttributeNode* attr);
        Emitter*   LoadEmitter      (pugi::xml_node& node, const std::list<AnimImage*>& sprites, Effect *parent);
        AnimImage* GetSpriteInList  (const std::list<AnimImage*>& sprites, int index) const;
    };

} // namespace TLFX

#endif // _TLFX_EFFECTSLIBRARY_H
