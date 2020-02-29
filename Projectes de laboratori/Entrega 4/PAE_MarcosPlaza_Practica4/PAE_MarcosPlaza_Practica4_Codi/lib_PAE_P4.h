/******************************
*
* Practica 4 | Programacio darquitectures encastades
* Marcos Plaza Gonzàlez
* 20026915
*
 *****************************/
#ifndef LIB_PAE_P4_H_
#define LIB_PAE_P4_H_

//definicion de constantes para las diferentes funciones
#define WRITE 0x03
#define READ 0x02
#define UP 1
#define DOWN 5
#define NITRO 0xFF
#define SLOW 0x70

typedef int bool;
#define true 1
#define false 0

typedef uint8_t byte;
#define TXD2_2Y (UCA2IFG & UCTXIFG)

//Formato de los paquetes de datos
struct RxReturn{
    byte StatusPacket[16];
    bool timeout;
    bool eCheckSum;
};

uint16_t temps;
uint16_t DatoLeido_UART;
byte Byte_Recibido;

//Funciones para transmision/recepción de datos
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16]);
struct RxReturn RxPacket(void);
void TxUAC2(byte bTxdData);
void Sentit_Dades_Rx(void);
void Sentit_Dades_Tx(void);
void EUSCIA2_IRQHandler(void);
struct RxReturn leerSensor(void);
uint32_t checkSum(byte packet[16], uint8_t length);
bool timeOut(uint16_t t);

//Comandos de los motores
void stopMovimientoRobot(void);
void encenderLedsMotor(bool,bool);
void apagarLedsMotor(void);
void moverRobot(bool);
void nitroRobot(bool);
void spinRobot(bool);

//Funciones relativas al timer
void init_Timers();
void Activa_TimerA1_TimeOut();
void TA1_0_IRQHandler (void);

#endif /* LIB_PAE_P4_H_ */
