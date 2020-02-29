#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432
#include "HardwareAbstractLayer.h"

#define TXD2_READY (UCA2IFG & UCTXIFG)

/*
 * En primer lugar tenemos las funciones que inicializan los recursos de la placa:
 *      -Interrupciones
 *      -Timers
 *      -UART
 *      -Pantalla LCD
 *      -Botones
 */

void init_interrupciones(){
    // Configuracion al estilo MSP430 "clasico":
    // Enable Port 4 interrupt on the NVIC
    // segun datasheet (Tabla "6-12. NVIC Interrupts", capitulo "6.6.2 Device-Level User Interrupts", p80-81 del documento SLAS826A-Datasheet),
    // la interrupcion del puerto 4 es la User ISR numero 38.
    // Segun documento SLAU356A-Technical Reference Manual, capitulo "2.4.3 NVIC Registers"
    // hay 2 registros de habilitacion ISER0 y ISER1, cada uno para 32 interrupciones (0..31, y 32..63, resp.),
    // accesibles mediante la estructura NVIC->ISER[x], con x = 0 o x = 1.
    // Asimismo, hay 2 registros para deshabilitarlas: ICERx, y dos registros para limpiarlas: ICPRx.

    //Int. port 3 = 37 corresponde al bit 5 del segundo registro ISER1:
    NVIC->ICPR[1] |= BIT5; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT5; //y habilito las interrupciones del puerto
    //Int. port 4 = 38 corresponde al bit 6 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT6; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT6; //y habilito las interrupciones del puerto
    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

    //NVIC -> ISER[0] |= BIT8; //interrupcion timerA
    NVIC->ICPR[0] |= BITA;//Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BITA; //interrupcion timer1

    NVIC->ICPR[0] |= BIT(18);//Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BIT(18); //interrupcion UART2

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

//Función para inicializar los timers
void init_Timers(){
    TA1CTL |= 0x0200; //iniciamos SMCLK en Stop mode
    TA1CCTL0 |= 0x0010; //habilitamos la interrupcion
    TA1CCR0 = 24000; // seteamos el tiempo de la interrupcion
    temps = 0; //seteamos la variable tiempo a 0
}

void init_UART(void)
{
    UCA2CTLW0 |= UCSWRST; //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK; //UCSYNC=0 mode as�ncron
    //UCMODEx=0 seleccionem mode UART
    //UCSPB=0 nomes 1 stop bit
    //UC7BIT=0 8 bits de dades
    //UCMSB=0 bit de menys pes primer
    //UCPAR=x ja que no es fa servir bit de paritat
    //UCPEN=0 sense bit de paritat
                        //Triem SMCLK (16MHz) com a font del clock BRCLK
    UCA2MCTLW = UCOS16; // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3; //Prescaler de BRCLK fixat a 3. Com SMCLK va a24MHz,
                        //volem un baud rate de 500kb/s i fem sobre-mostreig de 16
                        //el rellotge de la UART ha de ser de 8MHz (24MHz/3).
                        //Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3; //I/O funci�: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3);
                        //Configurem pin de selecci� del sentit de les dades Transmissi�/Recepeci�
    P3SEL0 &= ~BIT0; //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0; //Port P3.0 com sortida (Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0; //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la l�nia de comunicacions s�rie
    UCA2IE |= UCRXIE; //Aix� nom�s s�ha d�activar quan tinguem la rutina de recepci�
}

void init_botons(void)
{
    //Configuramos botones y leds
    //***************************
    //Leds RGB del MK II:
      P2DIR |= 0x50;  //Pines P2.4 (G), 2.6 (R) como salidas Led (RGB)
      P5DIR |= 0x40;  //Pin P5.6 (B)como salida Led (RGB)
      P2OUT &= 0xAF;  //Inicializamos Led RGB a 0 (apagados)
      P5OUT &= ~0x40; //Inicializamos Led RGB a 0 (apagados)

    //7 Leds
      P7DIR |= 0xFF;  //Pines P7.0 ... P7.7 como salida de leds de la placa secundaria
      P7OUT &= ~0xFF;   //Inicializamos el P7 a 0 (apagados)

    //Boton S1 del MK II:
      P5SEL0 &= ~0x02;   //Pin P5.1 como I/O digital,
      P5SEL1 &= ~0x02;   //Pin P5.1 como I/O digital,
      P5DIR &= ~0x02; //Pin P5.1 como entrada
      P5IES &= ~0x02;   // con transicion L->H
      P5IE |= 0x02;     //Interrupciones activadas en P5.1,
      P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 5
      //P5REN: Ya hay una resistencia de pullup en la placa MK II

    //Boton S2 del MK II:
      P3SEL0 &= ~0x20;   //Pin P3.5 como I/O digital,
      P3SEL1 &= ~0x20;   //Pin P3.5 como I/O digital,
      P3DIR &= ~0x20; //Pin P3.5 como entrada
      P3IES &= ~0x20;   // con transicion L->H
      P3IE |= 0x20;   //Interrupciones activadas en P3.5
      P3IFG = 0;  //Limpiamos todos los flags de las interrupciones del puerto 3
      //P3REN: Ya hay una resistencia de pullup en la placa MK II

    //Configuramos los GPIOs del joystick del MK II:
      P4DIR &= ~(BIT1 + BIT5 + BIT7);   //Pines P4.1, 4.5 y 4.7 como entrades,
      P4SEL0 &= ~(BIT1 + BIT5 + BIT7);  //Pines P4.1, 4.5 y 4.7 como I/O digitales,
      P4SEL1 &= ~(BIT1 + BIT5 + BIT7);
      P4REN |= BIT1 + BIT5 + BIT7;  //con resistencia activada
      P4OUT |= BIT1 + BIT5 + BIT7;  // de pull-up
      P4IE |= BIT1 + BIT5 + BIT7;   //Interrupciones activadas en P4.1, 4.5 y 4.7,
      P4IES &= ~(BIT1 + BIT5 + BIT7);   //las interrupciones se generaran con transicion L->H
      P4IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4

      P5DIR &= ~(BIT4 + BIT5);  //Pines P5.4 y 5.5 como entrades,
      P5SEL0 &= ~(BIT4 + BIT5); //Pines P5.4 y 5.5 como I/O digitales,
      P5SEL1 &= ~(BIT4 + BIT5);
      P5IE |= BIT4 + BIT5;  //Interrupciones activadas en 5.4 y 5.5,
      P5IES &= ~(BIT4 + BIT5);  //las interrupciones se generaran con transicion L->H
      P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4
    // - Ya hay una resistencia de pullup en la placa MK II
}

void init_LCD(void)
{
    halLcdInit(); //Inicializar y configurar la pantallita
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
}

void init_recursos(){
    //Inicializaciones:
    init_ucs_24MHz();       //Ajustes del clock (Unified Clock System)
    init_botons();         //Configuramos botones y leds
    init_interrupciones();  //Configurar y activar las interrupciones de los botones
    init_Timers();       //inicializamos los timers
    init_LCD();             // Inicializamos la pantalla
    init_UART();        //inicializamos la UART
}

/*
 * A continuacion tenemos las funciones relativas a la transmision y recepcion de datos de los modulos dynamixel
 */

// funcions per canviar el sentit de les comunicacions
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
bool timeOut(uint32_t t)
{
    return temps>=t;
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

//Establecemos el timer a modo STOP
void Stop_Timeout_A0(){
    TA0CTL &= ~MC_0; //modo STOP
    temps = 0;
}

//Reseteamos la variable temps
void Reset_Timeout_A0(){
    TA0CTL &= ~MC_0; //modo STOP
    temps = 0;
    TA0CTL |= MC_1; //modo UP
}

//Subrutina para la interrupcion del timer A1
void TA1_0_IRQHandler (void){
    TA1CCTL0 &= ~CCIE; //Conv� inhabilitar la interrupci� al comen�ament
    temps ++;   //el timer aumentará el tiempo +1

    TA1CCTL0 &= ~CCIFG; //Hem de netejar el flag de la interrupci�
    TA1CCTL0 |= CCIE; //S�ha d�habilitar la interrupci� abans de sortir
}

//interrupcion de recepcion en la UART
void EUSCIA2_IRQHandler(void)
{
    UCA2IE &= ~UCRXIE; //Interrupciones desactivadas en RX
    DatoLeido_UART = UCA2RXBUF;
    Byte_Recibido=true;
    UCA2IE |= UCRXIE; //Interrupciones reactivadas en RX
}
