#ifndef SYNC_H
#define SYNC_H

#include <stdint.h>
#include <string>
#include <list>
#include <ctime>
#include <iostream>

// nur für shiftVoidPointer
// todo: lösugn finden
#include "imageresource.h"

// SyncEngine uses this interface to send Data
// incoming Data can be passed to SyncEngine
class Connection{
public:
    // receiver has to copy data
    virtual void sendData(void *data, const uint32_t &size)=0;
};
/*

  Resource and Diff classes should have an interface like this:

class Resource{
    Resource();

    // create diff, so that
    // Diff d=res.diff(newRes);
    // res.patch(d);
    // will lead to res==newRes;
    Diff diff(Resource modifiedResource, bool &resourceIsModified);
    void patch(Diff d);

    // serialisation methods
    uint32_t serial_size();
    bool serialise(void *data, uint32_t size);
    bool deserialise(void *data, uint32_t size);
};

class Diff{
    Diff();

    // diff only needs serialisation methods
    uint32_t serial_size();
    bool serialise(void *data, uint32_t size);
    bool deserialise(void *data, uint32_t size);
};
*/



// todo: checksummendings machen, items mit gleichem timestamp händeln

template <class Resource_T, class Diff_T>
class SyncEngine{
public:
    SyncEngine(Connection *c):connection(c),updated(false){

    }
    // später:
    // virtual void tick();

    // call at first to init with resource
    void init(Resource_T res){
        currentRessourceState=res;
        history.clear();
        history.push_back(SyncItem(res));
        updated=true;
    }

    virtual bool haveUpdate(){
        return updated;
    }

    // call to get/send updates
    virtual Resource_T update(const Resource_T &res){
        std::cerr<<"SyncEngine::update()"<<std::endl;
        updated=false;

        bool resourceIsModified;
        Diff_T diff=currentRessourceState.diff(res,resourceIsModified);
        if(resourceIsModified){
            // send diff
            SyncItem item;
            item.type=SyncItem::TYPE_DIFF;
            item.timestamp=time(0);
            item.diff=diff;

            void *data;
            uint32_t size;
            size=item.serial_size();
            data=malloc(size);
            item.serialise(data,size);
            connection->sendData(data,size);
            processData(data,size,false);
            free(data);
        }

        calcCurrentRessourceState();
        mergeHistory();
        return currentRessourceState;
    }

    // call on incoming data
    virtual void processData(void *data, const uint32_t &pktsize, bool external=true){
        std::cerr<<"SyncEngine::processData()"<<std::endl;
        updated=external;

        SyncItem item(data,pktsize);

        if(item.type==0){
            std::cerr<<"SyncEngine::processData(): deserialisation failed"<<std::endl;
            return;
        }

        //first item in history should be resource
        /*
        if((history.size()>0)&&(item.type==SyncItem::TYPE_RES)&&(history.front.type==SyncItem::TYPE_DIFF)){
            item.timestamp=0;
        }*/

        history.push_back(item);
        history.sort(SyncItem::compare);

        mergeHistory();
    }

private:
    class SyncItem;

    bool updated;

    Connection *connection;

    Resource_T currentRessourceState;
    std::list<SyncItem> history;

    void mergeHistory(){
        std::cerr<<"SyncEngine::mergeHistory(): history.size()="<<history.size()<<std::endl;
        SyncItem front=history.front();
        history.pop_front();
        while(history.size()>10){
            std::cerr<<"SyncEngine::mergeHistory(): merging one element"<<std::endl;
            SyncItem item=history.front();
            front.resource.patch(history.front().diff);
            history.pop_front();
        }
        history.push_front(front);
    }

    bool calcCurrentRessourceState(){
        if(history.size()==0){
            return false;
        }
        if(history.front().type==SyncItem::TYPE_DIFF){
            std::cerr<<"SyncEngine::calcCurrentRessourceState(): first Item is not of type Resource"<<std::endl;
            return false;
        }
        currentRessourceState=history.front().resource;
        // der erste item ist resource, und wird daher übersprungen
        typename std::list<SyncItem>::iterator it;
        for(it=history.begin(); it!=history.end(); it++){
            if(it->type==SyncItem::TYPE_DIFF){
                currentRessourceState.patch((it->diff));
            }
        }
    }



    class SyncItem{
    public:
        static const uint8_t TYPE_DIFF=1;
        static const uint8_t TYPE_RES=2;
        uint8_t type;
        uint32_t timestamp;
        Diff_T diff;
        Resource_T resource;

        SyncItem():
            type(0),
            timestamp(0){}

        SyncItem(Resource_T res):
            type(TYPE_RES),
            resource(res),
            timestamp(0){}

        SyncItem(void *data, const uint32_t &size){
            type=*((uint8_t*)data);
            data=shiftVoidPointer(data,1);
            timestamp=((*((uint8_t*)data+0))<<0)|
                      ((*((uint8_t*)data+1))<<8)|
                      ((*((uint8_t*)data+2))<<16)|
                      ((*((uint8_t*)data+3))<<24);
            data=shiftVoidPointer(data,4);
            switch(type){
            case TYPE_DIFF:
                if(!diff.deserialise(data,size)){
                    type=0;
                }
                break;
            case TYPE_RES:
                if(!resource.deserialise(data,size)){
                    type=0;
                }
                break;
            default:
                type=0;
                std::cerr<<"Error in SyncEngine::SyncItem::SyncItem(): no type"<<std::endl;
            }
        }

        uint32_t serial_size(){
            uint32_t size=0;
            switch(type){
            case TYPE_DIFF:
                size+=diff.serial_size();
                size+=5;
                break;
            case TYPE_RES:
                size+=resource.serial_size();
                size+=5;
                break;
            default:
                size=0;
                std::cerr<<"Error in SyncEngine::SyncItem::serial_size(): no type"<<std::endl;
            }
            return size;
        }

        void serialise(void *data, const uint32_t &size){
            *((uint8_t*)data)=type;
            data=shiftVoidPointer(data,1);
            *((uint8_t*)(data+0))=(uint8_t)(timestamp>>0);
            *((uint8_t*)(data+1))=(uint8_t)(timestamp>>8);
            *((uint8_t*)(data+2))=(uint8_t)(timestamp>>16);
            *((uint8_t*)(data+3))=(uint8_t)(timestamp>>24);
            data=shiftVoidPointer(data,4);
            switch(type){
            case TYPE_DIFF:
                diff.serialise(data,size);
                break;
            case TYPE_RES:
                resource.serialise(data,size);
                break;
            default:
                std::cerr<<"Error in SyncEngine::SyncItem::serialize(): no type"<<std::endl;
            }
        }

        static bool compare(const SyncItem &first, const SyncItem &second){
            if(first.timestamp==second.timestamp){
                // todo: compare checksum
                return true;
            }else{
                return (first.timestamp<second.timestamp);
            }
        }
    };
};


#endif // SYNC_H
