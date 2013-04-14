#include "paintchatitems.h"

#include <iostream>

std::ostream& RsPaintChatItem::print(std::ostream &out, uint16_t indent){
    printRsItemBase(out, "RsPaintChatItem", indent);
    uint16_t int_Indent = indent + 2;
    printIndent(out, int_Indent);
    out << "Nothing here: " << 0 << std::endl;

    printRsItemEnd(out, "RsPaintChatItem", indent);
    return out;
}


uint32_t RsPaintChatSerialiser::size(RsItem *item){
    RsPaintChatItem *paintChatItem= dynamic_cast<RsPaintChatItem *>(item);
    uint32_t size=0;
    // header
    size+=8;

    // command
    size+=1;

    size+=paintChatItem->binData.TlvSize();

    std::cerr<<"RsPaintChatSerialiser::size():"<<size<<std::endl;

    return size;
}

bool RsPaintChatSerialiser::serialise(RsItem *item, void *data, uint32_t *pktsize){
    uint32_t tlvsize = size(item) ;
    uint32_t offset = 0;

    if (*pktsize < tlvsize)
        return false; /* not enough space */

    *pktsize = tlvsize;

    bool ok = true;

    ok &= setRsItemHeader(data, tlvsize, PacketId(), tlvsize);

    std::cerr << "RsPaintChatSerialiser::serialise() Header: " << ok << std::endl;
    std::cerr << "RsPaintChatSerialiser::serialise() Size: " << tlvsize << std::endl;

    /* skip the header */
    offset += 8;

    RsPaintChatItem *paintChatItem=dynamic_cast<RsPaintChatItem *>(item);

    *(((uint8_t*)data)+offset)=paintChatItem->command;
    offset+=1;

    ok &= paintChatItem->binData.SetTlv(data,*pktsize,&offset);

    if (offset != tlvsize)
    {
        ok = false;
        std::cerr << "RsPaintChatSerialiser::serialise()) Size Error! " << std::endl;
    }

    return ok;
}

RsItem *RsPaintChatSerialiser::deserialise(void *data, uint32_t *pktsize){
    std::cerr << "RsPaintChatSerialiser::deserialise()" << std::endl;

    /* get the type and size */
    uint32_t rstype = getRsItemId(data);
    uint32_t rssize = getRsItemSize(data);

    if ((RS_PKT_VERSION_SERVICE != getRsItemVersion(rstype)) || (RS_SERVICE_TYPE_PAINTCHAT_PLUGIN != getRsItemService(rstype)))
        return NULL ;

    uint32_t offset = 0;

    if (*pktsize < rssize){    /* check size */
        //throw std::runtime_error("Not enough space") ;
        std::cerr << "RsPaintChatSerialiser::deserialise() Not enough space! " << std::endl;
        return NULL;
    }

    bool ok = true;

    /* skip the header */
    offset += 8;

    RsPaintChatItem *paintChatItem=new RsPaintChatItem();

    paintChatItem->command=*(((uint8_t*)data)+offset);
    offset +=1;

    ok &= paintChatItem->binData.GetTlv(data,*pktsize,&offset);

    if (offset != rssize){
        //throw std::runtime_error("Serialization error.") ;
        std::cerr << "RsPaintChatSerialiser::deserialise() Serialization error. " << std::endl;
        delete paintChatItem;
        return NULL;
    }

    if (!ok){
        //throw std::runtime_error("Serialization error.") ;
        std::cerr << "RsPaintChatSerialiser::deserialise() Serialization error: ok=false " << std::endl;
        delete paintChatItem;
        return NULL;
    }

    return paintChatItem;
}
