#include "RobotManipulation.h"

void init(){
    speed = MINSPEED;
    muteSound = false;
    wall = WALL_D; //Por defecto pared dcha
    direction = true;
    init_recursos();
}

void robotRoute(struct RxReturn estructura){
    uint8_t id = estructura.StatusPacket[2], snsIzquierdo = estructura.StatusPacket[5], snsCentral = estructura.StatusPacket[6],snsDerecho = estructura.StatusPacket[7];

    switch(wall){
        case WALL_I:
            if (id == 100){
                    if (snsCentral >= 35 || snsIzquierdo >= 35){
                        apagarLedsMotor();
                        direction = true;
                        encenderLedsMotor(true,false); // Encendemos luces de movimiento hacia la derecha
                        spinRobot(direction,speed); // vamos a la izquierda

                    }else if (snsCentral <= 30 && snsIzquierdo <= 30){
                        apagarLedsMotor();
                        direction = false;
                        encenderLedsMotor(false,true); // Encendemos luces de movimiento hacia la derecha
                        spinRobot(direction,speed); // vamos a la derecha*
                    }
                    else{
                        apagarLedsMotor();
                        encenderLedsMotor(true,true); // Encendemos luces de movimiento
                        moverRobot(true,speed); // movemos el robot

                        bool timeout;
                        Reset_Timeout();

                        while (1) //Se_ha_recibido_Byte())
                        {
                            timeout=timeOut(550); // tiempo en decenas de microsegundos (ara 10ms)
                            if (timeout) break;//sale del while
                        }

                    }
            }
            break;

        case WALL_D:
            if (id == 100){
                if (snsCentral >= 35 || snsDerecho >= 35){
                    apagarLedsMotor();
                    direction = false;
                    encenderLedsMotor(false, true); // Encendemos luces de movimiento hacia la derecha
                    spinRobot(direction,speed); // vamos a la izquierda

                }else if (snsCentral <= 30 && snsDerecho <= 30){
                    apagarLedsMotor();
                    direction = true;
                    encenderLedsMotor(true, false); // Encendemos luces de movimiento hacia la derecha
                    spinRobot(direction,speed); // vamos a la izquierda
                }
                else{
                    apagarLedsMotor();
                    encenderLedsMotor(true,true); // Encendemos luces de movimiento
                    moverRobot(true,speed); // movemos el robot

                    bool timeout;
                    Reset_Timeout();

                    while (1) //Se_ha_recibido_Byte())
                    {
                        timeout=timeOut(550); // tiempo en decenas de microsegundos (ara 10ms)
                        if (timeout) break;//sale del while
                    }
                }
            }
            break;
    }
}

//Cambiar melodia
void musica()
{
    bool timeout;
    byte parametros[16];
    parametros[0] = 0x28;

    byte notasFFVF[] = {24,7,10,8,5,5,24,10,7,7,10,24,8,5,5,10,2,6,4,24,8,24,10,7,7,10,24,8,5,5,3,8,5,5,7,24,8,10,7,8,24};
    byte temposFFVF[] = {6,3,6,3,6,3,6,3,6,3,6,6,6,6,12,6,6,3,3,6,6,3,3,6,3,6,6,6,6,12,24,12,12,12,12,12,12,12,12,12,12};

    //byte notasFFVF[] = {5,5,5,5,2,3,5,3,5};
    //byte temposFFVF[] = {5,5,5,15,15,10,10};

    uint8_t i = 0;


    for(i=0; i < 41; i++){
        parametros[1] = notasFFVF[i];
        parametros[2] = temposFFVF[i]/3;
        TxPacket(SENSOR_100, 3, WRITE, parametros);
        RxPacket();
        Reset_Timeout();
         while (true) //Se_ha_recibido_Byte())
         {// tiempo en decenas de microsegundos (ara 10ms)
             timeout=timeOut(200);
             if (timeout)break;//sale del while
         }
    }
}

void setSpeed(uint16_t vel){
    speed = vel;
}

void setSound(bool mute){
    muteSound = mute;
}

void setWall(uint8_t right){
    wall = right;
}

void setDirection(bool direc){
    direction = direc;
}


uint16_t getSpeed(){
    return speed;
}

bool getSound(){
    return muteSound;
}

uint8_t getWall(){
    return wall;
}

bool getDirection(){
    return direction;
}

