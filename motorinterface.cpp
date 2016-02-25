#include "motorinterface.h"

MotorInterface::MotorInterface(int node, int channel)
{
    _pos = 0;
}

void MotorInterface::setTargetPosition(float pos)
{
    _pos = pos;
}

float MotorInterface::getActualPosition() const
{
    return _pos;
}

