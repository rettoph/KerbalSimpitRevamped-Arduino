#ifndef SimpitBuilder_h
#define SimpitBuilder_h

#include <Arduino.h>
#include "SimpitMessageType.h"
#include "Simpit.h"

class SimpitBuilder
{
private:
    uint16_t _typeCount;
    BaseSimpitMessageType **_types;
    bool _deconstructed;

public:
    SimpitBuilder();

    template<typename T> SimpitBuilder RegisterIncoming()
    {
        _types[_typeCount++] = new IncomingSimpitMessageType<T>(T::MessageTypeId, SimpitMessageTypeEnum::Incoming);
        return *this;
    }

    Simpit Build();
};

#endif