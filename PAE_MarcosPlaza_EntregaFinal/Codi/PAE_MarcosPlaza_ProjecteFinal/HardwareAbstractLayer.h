#ifndef HARDWAREABSTRACTLAYER_H_
#define HARDWAREABSTRACTLAYER_H_

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h"

/*
 * En la Hardware Abstract Layer deberemos tener toda la capa donde se configuran y inicializan los recursos
 * principales de la placa.
 *
 * 1) Inicializaciones de: Interrupciones, botones, UART i Timers junto a sus respectivos handlers. Posteriormente se hara un metodo para inicializar todo
 * 2) Tambien tendremos las funciones para la transmision y la recepcion de datos de los modulos Dynamixel
 * 3) Contaremos con otras funciones de soporte para los timers y la transmision y recepcion de los datos de los modulos
 *
 */

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

void init_recursos();
void init_Timers();
void init_interrupciones();
//void init_LCD(void);
void init_UART(void);
void init_botons(void);

//Funciones para transmision/recepción de datos
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16]);
struct RxReturn RxPacket(void);
void TxUAC2(byte bTxdData);
void Sentit_Dades_Rx(void);
void Sentit_Dades_Tx(void);
void EUSCIA2_IRQHandler(void);
uint32_t checkSum(byte packet[16], uint8_t length);
bool timeOut(uint32_t t);


//Funciones relativas al timer
void Activa_TimerA1_TimeOut();
void Stop_Timeout();
void Reset_Timeout();
void Stop_Timeout_A0();
void Reset_Timeout_A0();
void TA1_0_IRQHandler (void);

#endif /* HARDWAREABSTRACTLAYER_H_ */
