/******************************
*
* Practica 4 | Programacio darquitectures encastades
* Marcos Plaza Gonzàlez
* 20026915
*
 *****************************/
//includes necesarios
#include "msp.h"
#include "lib_PAE2.h"
#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>

//Llibrería de la part 2 de la pràctica
#include "lib_PAE_P4.h"

//Definimos los dos botones y el joystick de la placa
#define Button_S1 1
#define Button_S2 2
#define Jstick_Left 3
#define Jstick_Right 4
#define Jstick_Up 5
#define Jstick_Down 6
#define Jstick_Center 7

#define TXD2_READY (UCA2IFG & UCTXIFG)

//Cadenas de texto utilizadas en la practica
char saludo[16] = " PRACTICA 4 PAE";//max 15 caracteres visibles

char adelanteMSG[16] = "    ADELANTE";
char atrasMSG[16] = "     ATRAS  ";
char dchaMSG[16] = "     -->    ";
char izqMSG[16] = "     <--    ";
char powerUpMSG[16] = "  NITRO BOOST!";
char stopMSG[16] = "     STOP    ";

char cadena[16];
char cadena2[16];
char borrado[] = "               "; //una linea entera de 15 espacios en blanco

uint8_t linea = 1;
uint8_t estado = 0;
uint8_t estado_anterior = 8;

bool forward = true;//El nostre robot tindrá un sentit endavant per defecte
bool direccion = true;//El nostre robot girará cap a la dreta per defecte

/**************************************************************************
 * INICIALIZACION DEL CONTROLADOR DE INTERRUPCIONES (NVIC).
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
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

    NVIC->ICPR[0] |= BITA;//Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BITA; //interrupcion timer A1

    NVIC->ICPR[0] |= BIT(18);//Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BIT(18); //interrupcion UART

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**************************************************************************
 * INICIALIZACION DE LOS BOTONES & LEDS DEL BOOSTERPACK MK II.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
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

//Funció per inicialitzar la UART
void init_UART(void)
{
    UCA2CTLW0 |= UCSWRST;       //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK; //UCSYNC=0 mode asincron
                                //UCMODEx=0 seleccionem mode UART
                                //UCSPB=0 nomes 1 stop bit
                                //UC7BIT=0 8 bits de dades
                                //UCMSB=0 bit de menys pes primer
                                //UCPAR=x ja que no es fa servir bit de paritat
                                //UCPEN=0 sense bit de paritat
                                //Triem SMCLK (16MHz) com a font del clock BRCLK
    UCA2MCTLW = UCOS16;         //Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3;                //Prescaler de BRCLK fixat a 3. Com SMCLK va a24MHz,
                                //volem un baud rate de 500kb/s i fem sobre-mostreig de 16
                                //el rellotge de la UART ha de ser de 8MHz (24MHz/3).

    //Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3;      //I/O funcio: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3);
    //Configurem pin de seleccio del sentit de les dades Transmissio/Recepecio
    P3SEL0 &= ~BIT0; //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0; //Port P3.0 com sortida (Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0; //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la linia de comunicacions serie
    UCA2IE |= UCRXIE; //Aixo nomes sha dactivar quan tinguem la rutina de recepcio
}

/**************************************************************************
 * INICIALIZACION DE LA PANTALLA LCD.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_LCD(void)
{
    halLcdInit(); //Inicializar y configurar la pantallita
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
}

/**************************************************************************
 * BORRAR LINEA
 *
 * Datos de entrada: Linea, indica la linea a borrar
 *
 * Sin datos de salida
 *
 **************************************************************************/
void borrar(uint8_t Linea)
{
    halLcdPrintLine(borrado, Linea, NORMAL_TEXT); //escribimos una linea en blanco
}

/**************************************************************************
 * ESCRIBIR LINEA
 *
 * Datos de entrada: Linea, indica la linea del LCD donde escribir
 *                   String, la cadena de caracteres que vamos a escribir
 *
 * Sin datos de salida
 *
 **************************************************************************/
void escribir(char String[], uint8_t Linea)

{
    halLcdPrintLine(String, Linea, NORMAL_TEXT); //Enviamos la String al LCD, sobreescribiendo la Linea indicada.
}

