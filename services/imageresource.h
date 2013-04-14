#ifndef IMAGERESOURCE_H
#define IMAGERESOURCE_H

#include <QImage>

class ImageDiff;

class ImageResource{
public:
    ImageResource();
    ImageResource(const ImageResource &other);
    ~ImageResource();
    ImageResource& operator= (ImageResource other);

    // serialisation methods
    uint32_t serial_size();
    bool serialise(void *pktdata, uint32_t pktsize);
    bool deserialise(void *pktdata, uint32_t pktsize);

    ImageDiff diff(ImageResource modifiedResource, bool &resourceIsModified);
    void patch(ImageDiff d);

    QImage toQImage();
    void fromQImage(const QImage &other);

private:
    uint32_t width;
    uint32_t height;

    void *data;
    uint32_t size;
};

class ImageDiff{
public:
    ImageDiff();
    ImageDiff(const ImageDiff &other);
    ~ImageDiff();
    ImageDiff& operator=(ImageDiff other);

    uint32_t serial_size();
    bool serialise(void *pktdata, uint32_t pktsize);
    bool deserialise(void *pktdata, uint32_t pktsize);

    void *data;
    uint32_t size;
};

// serialisation helpers
void serialise_uint32(void *data,uint32_t val);
uint32_t deserialise_uint32(void *data);
void* shiftVoidPointer(void *p, uint8_t offset);

void compress(uint8_t *&,uint32_t &);
void decompress(uint8_t *&,uint32_t &);

// alt und tut
/*
class ImageDiff
{
public:
    ImageDiff();

    ImageDiff(const ImageDiff &other);
    ~ImageDiff();
    ImageDiff& operator= (const ImageDiff other);

    bool diff(const QImage& oldimg, const QImage &newimg);
    bool patch(QImage &);

    bool serialize(void *,const uint32_t&);
    void serial_size(uint32_t&);

    bool deserialize(void *,const uint32_t&);

private:
    void compress(uint8_t *&,uint32_t &);
    void decompress(uint8_t *&,uint32_t &);

    uint8_t *data;
    uint32_t size;
    bool compressed;
};
*/
#endif // IMAGERESOURCE_H
