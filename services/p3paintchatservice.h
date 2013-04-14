#ifndef P3PAINTCHATSERVICE_H
#define P3PAINTCHATSERVICE_H

#include "plugins/rspqiservice.h"
#include "interface/paintchatservice.h"

#include <map>

class p3PaintChatService : public RsPQIService, public PaintChatService
{
public:
    p3PaintChatService(RsPluginHandler *handler);
    ~p3PaintChatService();

    virtual int tick();

    virtual void init(std::string id, ImageResource res);
    // tell remote end to init
    virtual void sendInit(std::string id, ImageResource res);
    virtual bool haveUpdate(std::string id);
    virtual bool receivedInit(std::string id);
    virtual ImageResource update(std::string id, ImageResource res);

private:

    SyncEngine<ImageResource,ImageDiff>* addPeer(std::string peerId);

    static const uint8_t COMMAND_INIT=0;
    static const uint8_t COMMAND_SYNC=1;

    class PaintChatConnection: public Connection{
    public:
        PaintChatConnection(std::string connId, p3PaintChatService *service );
        void sendData(void *data, const uint32_t &size);
        std::string connId;
        p3PaintChatService *service;
    };

    std::map<std::string,Connection*>connections;
    std::map<std::string,SyncEngine<ImageResource,ImageDiff>*> syncEngines;
    std::list<std::string> receivedInits;
};

#endif // P3PAINTCHATSERVICE_H
