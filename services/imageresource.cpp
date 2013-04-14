#include "imageresource.h"

#include <QColor>
#include <iostream>

ImageResource::ImageResource():width(0),height(0),data(0),size(0){

}
ImageResource::ImageResource(const ImageResource &other){
    //std::cerr<<"ImageResource::ImageResource(const ImageResource &other)"<<std::endl;
    width=other.width;
    height=other.height;
    size=other.size;
    data=malloc(size);
    if(data){
        memcpy(data,other.data,size);
    }else{
        std::cerr<<"ImageResource::ImageResource(const ImageResource &other): Error: malloc failed"<<std::endl;
    }
}
ImageResource::~ImageResource(){
    if(data){
        free(data);
    }
    //std::cerr<<"ImageResource::~ImageResource()"<<std::endl;
}
ImageResource& ImageResource::operator =(ImageResource other){
    //std::cerr<<"ImageResource& ImageResource::operator =(ImageResource other)"<<std::endl;
    // other is already a copy, so just swap
    // other will destroy old data
    // copy-and-swap idiom
    std::swap(width,other.width);
    std::swap(height,other.height);
    std::swap(data,other.data);
    std::swap(size,other.size);
    return *this;
}

uint32_t ImageResource::serial_size(){
    return size+4+4+4;
}
bool ImageResource::serialise(void *pktdata, uint32_t pktsize){
    if(pktsize<serial_size()){
        return false;
    }
    serialise_uint32(shiftVoidPointer(pktdata,0),width);
    serialise_uint32(shiftVoidPointer(pktdata,4),height);
    serialise_uint32(shiftVoidPointer(pktdata,8),size);
    memcpy(shiftVoidPointer(pktdata,12),data,size);
    return true;
}
bool ImageResource::deserialise(void *pktdata, uint32_t pktsize){
    if(data){
        free(data);
    }
    width=deserialise_uint32(shiftVoidPointer(pktdata,0));
    height=deserialise_uint32(shiftVoidPointer(pktdata,4));
    size=deserialise_uint32(shiftVoidPointer(pktdata,8));
    data=malloc(size);
    memcpy(data,shiftVoidPointer(pktdata,12),size);
    return true;
}

