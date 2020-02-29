/******************************
*
* Practica 4 | Programacio darquitectures encastades
* Marcos Plaza Gonzàlez
* 20026915
*
 *****************************/
#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432
#include "lib_PAE_P4.h"

#define TXD2_READY (UCA2IFG & UCTXIFG)

//Función para inicializar los timers
void init_Timers(){
    TA1CTL |= 0x0200; //iniciamos SMCLK en Stop mode
    TA1CCTL0 |= 0x0010; //habilitamos la interrupcion
    TA1CCR0 = 24000; // seteamos el tiempo de la interrupcion
    temps = 0; //seteamos la variable tiempo a 0
}

//Establecemos el timer a modo UP
void Activa_TimerA1_TimeOut()
{
    TA1CTL |= MC_1; //modo UP
}

//Establecemos el timer a modo STOP
void Stop_Timeout(){
    TA1CTL &= ~MC_0; //modo STOP
    temps = 0;
}

//Reseteamos la variable temps
void Reset_Timeout(){
    TA1CTL &= ~MC_0; //modo STOP
    temps = 0;
    TA1CTL |= MC_1; //modo UP
}

/* funcions per canviar el sentit de les comunicacions */
void Sentit_Dades_Rx(void)
{   //Configuraci� del Half Duplex dels motors: Recepci�
    P3OUT &= ~BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 0 (Rx)
}
void Sentit_Dades_Tx(void)
{   //Configuraci� del Half Duplex dels motors: Transmissi�
    P3OUT |= BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Tx)
}

void TxUAC2(byte bTxdData)
{
    while(!TXD2_READY); // Espera a que estigui preparat el buffer de transmissi�
    UCA2TXBUF = bTxdData;
}

//Función de transmisión del paquete de instrucciones
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16])
{
    byte bCount,bCheckSum,bPacketLength;
    volatile int i = 50;
    char error[] = "adr. no permitida";

    if ((Parametros[0]<5 && bInstruction==3)){//si se intenta escribir en una direccion <= 0x05,
        //emitir mensaje de error de direccion prohibida:
          halLcdPrintLine(error,8,INVERT_TEXT);
          //y salir de la funcion sin mas:
          return 0;
    }

    byte TxBuffer[32];
    Sentit_Dades_Tx(); //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)
    TxBuffer[0] = 0xff; //Primers 2 bytes que indiquen inici de trama FF, FF.
    TxBuffer[1] = 0xff;
    TxBuffer[2] = bID; //ID del m�dul al que volem enviar el missatge
    TxBuffer[3] = bParameterLength+2; //Length(Parameter,Instruction,Checksum)
    TxBuffer[4] = bInstruction; //Instrucci� que enviem al M�dul

    for(bCount = 0; bCount < bParameterLength; bCount++) //Comencem a generar la trama que hem d�enviar
    {
        TxBuffer[bCount+5] = Parametros[bCount];
    }

    bCheckSum = 0;
    bPacketLength = bParameterLength+4+2;

    for(bCount = 2; bCount < bPacketLength-1; bCount++) //C�lcul del checksum
    {
        bCheckSum += TxBuffer[bCount];
    }

    TxBuffer[bCount] = ~bCheckSum; //Escriu el Checksum (complement a 1)

    for(bCount = 0; bCount < bPacketLength; bCount++) //Aquest bucle �s el que envia la trama al M�dul Robot
    {
        TxUAC2(TxBuffer[bCount]);
    }

    while((UCA2STATW&UCBUSY)); //Espera fins que s�ha transm�s el �ltim byte

    Sentit_Dades_Rx(); //Posem la l�nia de dades en Rx perqu� el m�dul Dynamixel envia resposta

    return(bPacketLength);
}

struct RxReturn RxPacket(void)
{
    struct RxReturn respuesta;
    byte bCount, bLenght, bChecksum;
    bool Rx_time_out;
    Sentit_Dades_Rx(); //Ponemos la linea half duplex en Rx (recibir dato)
    Activa_TimerA1_TimeOut();//Activamos el timer para el timeOut

    for(bCount = 0; bCount < 4; bCount++)
    {
        Reset_Timeout(); //Lo reseteamos...
        Byte_Recibido=false; //si esta a false es que no se ha recibido el byte

        while (!Byte_Recibido)
        {
            Rx_time_out=timeOut(1000); // corresponde a 10 ms
            if (Rx_time_out)break;
        }

        if (Rx_time_out)break; //sale del for si ha habido el tiempo de espera ha superado lo que debería

        //De otra forma continuamos...
        respuesta.StatusPacket[bCount] = DatoLeido_UART;

    }

