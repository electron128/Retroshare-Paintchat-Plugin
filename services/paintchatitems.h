#ifndef PAINTCHATITEMS_H
#define PAINTCHATITEMS_H

#include "serialiser/rsserviceids.h"
#include "serialiser/rsserial.h"
#include "serialiser/rstlvtypes.h"

// --------TODO------------
const uint16_t RS_SERVICE_TYPE_PAINTCHAT_PLUGIN=RS_SERVICE_TYPE_PLUGIN_ARADO_TEST_ID1;


class RsPaintChatItem: public RsItem{
public:
    RsPaintChatItem():RsItem(RS_PKT_VERSION_SERVICE, RS_SERVICE_TYPE_PAINTCHAT_PLUGIN, 0),binData(0){
        setPriorityLevel(QOS_PRIORITY_DEFAULT);
    }
    virtual ~RsPaintChatItem(){}
    virtual void clear(){}

    virtual std::ostream& print(std::ostream &out, uint16_t indent = 0);

    uint8_t command;
    RsTlvBinaryData binData;
};

class RsPaintChatSerialiser: public RsSerialType
{
    public:
        RsPaintChatSerialiser()
            :RsSerialType(RS_PKT_VERSION_SERVICE, RS_SERVICE_TYPE_PAINTCHAT_PLUGIN)
        {
        }
        virtual ~RsPaintChatSerialiser() {}

        virtual uint32_t 	size (RsItem *item);
        virtual	bool serialise  (RsItem *item, void *data, uint32_t *pktsize);
        virtual	RsItem *deserialise(void *data, uint32_t *pktsize);
};

#endif // PAINTCHATITEMS_H