ImageDiff ImageResource::diff(ImageResource modifiedResource, bool &resourceIsModified){
    if(!modifiedResource.data){
        std::cerr<<"ImageResource::diff(): Error: no data in modifiedResource to diff"<<std::endl;
    }
    if(!data){
        std::cerr<<"ImageResource::diff(): Error: no own data, copying modifiedResource"<<std::endl;
        *this = modifiedResource;
        if(*(uint8_t*)data==*(uint8_t*)modifiedResource.data){
            std::cerr<<"ImageResource::diff(): copy ok"<<std::endl;
        }
    }
    if((width!=modifiedResource.width)||(height!=modifiedResource.height)){
        std::cerr<<"ImageResource::diff(): Error: height or width not equal"<<std::endl;
    }

    std::cerr<<"ImageResource::diff()"<<std::endl;

    // width+height+size
    uint32_t headersize=(4+4+4);

    // (size/8)+1 bytes for diffbits
    uint32_t numberOfDiffBytes=(size>>3)+1;

    uint32_t diffSize=headersize+numberOfDiffBytes;
    std::cerr<<"ImageResource::diff(): headersize+numberOfDiffBytes="<<diffSize<<std::endl;
    resourceIsModified=false;
    for(uint32_t i=0;i<size;i++){
        if(((uint8_t*)data)[i]!=((uint8_t*)modifiedResource.data)[i]){
            diffSize++;
            resourceIsModified=true;
        }
    }

    if(!resourceIsModified){
        return ImageDiff();
    }

    std::cerr<<"ImageResource::diff(): malloc("<<diffSize<<")"<<std::endl;
    uint8_t *diffData=(uint8_t*)malloc(diffSize);
    if(diffData==NULL){
        std::cerr<<"ImageResource::diff(): Error: malloc failed"<<std::endl;
        return ImageDiff();
    }

    void *header=(void*)diffData;
    uint8_t *diffBits=diffData+headersize;
    uint8_t *diffBytes=diffBits+numberOfDiffBytes;

    serialise_uint32(shiftVoidPointer(header,0),width);
    serialise_uint32(shiftVoidPointer(header,4),height);
    serialise_uint32(shiftVoidPointer(header,8),size);

    uint8_t bitNo=0;
    uint8_t byte=0;

    //uint8_t *modifiedData=(uint8_t)modifiedResource.data;
    for(uint32_t i=0;i<size;i++){
        if(((uint8_t*)data)[i]!=((uint8_t*)modifiedResource.data)[i]){
            //std::cerr<<"ImageResource::patch(): saving diff for byte "<<i<<" at offset "<<diffBytes-diffData<<std::endl;
            byte|=(1<<bitNo);
            *diffBytes=((uint8_t*)modifiedResource.data)[i];
            diffBytes++;
        }
        //modifiedResource.data=shiftVoidPointer(modifiedResource.data,1); //------- käse, wegen [i]
        bitNo++;
        if(bitNo==8){
            bitNo=0;
            *diffBits=byte;
            diffBits++;
            byte=0;
        }
    }

    compress(diffData,diffSize);

    ImageDiff imageDiff;
    imageDiff.data=(void*)diffData;//-------------- vorher wars ohne "(void*)", compiler hat nicht gemeckert
    imageDiff.size=diffSize;

    return imageDiff;
}
void ImageResource::patch(ImageDiff d){
    if(!d.data){
        std::cerr<<"ImageResource::patch(): Error: diff has no data"<<std::endl;
        return;
    }
    if(!data){
        std::cerr<<"ImageResource::patch(): Error: no data"<<std::endl;
        return;
    }

    uint8_t *p=(uint8_t*)d.data;
    decompress(p,d.size);
    d.data=(void*)p;

    if(width!=deserialise_uint32(shiftVoidPointer(d.data,0))){
        std::cerr<<"ImageResource::patch(): Error: wrong width"<<std::endl;
        return;
    }
    if(height!=deserialise_uint32(shiftVoidPointer(d.data,4))){
        std::cerr<<"ImageResource::patch(): Error: wrong height"<<std::endl;
        return;
    }
    if(size!=deserialise_uint32(shiftVoidPointer(d.data,8))){
        std::cerr<<"ImageResource::patch(): Error: wrong size"<<std::endl;
        return;
    }

    //std::cerr<<"ImageResource::patch()"<<std::endl;

    // width+height+size
    uint32_t headersize=(4+4+4);

    // (size/8)+1 bytes for diffbits
    uint32_t numberOfDiffBytes=(size>>3)+1;

    uint8_t *diffData=(uint8_t*)d.data;

    uint8_t *diffBits=diffData+headersize;
    uint8_t *diffBytes=diffBits+numberOfDiffBytes;

    uint8_t bitNo=0;
    for(uint32_t i=0;i<size;i++){
        //std::cerr<<"ImageResource::patch(): i:"<<i<<" bitNo:"<<(uint32_t)bitNo<<std::endl;
        if((*diffBits)&(1<<bitNo)){
            //std::cerr<<"ImageResource::patch(): patching byte "<<i<<" with data from offset "<<diffBytes-diffData<<std::endl;
            ((uint8_t*)data)[i] = *diffBytes;
            diffBytes++;
        }
        bitNo++;
        if(bitNo==8){
            bitNo=0;
            diffBits++;
        }
    }
    //std::cerr<<"ImageResource::patch(): processed "<<diffBytes-diffData<<" bytes d.size:"<<d.size<<" bytes "<<std::endl;
}

