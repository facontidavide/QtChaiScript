#ifndef MOTORINTERFACE_H
#define MOTORINTERFACE_H

#include "callbackdispatcher.h"

class MotorInterface
{
public:
    MotorInterface(SimpleCallbackDispatcher* dispatcher,  int node, int channel);

    void setTargetPosition(float pos);
    float getActualPosition() const;

    void messageReceived(int value);

private:
    float _pos;
    int _node;
    int _channel;
    CallbackSubscriber _subscriber;
};

#endif // MOTORINTERFACE_H
