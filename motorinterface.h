#ifndef MOTORINTERFACE_H
#define MOTORINTERFACE_H


class MotorInterface
{
public:
    MotorInterface(int node, int channel);

    void setTargetPosition(float pos);
    float getActualPosition() const;

private:
    float _pos;
};

#endif // MOTORINTERFACE_H