QImage ImageResource::toQImage(){
    if(!data){
        std::cerr<<"ImageResource::toQImage(): Error: no data"<<std::endl;
    }
    QImage img(width,height,QImage::Format_RGB32);
    uint8_t *dataPointer=(uint8_t*)data;
    for(uint32_t line=0;line<height;line++){
        QRgb *linePointer=(QRgb*)img.scanLine(line);
        for(uint32_t row=0;row<width;row++){
            *linePointer=qRgba(*(dataPointer+0),*(dataPointer+1),*(dataPointer+2),*(dataPointer+3));
            dataPointer+=4;
            linePointer++;
        }
    }
    return img;
}
void ImageResource::fromQImage(const QImage &other){
    if(data){
        free(data);
    }
    width=other.width();
    height=other.height();
    size=width*height*4;
    std::cerr<<"ImageResource::fromQImage() malloc("<<size<<")"<<std::endl;
    data=malloc(size);
    if(data==NULL){
        std::cerr<<"ImageResource::fromQImage() malloc failed"<<std::endl;
        return;
    }

    uint8_t *dataPointer=(uint8_t*)data;
    for(uint32_t line=0;line<height;line++){
        QRgb *linePointer=(QRgb*)other.scanLine(line);
        for(uint32_t row=0;row<width;row++){
            *(dataPointer+0)=qRed(*linePointer);
            *(dataPointer+1)=qGreen(*linePointer);
            *(dataPointer+2)=qBlue(*linePointer);
            *(dataPointer+3)=qAlpha(*linePointer);
            dataPointer+=4;
            linePointer++;
        }
    }
}

ImageDiff::ImageDiff():data(0),size(0){

}
ImageDiff::ImageDiff(const ImageDiff &other){
    //std::cerr<<"ImageDiff::ImageDiff(const ImageDiff &other)"<<std::endl;
    size=other.size;
    data=malloc(size);
    memcpy(data,other.data,size);
}
ImageDiff::~ImageDiff(){
    if(data){
        free(data);
    }
    //std::cerr<<"ImageDiff::~ImageDiff()"<<std::endl;
}
ImageDiff& ImageDiff::operator=(ImageDiff other){
    //std::cerr<<"ImageDiff::operator=(ImageDiff other)"<<std::endl;
    std::swap(size,other.size);
    std::swap(data,other.data);
    return *this;
}
uint32_t ImageDiff::serial_size(){
    return size+4;
}
bool ImageDiff::serialise(void *pktdata, uint32_t pktsize){
    if(pktsize<serial_size()){
        return false;
    }
    serialise_uint32(pktdata,size);
    memcpy(shiftVoidPointer(pktdata,4),data,size);
    return true;
}
bool ImageDiff::deserialise(void *pktdata, uint32_t pktsize){
    if(data){
        free(data);
    }
    size=deserialise_uint32(pktdata);
    data=malloc(size);
    memcpy(data,shiftVoidPointer(pktdata,4),size);
    return true;
}

// serialisation helpers

void serialise_uint32(void *data, uint32_t val){
    *(((uint8_t*)data)+0)=(uint8_t)(val>>0);
    *(((uint8_t*)data)+1)=(uint8_t)(val>>8);
    *(((uint8_t*)data)+2)=(uint8_t)(val>>16);
    *(((uint8_t*)data)+3)=(uint8_t)(val>>24);
}
uint32_t deserialise_uint32(void *data){
    uint32_t val=0;
    val|=(*(((uint8_t*)data)+0))<<0;
    val|=(*(((uint8_t*)data)+1))<<8;
    val|=(*(((uint8_t*)data)+2))<<16;
    val|=(*(((uint8_t*)data)+3))<<24;
    return val;
}
void* shiftVoidPointer(void *p, uint8_t offset){
    return (void*)(((uint8_t*)p)+offset);
}


