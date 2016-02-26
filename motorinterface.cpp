#include "motorinterface.h"
#include <iostream>

MotorInterface::MotorInterface(SimpleCallbackDispatcher* dispatcher, int node, int channel)
{
    _pos = 0;
    _node = node;
    _channel = channel;

    using namespace std::placeholders;
    _subscriber =  dispatcher->add( std::bind( &MotorInterface::messageReceived, this, _1) );
}

void MotorInterface::setTargetPosition(float pos)
{
    _pos = pos;
}

float MotorInterface::getActualPosition() const
{
    return _pos;
}

void MotorInterface::messageReceived(int value)
{
    std::cout<< _node << " messageReceived " << value << std::endl;
    fflush(stdout);
}

