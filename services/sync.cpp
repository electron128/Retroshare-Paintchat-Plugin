#include "sync.h"

// alter code

SyncEngineImpl::SyncEngineImpl(Connection *c, Diff *d, Resource *r):connection(c),diff(d),resource(r){

}

void SyncEngineImpl::tick(){

}

Resource SyncEngineImpl::update(const Resource &res){
    Diff *diff;
    res.diff(currentRessourceState,res,diff);
    SyncItem item;
    item.type=SyncItem::TYPE_DIFF;
    item.timestamp=time();
    item.diff=diff;

    void *buf;
    uint32_t size;
    item.serial_size(size);
    buf=malloc(size);
    item.serialize(data,size);
    connection->sendData(data,size);
    processData(data,size);
    free(buf);

    calcCurrentRessourceState();
    return currentRessourceState;
}

void SyncEngineImpl::processData(void *data, const uint32_t &size){
    SyncItem item(data,size);

    //first item in history should be resource
    if((history.size()>0)&&(item.type==SyncItem::TYPE_RES)&&(history.front.type==SyncItem::TYPE_DIFF)){
        item.timestamp=0;
    }

    history.push_back(item);
    history.sort(SyncItem.compare);
}

bool SyncEngineImpl::calcCurrentRessourceState(){
    if(history.size()==0){
        return false;
    }
    if(history.front.type==SyncItem::TYPE_DIFF){
        return false;
    }
    currentRessourceState=history.front();
    for(std::list<SyncItem>::iterator it=(history.begin())+1;it!=history.end();it++){
        if(it->first.type==SyncItem::TYPE_DIFF){
            currentRessourceState.patch(*(it->first.diff));
        }
    }
}

void SyncEngineImpl::SyncItem::SyncItem(void *data, const &size){
    type=*((uint8_t*)data);
    data++;
    timestamp=((*((uint8_t*)data+0))<<0)|
              ((*((uint8_t*)data+1))<<8)|
              ((*((uint8_t*)data+2))<<16)|
              ((*((uint8_t*)data+3))<<24);
    data+=4;
    switch(type){
    case TYPE_DIFF:
        SyncEngineImpl::diff->deserialise(data,size,diff);
        break;
    case TYPE_RES:
        SyncEngineImpl::resource->deserialise(data,size,resource);
        break;
    default:
        std::cerr<<"Error in SyncEngineImpl::SyncItem::SyncItem(): no type";
    }
}

void SyncEngineImpl::SyncItem::serial_size(uint32_t &size){
    switch(type){
    case TYPE_DIFF:
        diff->serial_size(size);
        size+=5;
        break;
    case TYPE_RES:
        resource->serial_size(size);
        size+=5;
        break;
    default:
        size=0;
        std::cerr<<"Error in SyncEngineImpl::SyncItem::serial_size(): no type";
    }
}

void SyncEngineImpl::SyncItem::serialize(void *data, const uint32_t &size){
    *((uint8_t*)data)=type;
    data++;
    *((uint8_t*)(data+0))=(uint8_t)(timestamp>>0);
    *((uint8_t*)(data+1))=(uint8_t)(timestamp>>8);
    *((uint8_t*)(data+2))=(uint8_t)(timestamp>>16);
    *((uint8_t*)(data+3))=(uint8_t)(timestamp>>24);
    data+=4;
    switch(type){
    case TYPE_DIFF:
        diff->serialize(data,size);
        break;
    case TYPE_RES:
        resource->serialize(data,size);
        break;
    default:
        std::cerr<<"Error in SyncEngineImpl::SyncItem::serialize(): no type";
    }
}

void SyncEngineImpl::SyncItem::deserialise(void *data, const uint32_t &size, t *&obj){

}

static bool SyncEngineImpl::SyncItem::compare(const SyncItem &first, const SyncItem &second){
    if(first.timestamp==second.timestamp){
        // todo: compare checksum
        return true;
    }else{
        return (first.timestamp<second.timestamp);
    }
}