// replaces buffer with compressed buffer
// size tells how many bytes to compress, and returns the size of the compressed buffer
void compress(uint8_t *&inbuf, uint32_t &insize){
    uint8_t *inbufbegin=inbuf;
    std::cerr<<"Imagediff::compress(): "<<insize<<" Bytes to compress"<<std::endl;
    // calculate compressed size
    uint32_t zerolength=0;
    uint32_t compressedsize=0;
    for(uint32_t i=0;i<insize;i++)
    {
        if(*inbuf){
            if(zerolength){
                zerolength=0;
                compressedsize=compressedsize+2;
            }
            compressedsize++;
        }else{
            if(zerolength<0xFD){
                zerolength++;
            }else{
                zerolength=0;
                compressedsize=compressedsize+2;
            }
        }
        inbuf++;
    }
    //last zeros
    compressedsize=compressedsize+2;

    // compress data
    //header=4bytes
    compressedsize+=4;
    std::cerr<<"Imagediff::compress(): malloc "<<compressedsize<<" Bytes"<<std::endl;
    uint8_t *outbuf=(uint8_t*)malloc(compressedsize);
    uint8_t *outbufbegin=outbuf;
    *((uint32_t*)outbuf)=insize;
    outbuf+=4;
    zerolength=0;
    inbuf=inbufbegin;
    for(uint32_t i=0;i<insize;i++)
    {
        if(*inbuf){
            if(zerolength){
                *outbuf=0;
                outbuf++;
                *outbuf=zerolength;
                outbuf++;
                zerolength=0;
            }
            *outbuf=*inbuf;
            outbuf++;
        }else{
            if(zerolength<0xFD){
                zerolength++;
            }else{
                zerolength++;
                *outbuf=0;
                outbuf++;
                *outbuf=zerolength;
                outbuf++;
                zerolength=0;
            }
        }
        inbuf++;
    }
    // last zeros
    *outbuf=0;
    outbuf++;
    *outbuf=zerolength;
    outbuf++;

    std::cerr<<"Imagediff::compress(): compressed "<<inbuf-inbufbegin<<" Bytes to "<<outbuf-outbufbegin<<" Bytes"<<std::endl;
    free(inbufbegin);
    inbuf=outbufbegin;
    insize=compressedsize;
}

// replaces compressed buffer with uncompressed buffer
// size tells how many bytes to uncompress, and returns the size of uncompressed buffer
void decompress(uint8_t *&inbuf, uint32_t &insize){
    uint8_t *inbufbegin=inbuf;
    uint32_t outsize=*((uint32_t*)inbuf);
    std::cerr<<"Imagediff::decompress(): malloc "<<outsize<<" Bytes"<<std::endl;
    uint8_t *outbuf=(uint8_t*)malloc(outsize);
    uint8_t *outbufbegin=outbuf;
    // skip header
    inbuf+=4;
    insize-=4;
    for(uint32_t i=0;i<insize;i++){
        if(*inbuf){
            *outbuf=*inbuf;
            outbuf++;
        }else{
            //skip nullbyte
            inbuf++;
            i++;
            //create null
            for(uint32_t j=0;j<(*inbuf);j++){
                *outbuf=0;
                outbuf++;
            }
        }
        inbuf++;
    }

    std::cerr<<"Imagediff::decompress(): decompressed "<<inbuf-inbufbegin<<" Bytes to "<<outbuf-outbufbegin<<" Bytes"<<std::endl;

    free(inbufbegin);
    inbuf=outbufbegin;
    insize=outsize;
}


