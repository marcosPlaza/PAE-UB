#ifndef ROBOTFUNCTIONALITIES_H_
#define ROBOTFUNCTIONALITIES_H_
#include "HardwareAbstractLayer.h"

//definicion de constantes para las diferentes funciones
#define MOTOR_2 2
#define MOTOR_3 3
#define WRITE 0x03
#define READ 0x02
#define UP 1
#define DOWN 5
#define MAXSPEED 0xA0
#define MINSPEED 0x70
#define MOVER_MOTOR 0x20
#define LED_MOTOR 0x19
#define SENSOR_100 100
#define IR_CENTRAL 0x1B
#define IR_IZQ 0x1A
#define IR_DRC 0x1C

//Comandos de los motores
void stopMovimientoRobot(void);
void encenderLedsMotor(bool,bool);
void apagarLedsMotor(void);
void moverRobot(bool, uint16_t);
void spinRobot(bool, uint16_t);
struct RxReturn leerSensor(void);

#endif /* ROBOTFUNCTIONALITIES_H_ */