    if (!Rx_time_out){
    // Continua llegint la resta de bytes del Status Packet
        bLenght = DatoLeido_UART;
        for(bCount = 0; bCount < bLenght; bCount++) {
                Reset_Timeout();
                Byte_Recibido=false; //No se ha recibido el byte
                while (!Byte_Recibido)
                {
                    Rx_time_out=timeOut(1000); // corresponde a 10 ms
                    if (Rx_time_out)break;
                }

                if (Rx_time_out)break; //sale del for si ha habido el tiempo de espera ha superado lo que debería

                //De otra forma continuamos...
                respuesta.StatusPacket[bCount+4] = DatoLeido_UART;

        }

        bChecksum = respuesta.StatusPacket[bLenght+3];
        //por último calculamos el checkSum

        respuesta.eCheckSum =  bChecksum !=  ~checkSum(respuesta.StatusPacket, bLenght);  //te comprueba si los 2 checkSum son iguales (0) o diferentes = ERROR (1)

    } if(Rx_time_out) {respuesta.timeout = true;}

    Stop_Timeout();//Si hemos llegado a este punto paramos el timer

    return respuesta;
}

//interrupcion de recepcion en la UART
void EUSCIA2_IRQHandler(void)
{
    UCA2IE &= ~UCRXIE; //Interrupciones desactivadas en RX
    DatoLeido_UART = UCA2RXBUF;
    Byte_Recibido=true;
    UCA2IE |= UCRXIE; //Interrupciones reactivadas en RX
}

//Función para realizar el checkSum segun la formula
uint32_t checkSum(byte packet[16], uint8_t length){
    int i;      //contador for
    uint32_t result= 0;   //vamos acumulando la suma del checkSum

    for (i = 2; i < length +3; i++){
        result += packet[i];      //sumamos el ID + cada uno de los parametros + el lenght
    }

    return result;      //devolvermos el resultado para comprobar con el checkSum original
}

//Devuelve true si el contador tiempo ha superado a t
bool timeOut(uint16_t t)
{
    return temps>=t;
}

//Subrutina para la interrupcion del timer A1
void TA1_0_IRQHandler (void){
    TA1CCTL0 &= ~CCIE; //Conv� inhabilitar la interrupci� al comen�ament
    temps ++;   //el timer aumentará el tiempo +1

    TA1CCTL0 &= ~CCIFG; //Hem de netejar el flag de la interrupci�
    TA1CCTL0 |= CCIE; //S�ha d�habilitar la interrupci� abans de sortir
}

//Función para encender los leds del motor
//Recibe dos booleans uno para cada motor
void encenderLedsMotor(bool rLed, bool lLed){
    byte parametros[16];

    parametros[0] = 0x19;                  //ponemos el led del motor a 1 (encendido)
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

    parametros[0] = 0x19;              //ponemos el led del motor a 0 (apagado)
    parametros[1] = 0;

    TxPacket(2, 2, WRITE, parametros);   //enviamos al motor2
    RxPacket();                             //Comprobamos que se ha recibido correctamente

    TxPacket(3, 2, WRITE, parametros);   //enviamos al motor3
    RxPacket();                             //Comprobamos que se ha recibido correctamente

}

//Funcion para mover el robot
void moverRobot(bool forward){
    byte parametros[16];

    parametros[0] = 0x20; // Escribimos el moving speed - direccion 0x20
    parametros[1] = SLOW; // velocidad moedrada

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

    parametros[0] = 0x20;                //ponemos a 0 la direccion 0x20
    parametros[1] = 0;
    parametros[2] = 0;
    TxPacket(2, 3, WRITE, parametros);    //enviamos la informacion al motor2
    RxPacket();                                 //Comprobamos que se ha recibido correctamente
    TxPacket(3, 3, WRITE, parametros);    //enviamos la informacion al motor3
    RxPacket();                                 //Comprobamos que se ha recibido correctamente
}

//Consiste en lo mismo que moverRobot(bool forward) pero seteando una velocidad mayor
void nitroRobot(bool forward){
    byte parametros[16];

    parametros[0] = 0x20;
    parametros[1] = NITRO;

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
        TxPacket(3, 3, WRITE, parametros); //Enviamos al motor 3
        RxPacket();

    }
}

//Funcion para girar el robot
void spinRobot(bool sentido){
    byte parametros[16];
    stopMovimientoRobot();         //detenemos los motores para mover solo la rueda que toca

    parametros[0] = 0x20;
    parametros[1] = SLOW;               //los 7 primeros bits los ponemos a 0x70 (112)
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

    parametros[0] = 0X1B;                         //Accedemos a la direccion 0x1B
    parametros[1] = 3;                              //indicamos los siguientes 3 sensores (iz,centreal,drch)
    TxPacket(100, 2, READ, parametros);      //enviamos la trama para que lea los valores del motor100
    struct RxReturn retorn = RxPacket();            //Comprobamos que se ha recibido correctamente
    return retorn;
}
