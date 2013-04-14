#ifndef PAINTCHATSERVICE_H
#define PAINTCHATSERVICE_H

#include "services/sync.h"
#include "services/imageresource.h"

class PaintChatService;
extern PaintChatService *paintChatService;

class PaintChatService{
public:
    virtual void init(std::string id, ImageResource res)=0;
    // tell remote end to init
    virtual void sendInit(std::string id, ImageResource res)=0;
    virtual bool haveUpdate(std::string id)=0;
    virtual bool receivedInit(std::string id)=0;
    virtual ImageResource update(std::string id, ImageResource res)=0;
};

#endif // PAINTCHATSERVICE_H