// alt und tut
/*

ImageDiff::ImageDiff():data(0)
{

}

ImageDiff::ImageDiff(const ImageDiff &other){
    size=other.size;
    compressed=other.compressed;
    if(other.data){
        if(data){
            free(data);
        }
        data=malloc(size);
        memcpy(data,other.data,size);
    }
}

ImageDiff::~ImageDiff(){
    if(data){
        free(data);
    }
}

ImageDiff& ImageDiff::operator =(const ImageDiff other){
    // other is already a copy, so just swap
    // other will destroy old data
    std::swap(*this,other);
    return *this;
}

bool ImageDiff::diff(const QImage &oldimg, const QImage &newimg)
{
    if(oldimg.isNull()||newimg.isNull()){
        std::cerr<<"Imagediff::diff(): at least one image is null"<<std::endl;
        return false;
    }
    if((oldimg.format()!=QImage::Format_RGB32)||(newimg.format()!=QImage::Format_RGB32)){
        std::cerr<<"Imagediff::diff(): wrong format"<<std::endl;
        return false;
    }
    if(oldimg.size()!=newimg.size()){
        std::cerr<<"Imagediff::diff(): sizes not equal"<<std::endl;
        return false;
    }

    size=oldimg.byteCount()+4+4;

    uint8_t *bufferuncompressed=(uint8_t*)malloc(size);

    uint8_t *buffer8u=bufferuncompressed;

    //header
    *((uint32_t*) (buffer8u))=oldimg.width();
    buffer8u+=4;
    *((uint32_t*) (buffer8u))=oldimg.height();
    buffer8u+=4;

    std::cerr<<"Imagediff::diff(): width:"<<oldimg.width()<<" height:"<<oldimg.height()<<std::endl;

    // calculate diff
    for(uint32_t line=0;line<oldimg.height();line++){
        uint8_t *o=(uint8_t*)oldimg.scanLine(line);
        uint8_t *n=(uint8_t*)newimg.scanLine(line);
        for(uint32_t row=0;row<(oldimg.width()*4);row++){
            if((*o)==(*n)){
                *buffer8u=0;
            }else{
                //std::cerr<<"Imagediff::diff(): new pixel line:"<<line<<" row:"<<row<<" val:"<<(uint32_t)*n<<std::endl;
                *buffer8u=*n;

                //replace 0, because 0 will mark unchanged regions
                if(*buffer8u==0){
                    std::cerr<<"Imagediff::diff(): WARNING: imagedata contains 0, setting to 1"<<std::endl;
                    *buffer8u=1;
                }
            }
            o++;
            n++;
            buffer8u++;
        }

    }

    std::cerr<<"Imagediff::diff(): calculated diff over "<<buffer8u-bufferuncompressed<<" Bytes"<<std::endl;

    if(data){
        free(data);
    }
    data=bufferuncompressed;

    compress(data,size);
    compressed=true;

    std::cerr<<"Imagediff::diff(): size:"<<size<<" Bytes"<<std::endl;

    return true;
}

bool ImageDiff::patch(QImage &oldimage){

    if(!data){
        std::cerr<<"Imagediff::patch(): no data to patch"<<std::endl;
        return false;
    }

    if(compressed){
        decompress(data,size);
        compressed=false;
    }

    uint8_t *inbuffer;
    inbuffer=data;

    uint32_t width= *((uint32_t*)inbuffer);
    inbuffer+=4;

    uint32_t height= *((uint32_t*)inbuffer);
    inbuffer+=4;

    std::cerr<<"Imagediff::patch(): old width:"<<oldimage.width()<<" old height:"<<oldimage.height()<<std::endl;
    std::cerr<<"Imagediff::patch(): new width:"<<width<<" new height:"<<height<<std::endl;

    QImage newimage(width,height,QImage::Format_RGB32);
    //newimage.fill(Qt::black);//-------------
    //uint8_t *newimagebits=(uint8_t*)newimage.bits();
    //uint8_t *oldimagebits=(uint8_t*)oldimage.bits();

    uint32_t line=0;
    uint32_t row=0;
    uint8_t *newimagebits;
    uint8_t *oldimagebits;

    //                  skip header
    for(uint32_t i=0;i<(size-(4+4));i++){
        if(row==(width*4)){
            line++;
            row=0;
        }
        if(row==0){
            //std::cerr<<"Imagediff::patch(): i:"<<i<<" line:"<<line<<" row:"<<row<<std::endl;
            newimagebits=(uint8_t*)newimage.scanLine(line);
            oldimagebits=(uint8_t*)oldimage.scanLine(line);
        }

        // *newimagebits=128;

        if(*inbuffer){
            //std::cerr<<"Imagediff::patch(): new pixel i:"<<i<<" line:"<<line<<" row:"<<row<<" val:"<<(uint32_t)*inbuffer<<std::endl;
            *newimagebits=*inbuffer;
        }else{
            *newimagebits=*oldimagebits;
        }
        row++;
        inbuffer++;

        newimagebits++;
        oldimagebits++;
    }

    std::cerr<<"Imagediff::patch(): processed "<<inbuffer-data<<" Bytes of data"<<std::endl;

    oldimage=newimage;
    return true;
}

void ImageDiff::compress(uint8_t *&inbuf, uint32_t &insize){
    uint8_t *inbufbegin=inbuf;
    std::cerr<<"Imagediff::compress(): "<<insize<<" Bytes to compress"<<std::endl;
    // calculate compressed size
    uint32_t zerolength=0;
    uint32_t compressedsize=0;
    for(uint32_t i=0;i<insize;i++)
    {
        if(*inbuf){
            if(zerolength){
                zerolength=0;
                compressedsize=compressedsize+2;
            }
            compressedsize++;
        }else{
            if(zerolength<0xFD){
                zerolength++;
            }else{
                zerolength=0;
                compressedsize=compressedsize+2;
            }
        }
        inbuf++;
    }
    //last zeros
    compressedsize=compressedsize+2;

    // compress data
    //header=4bytes
    compressedsize+=4;
    std::cerr<<"Imagediff::compress(): malloc "<<compressedsize<<" Bytes"<<std::endl;
    uint8_t *outbuf=(uint8_t*)malloc(compressedsize);
    uint8_t *outbufbegin=outbuf;
    *((uint32_t*)outbuf)=insize;
    outbuf+=4;
    zerolength=0;
    inbuf=inbufbegin;
    for(uint32_t i=0;i<insize;i++)
    {
        if(*inbuf){
            if(zerolength){
                *outbuf=0;
                outbuf++;
                *outbuf=zerolength;
                outbuf++;
                zerolength=0;
            }
            *outbuf=*inbuf;
            outbuf++;
        }else{
            if(zerolength<0xFD){
                zerolength++;
            }else{
                zerolength++;
                *outbuf=0;
                outbuf++;
                *outbuf=zerolength;
                outbuf++;
                zerolength=0;
            }
        }
        inbuf++;
    }
    // last zeros
    *outbuf=0;
    outbuf++;
    *outbuf=zerolength;
    outbuf++;

    std::cerr<<"Imagediff::compress(): compressed "<<inbuf-inbufbegin<<" Bytes to "<<outbuf-outbufbegin<<" Bytes"<<std::endl;
    free(inbufbegin);
    inbuf=outbufbegin;
    insize=compressedsize;
}

void ImageDiff::decompress(uint8_t *&inbuf, uint32_t &insize){
    uint8_t *inbufbegin=inbuf;
    uint32_t outsize=*((uint32_t*)inbuf);
    std::cerr<<"Imagediff::decompress(): malloc "<<outsize<<" Bytes"<<std::endl;
    uint8_t *outbuf=(uint8_t*)malloc(outsize);
    uint8_t *outbufbegin=outbuf;
    // skip header
    inbuf+=4;
    insize-=4;
    for(uint32_t i=0;i<insize;i++){
        if(*inbuf){
            *outbuf=*inbuf;
            outbuf++;
        }else{
            //skip nullbyte
            inbuf++;
            i++;
            //create null
            for(uint32_t j=0;j<(*inbuf);j++){
                *outbuf=0;
                outbuf++;
            }
        }
        inbuf++;
    }

    std::cerr<<"Imagediff::decompress(): decompressed "<<inbuf-inbufbegin<<" Bytes to "<<outbuf-outbufbegin<<" Bytes"<<std::endl;

    free(inbufbegin);
    inbuf=outbufbegin;
    insize=outsize;
}
*/
