#include "Simpit.h"
#include "COBS.h"
#include "CheckSum.h"
#include "CoreSimpitMessageTypes.h"
#include "CoreSimpitMessageTypeIds.h"

#define SIMPIT_VERSION "2.4.0"

Simpit::Simpit(SimpitMessageTypeProvider *messageTypes, Stream &serial)
{
    _messageTypes = messageTypes;
    _serial = new SerialPort(serial);
    _register = new RegisterHandler();
    _deregister = new DeregisterHandler();
}

bool Simpit::Init(byte response)
{
    // Empty the serial buffer
    _serial->Clear();

    // Broadcast a SYN request
    Synchronisation synchronisation = Synchronisation();
    synchronisation.Type = SynchronisationMessageTypeEnum::SYN;
    synchronisation.Version = FixedString(SIMPIT_VERSION);
    _serial->TryWriteOutgoing(SIMPIT_CORE_OUTGOING_SYNCHRONISATION_ID, &synchronisation, sizeof(Synchronisation));

    // Wait for reply - if non in 1 sec, return false
    byte count = 0;
    while(_serial->TryReadIncoming(_buffer) == false)
    {
        count += 1;
        delay(100);

        if(count > 10)
        {
            return false;
        }
    }

    byte id;
    if(_buffer.TryReadByte(id) == false)
    { // No data recieved?
        return false;
    }

    if(id != SIMPIT_CORE_INCOMING_HANDSHAKE_MESSAGE_ID)
    { // Invalid Handshake packet
        return false;
    }

    Handshake handshake = Handshake();
    _buffer.TryReadBytes(sizeof(Handshake), &handshake);

    if(handshake.HandshakeType != 0x01)
    { // Not a SYNACK response
        return false;
    }
    
    if(handshake.Payload != response)
    { // Incorrect handshake response
        return false;
    }

    synchronisation.Type = SynchronisationMessageTypeEnum::ACK;
    _serial->TryWriteOutgoing(SIMPIT_CORE_OUTGOING_SYNCHRONISATION_ID, &synchronisation, sizeof(Synchronisation));

    return true;
}

void Simpit::Update()
{
    this->ReadIncoming();
}

int Simpit::ReadIncoming()
{
    int count = 0;
    while(_serial->TryReadIncoming(_buffer))
    {
        byte id;
        if(_buffer.TryReadByte(id) == false)
        { // Invalid message
            continue;
        }

        IncomingSimpitMessageType* incoming;
        if(_messageTypes->TryGetIncomingMessageType(id, *&incoming) == false)
        { // Unknown message type id
            this->Log("Simpit-Arduino: Unknown message: " + id);
            continue;
        }

        incoming->Publish(this, _buffer);
        count++;
    }

    _buffer.Clear();
    return count;
}

void Simpit::Log(String value)
{
    this->Log(value, CustomLogFlags::PrintToScreen);
}

void Simpit::Log(String value, CustomLogFlags flags)
{
    CustomLog log = CustomLog();
    log.Flags = flags;
    log.Value = FixedString(value);

    this->WriteOutgoing(log);
}