void main(void)
{
    WDTCTL = WDTPW+WDTHOLD;         // Paramos el watchdog timer

    //Inicializacions:
    init_ucs_24MHz();       //Ajustes del clock (Unified Clock System)
    init_UART();        //inicializamos la UART
    init_botons();         //Configuramos botones y leds
    init_interrupciones();  //Configurar y activar las interrupciones de los botones
    init_Timers();          //inicializamos los timers
    init_LCD();             // Inicializamos la pantalla

    halLcdPrintLine(saludo,linea, INVERT_TEXT); //escribimos saludo en la primera linea
    linea++;                    //Aumentamos el valor de linea y con ello pasamos a la linea siguiente

    struct RxReturn estructura;   //Estructura para valorar los valores que retorna los sensores de proximidad

    do{
        //Imprimimos los valores que toman los sensores en cada instante
        sprintf(cadena2,"C%3d D%3d I%3d",estructura.StatusPacket[5],estructura.StatusPacket[6],estructura.StatusPacket[7]);
        escribir(cadena2,linea+6);
        //Vamos a estar leyendo datos del sensor continuamente por lo tanto metiendolo en el do while nos aseguramos
        //de que no tengamos que si encuentra un obstaculo no deba refrescarse apretando un boton
        estructura = leerSensor();

                  if(estructura.StatusPacket[2] == 100){
                    if(estructura.StatusPacket[5] >= 100 || estructura.StatusPacket[6] >= 100 || estructura.StatusPacket[7] >= 100){
                        estado_anterior = estado;
                        estado = Button_S2; //Cambiamos el estado
                        escribir(" Sensor active",linea+5);
                    }else{
                        escribir("              ",linea+5);
                    }

                  }

        if (estado_anterior != estado){         // Dependiendo del valor del estado se encender� un LED u otro.{
            sprintf(cadena," Estado %d", estado);    // Guardamos en cadena la siguiente frase: estado "valor del estado"
            escribir(cadena,linea);          // Escribimos la cadena al LCD
            estado_anterior = estado;          // Actualizamos el valor de estado_anterior, para que no est� siempre escribiendo.
            /**********************************************************+
                A RELLENAR POR EL ALUMNO BLOQUE switch ... case
            Para gestionar las acciones:
            Boton S1, estado = 1
            Boton S2, estado = 2
            Joystick left, estado = 3
            Joystick right, estado = 4
            Joystick up, estado = 5
            Joystick down, estado = 6
            Joystick center, estado = 7
            ***********************************************************/

                switch(estado){
                    case Button_S1:
                        apagarLedsMotor();
                        encenderLedsMotor(true,true); // Encendemos luces de movimiento
                        forward = true; // vamos hacia delante
                        moverRobot(forward); // movemos el robot
                        escribir(borrado,linea+4); // escribimos por pantalla hacia donde va
                        escribir(adelanteMSG,linea+3);
                        break;

                    case Button_S2:
                        escribir(stopMSG,linea+3); //escribimos por pantalla hacia donde va
                        escribir(borrado,linea+4);
                        apagarLedsMotor(); // apagamos las luces de movimiento
                        stopMovimientoRobot();       // Paramos los motores
                        break;

                    case Jstick_Up:
                        apagarLedsMotor();
                        encenderLedsMotor(true,true); // Encendemos luces de movimiento
                        forward = true; // obligamos a ir hacia delante
                        escribir(adelanteMSG,linea+3); //escribimos hacia donde va
                        escribir(borrado,linea+4);
                        moverRobot(forward); // movemos el robot
                        break;

                    case Jstick_Down:
                        apagarLedsMotor();
                        encenderLedsMotor(true,true); // Encendemos luces de movimiento
                        forward = false;
                        escribir(atrasMSG,linea+3); //escribimos hacia donde va
                        escribir(borrado,linea+4);
                        moverRobot(forward); // movemos el robot
                        break;

                    case Jstick_Left:
                        apagarLedsMotor();
                        encenderLedsMotor(false,true); // Encendemos luces de movimiento hacia la derecha
                        direccion = false;
                        escribir(izqMSG,linea+3); //escribimos hacia donde va
                        escribir(borrado,linea+4);
                        spinRobot(direccion); // vamos a la izquierda
                        break;

                    case Jstick_Right:
                        apagarLedsMotor();
                        encenderLedsMotor(true,false); // Encendemos luces de movimiento hacia la derecha
                        direccion = true;
                        escribir(dchaMSG,linea+3); //escribimos hacia donde va
                        escribir(borrado,linea+4);
                        spinRobot(direccion); // vamos a la derecha
                        break;

                    case Jstick_Center:
                        apagarLedsMotor();
                        encenderLedsMotor(true,true); // Encendemos luces de movimiento hacia la derecha
                        nitroRobot(forward);
                        if(forward){
                            escribir(borrado,linea+3); //escribimos hacia donde va
                            escribir(adelanteMSG,linea+3);
                        }else{
                            escribir(borrado,linea+3); //escribimos hacia donde va
                            escribir(atrasMSG,linea+3);
                        }
                        escribir(powerUpMSG,linea+4); // Mensaje de turbo
                        break;
                }
            }
    }while(1); //Condicion para que el bucle sea infinito
}

/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectar� qu� bot�n se ha pulsado
 *
 * Sin Datos de entrada
 *
 * Sin datos de salida
 *
 * Actualizar el valor de la variable global estado
 *
 **************************************************************************/

//ISR para las interrupciones del puerto 3:
void PORT3_IRQHandler(void){//interrupcion del pulsador S2
    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= 0xDF;  //interrupciones del boton S2 en port 3 desactivadas
    estado_anterior=0;
    switch(flag){
    case 0x0C: //pin 5
        estado = Button_S2;
        break;
    }

    P3IE |= 0x20;   //interrupciones S2 en port 3 reactivadas
}

//ISR para las interrupciones del puerto 4:
void PORT4_IRQHandler(void){  //interrupci�n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= 0x5D;   //interrupciones Joystick en port 4 desactivadas
    estado_anterior=0;

    switch(flag){
            case 0x04: //pin 1
                estado = Jstick_Center; //center
                break;
            case 0x0C: //pin 5
                estado = Jstick_Right; //right
                break;
            case 0x10: //pin 7
                estado = Jstick_Left; //Left
                break;
            }

    P4IE |= 0xA2;   //interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void){  //interrupci�n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas
    estado_anterior=0;
    switch(flag){
            case 0x04: //pin 1
                estado = Button_S1; //center
                break;
            case 0x0A:// pin 4
                estado = Jstick_Up;
                break;
            case 0x0C: //pin 5
                estado = Jstick_Down; //right
                break;
            }

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}
