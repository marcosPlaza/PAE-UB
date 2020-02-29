#include "RobotFunctionalities.h"

//Función para encender los leds del motor
//Recibe dos booleans uno para cada motor
void encenderLedsMotor(bool rLed, bool lLed){
    byte parametros[16];

    parametros[0] = LED_MOTOR;                  //ponemos el led del motor a 1 (encendido)
    parametros[1] = 1;

    if(rLed){
        TxPacket(2, 2, WRITE, parametros);    //enviamos al motor2
        RxPacket();                                 //Comprobamos que se ha recibido correctamente
    }

    if(lLed){
        TxPacket(3, 2, WRITE, parametros);    //enviamos al motor3
        RxPacket();                                 //Comprobamos que se ha recibido correctamente
    }
}

//Función para apagar los leds del motor
void apagarLedsMotor(void){
    byte parametros[16];

    parametros[0] = LED_MOTOR;              //ponemos el led del motor a 0 (apagado)
    parametros[1] = 0;

    TxPacket(2, 2, WRITE, parametros);   //enviamos al motor2
    RxPacket();                             //Comprobamos que se ha recibido correctamente

    TxPacket(3, 2, WRITE, parametros);   //enviamos al motor3
    RxPacket();                             //Comprobamos que se ha recibido correctamente

}

//Funcion para mover el robot
void moverRobot(bool forward, uint16_t speed){
    byte parametros[16];

    parametros[0] = MOVER_MOTOR; // Escribimos el moving speed - direccion MOVER_MOTOR

    if(speed <= MAXSPEED && speed >=MINSPEED){
        parametros[1] = speed; // velocidad moedrada
    }else{
        parametros[1] = MINSPEED; // velocidad moedrada
    }

    if(forward){
        parametros[2] = DOWN; // Vamos hacia delante
        TxPacket(2, 3, WRITE, parametros); //Enviamos al motor 2
        RxPacket();

        parametros[2] = UP;
        TxPacket(3, 3, WRITE, parametros); //Enviamos al motor 3
        RxPacket();

    }else{
        parametros[2] = UP; // Vamos hacia atrás
        TxPacket(2, 3, WRITE, parametros); // Enviamos al motor 2
        RxPacket();

        parametros[2] = DOWN;
        TxPacket(3, 3, WRITE, parametros); // Enviamos al motor 3
        RxPacket();

    }

}

//Paramos el movimiento del robot
void stopMovimientoRobot(void){
    byte parametros[16];

    parametros[0] = MOVER_MOTOR;                //ponemos a 0 la direccion MOVER_MOTOR
    parametros[1] = 0;
    parametros[2] = 0;
    TxPacket(2, 3, WRITE, parametros);    //enviamos la informacion al motor2
    RxPacket();                                 //Comprobamos que se ha recibido correctamente
    TxPacket(3, 3, WRITE, parametros);    //enviamos la informacion al motor3
    RxPacket();                                 //Comprobamos que se ha recibido correctamente
}

//Funcion para girar el robot
void spinRobot(bool sentido, uint16_t speed){
    byte parametros[16];
    stopMovimientoRobot();         //detenemos los motores para mover solo la rueda que toca

    parametros[0] = MOVER_MOTOR;

    //if(speed <= MAXSPEED && speed >=MINSPEED){
            //parametros[1] = speed; // velocidad moedrada
    //}else{
            parametros[1] = MINSPEED; // velocidad moedrada
    //}

    if(sentido){
        parametros[2] = UP;                 //Motor 2 hacia adelante
        TxPacket(3, 3, WRITE, parametros);    //diremos que se mueva solo el motor2 hacia adelante - sentido horario
        RxPacket();                         //Comprobamos que se ha recibido correctamente

    }else{
        parametros[2] = DOWN;               //Motor 3 hacia atras
        TxPacket(2, 3, WRITE, parametros);    //diremos que se mueva solo el motor3 hacia atras - sentido antihorario
        RxPacket();                      //Comprobamos que se ha recibido correctamente

    }

}

//Funcion que nos devuelve un tipo RxReturn para leer el sensor
struct RxReturn leerSensor(void)             //se encarga de devolver los datos que recive de los sensores
{
    byte parametros[16];

    parametros[0] = 0X1A;                         //Accedemos a la direccion 0x1A
    parametros[1] = 3;                              //indicamos los siguientes 3 sensores (iz,centreal,drch)
    TxPacket(100, 2, READ, parametros);      //enviamos la trama para que lea los valores del motor100
    struct RxReturn retorn = RxPacket();            //Comprobamos que se ha recibido correctamente
    return retorn;
}
