#ifndef SIMPLESERIALISER_H
#define SIMPLESERIALISER_H

// versuch einen serialiser zu schreiben der von alleine funktioniert
// idee: jeder basisdatentyp wird überladen, und meldet sich dann selbtständig an seinem container an
// der container ist ist die itemklasse, und erbt von einer allgemeinen containerimplementierung
// der container hat eine liste der member und ruft dann deren serialisierung auf
/*
class Serializable_uint8{
public:
    Serializable_uint8(SerialisableContainer *c);
private:
    uint8_t data;
};

class SerialisableContainer{

};

class Message:SerialisableContainer{
public:
    Serializable_uint8 uint8;
};
*/

#endif // SIMPLESERIALISER_H
