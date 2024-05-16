#ifndef SerialPort_h
#define SerialPort_h

#include <Arduino.h>
#include "SimpitStream.h"

class SerialPort
{

private:
    Stream *_serial;
    SimpitStream *_incoming;
    SimpitStream *_outgoing;

public:
    SerialPort(Stream &serial);

    bool TryReadIncoming(SimpitStream &incoming);

    bool TryWriteOutgoing(byte messageTypeId, void* data, unsigned int size);
};

#endif