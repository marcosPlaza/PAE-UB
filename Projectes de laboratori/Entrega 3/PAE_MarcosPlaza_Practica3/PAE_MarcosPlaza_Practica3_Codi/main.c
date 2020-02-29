/******************************
*
* Practica 3 | Programacio darquitectures encastades
* Marcos Plaza Gonzàlez
* 20026915
*
 *****************************/

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE3.h" //Funciones de la anterior practica


//Definimos las constantes necesarias para S1/S2/direcciones del Joystick
#define S1 1
#define S2 2
#define LEFT 3
#define RIGHT 4
#define UP 5
#define DOWN 6
#define CENTER 7

//En este bloque tenemos declaradas las variables globales que necesitamos

//Variables de la anterior practica
char saludo[16] = " PRACTICA 3 PAE"; //Max 15 caracteres visibles
char cadena[16]; //Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)
char lineas[16] = "---------------"; //Cadena para imprimir una linea para separar el clock y la alarma del resto
char aviso1[16] = " Alm. activada"; //Aviso para establecer si la alarma esta activada
char aviso2[16] = " Mod. CLK"; //Aviso para indicar que se esta modificando el clock
char aviso3[16] = " Mod. ALM"; //Aviso para indicar que se esta modificando la alarma
char clock[16]; //Cadena para imprimir el paso del tiempo en el clock
char alarm[16]; //Cadena para imprimir la hora establecida en la alarma
char borrado[] = "               "; //Una linea entera de 15 espacios en blanco
uint8_t linea = 0; //Variable para situar la linea mediante la instruccion sprintf
uint8_t estado = 0; //Variable para almacenar el estado en que se halla el programa. Aprovecharemos el estado para el switch
uint8_t estado_anterior = 8; //Variable para almacenar el estado anterior. Debe ser diferente a 0 inicialmente
uint32_t retraso = 500; //antes era de 500000
uint32_t maxretard = 1050; //establecemos un limite superior para el retraso del parpadeo de los leds
uint16_t minretard = 100; //establecemos un limite inferior para el retraso del parpadeo de los leds
uint8_t posJ=0; //int para saber el estado para poder recorrer los leds de derecha a izquierda o a la inversa

//Inicialicación variables propias de esta practica
uint8_t modClkAlm = 0; //Nos sirve para conocer si se está modificando el clock o la alarma (segun el boton que hayamos pulsado)
uint8_t tempsClock = 0; //estado para saber si modificar hora,minuto o segundos en el clock. Por defecto estamos en segundos
uint8_t tempsAlarma = 0; //estado para saber si modificar hora,minuto o segundos en la alarma. Por defecto estamos en segundos
uint16_t milisegundos = 0; //int para controlar los milisegundos a la frecuencia de la fuente del clock seleccionada
uint8_t segundos = 0; //valor para los segundos del clock
uint8_t secAlarma = 0; //valor para los segundos de la alarma
uint8_t minutos = 0; //valor para los minutos del clock
uint8_t minAlarma = 0; //valor para los minutos de la alarma
uint8_t horas = 0; //valor para las horas del clock
uint8_t horaAlarma = 0; //valor para los horas de la alarma
uint8_t activeAlarma = 0; //estado para saber si la alarma ha sido modificada

//Inicializacion de funciones auxiliares que nos seran necesarias para la practica
void off_RGB();//Apagar los leds (estado 2 / Reset)
void knightRiderEffect(uint8_t est, uint32_t ret); //Necesitamos conocer el estado para determinar si se va hacia la derecha o a la izquierda
void init_Timers(); // Funcio per inicialitzar els timers
void addTime(); //Funcio que ens serveix per afegir temps (de 1 en 1) en hh/mm/ss
void subsTime(); //Funcio que ens serveix per restar temps (de 1 en 1) en hh/mm/ss
void reset(); //Funcio que ens serveix per setejar hh/mm/ss a 0

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

    //Controlem el controlador dinterrupcions
    NVIC -> ICPR[0] |= BIT8; //Nos aseguramos que no queda ninguna interrupcion residual pendiente para este puerto,
    NVIC -> ICPR[0] |= BITA; //y habilito las interrupciones del puerto

    NVIC -> ISER[0] |= BIT8; //interrupcio timer1 de lexercici 1
    NVIC -> ISER[0] |= BITA; //interrupcio timer2 de lexercici 2

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
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

      /*
       * La forma de tratar los GPIOs
       * 1- Configurar Pines como entradas o salidas -> PXDIR
       * 2- Configurar Pines como I/O digitales -> PXSEL0 y PXSEL1
       * 3- Activamos las interrupciones con transición low -> high -> PXIES
       * 4- Activamos las interrupciones -> PXIE
       * 5- Limpiamos los flags de las interrupciones del puerto x
       */
}

/**************************************************************************
 * INICIALIZACION DE LOS TIMERS
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_Timers(){
    //Habilitaremos las interrupciones a nivel de dispositivo
    TA0CCTL0 |= BIT4;
    TA1CCTL0 |= BIT4;

    //Seleccionamos la fuente de reloj SMCLK
    TA0CTL |= BIT9;
    TA1CTL |= BIT9;
    TA1CTL |= MC_1; //establecemos modo UP para el timer 1

    //Valor del comparador que utiliza el circuito del timer
    TA0CCR0 = 24000; // seteamos el tiempo en que se da la interrupcion (cada segundo)
    TA1CCR0 = 24000; // seteamos el tiempo a 1 milisegundo (24000000 Hz / 1000 = 24000)

}

/**************************************************************************
 * DELAY - A CONFIGURAR POR EL ALUMNO - con bucle while
 *
 * Datos de entrada: Tiempo de retraso. 1 segundo equivale a un retraso de 1000000 (aprox)
 *
 * Sin datos de salida
 *
 **************************************************************************/
void delay_t (uint32_t temps)
{
   volatile uint32_t i;
   i = temps;
   do {
       i--;
   }while (i != 0);
}

/**************************************************************************
 * APAGAR LOS LEDS
 *
 **************************************************************************/
void off_RGB(){
    //Posarem a 0 els leds RGB, es a dir els pins 6 del port 2 i 5 i el pin 4 del port 2.
    P2OUT &= ~0x50;
    P5OUT &= ~0x40;
}

/**************************************************************************
 * EFECTO DEL COCHE FANTÁSTICO CON LOS LEDS DE LA PLACA SECUNDARIA - NO LO USARE EN ESTA PRACTICA
 *
 **************************************************************************/
void knightRiderEffect(uint8_t est, uint32_t ret){

    delay_t(ret); //Esperamos un lapso pequeño para iniciar el cambio

    if(est == RIGHT){ //enciende los leds de izquierda a derecha
        for(P7OUT = 1; P7OUT < 128; P7OUT <<= 1){ //desplazamos un led
            delay_t(ret);
        }
        delay_t(ret);
        P7OUT = 0x00;
    }else if(est == LEFT){  //enciende los leds de derecha a izquierda
        for(P7OUT = 128; P7OUT > 0; P7OUT >>= 1){ //desplazamos un led
                delay_t(ret);
        }
        delay_t(ret);
        P7OUT = 0x00; //Apagamos los led
    }

}

/*****************************************************************************
 * CONFIGURACIÓN DEL PUERTO 7. A REALIZAR POR EL ALUMNO
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 ****************************************************************************/
void config_P7_LEDS (void)
{
    //Posem els registres a 0 per tant, els pins del port 7 funcionaran com a GPIO
    P7SEL0 = 0x00;//Configuraremos los pines del puerto 7 como pines de entrada/salida digitales
    P7SEL1 = 0x00;
    P7DIR = 0xFF; //Serán sortides
    P7OUT &= ~0xFF; //Apaguem els leds inicialment
}

/**************************************************************************
 * AÑADIR UNIDADES DE TIEMPO AL CLOCK
 *
 **************************************************************************/
void addTime(){
    switch(tempsClock){
        case 0:
            //una vez llega a 23, debemos hacer que vuelva a empezar
            horas = (horas + 1) % 24;
            break;
        case 1:
            //una vez llega a 59, debemos hacer que vuelva a empezar
            minutos = (minutos + 1) % 60;
            break;
        case 2:
            //una vez llega a 59, debemos hacer que vuelva a empezar
            segundos = (segundos + 1) % 60;
            break;
    }
}

/**************************************************************************
 * SUBSTRAER UNIDADES DE TIEMPO AL CLOCK
 *
 **************************************************************************/
void substTime(){
    switch(tempsClock){
    case 0:
        //Una vez se llega al 00 debemos hacer que el anterior sea 23
        horas = (horas + 23)%24;
        break;
    case 1:
        //Una vez se llega al 00 debemos hacer que el anterior sea 59
        minutos = (minutos + 59) % 60;
        break;
    case 2:
        //Una vez se llega al 00 debemos hacer que el anterior sea 59
        segundos = (segundos + 59) % 60;
        break;
    }
}

/**************************************************************************
 * RESETEAR TIEMPO AL CLOCK
 *
 **************************************************************************/
void reset(){
    //ponemos hh/mm/ss a 0
    horas = 0;
    minutos = 0;
    segundos = 0;
}

/**************************************************************************
 * AÑADIR UNIDADES DE TIEMPO A LA ALARMA
 *
 **************************************************************************/
void addTimeAlm(){
    switch(tempsAlarma){
        case 0:
            //una vez llega a 23, debemos hacer que vuelva a empezar
            horaAlarma = (horaAlarma + 1) % 24;
            break;
        case 1:
            //una vez llega a 59, debemos hacer que vuelva a empezar
            minAlarma = (minAlarma + 1) % 60;
            break;
        case 2:
            //una vez llega a 59, debemos hacer que vuelva a empezar
            secAlarma = (secAlarma + 1) % 60;
            break;
    }
}

/**************************************************************************
 * SUBSTRAER UNIDADES DE TIEMPO A LA ALARMA
 *
 **************************************************************************/
void substTimeAlm(){
    switch(tempsAlarma){
    case 0:
        //Una vez se llega al 00 debemos hacer que el anterior sea 23
        horaAlarma = (horaAlarma + 23)%24;
        break;
    case 1:
        //Una vez se llega al 00 debemos hacer que el anterior sea 59
        minAlarma = (minAlarma + 59) % 60;
        break;
    case 2:
        //Una vez se llega al 00 debemos hacer que el anterior sea 59
        secAlarma = (secAlarma + 59) % 60;
        break;
    }
}

/**************************************************************************
 * RESETAR TIEMPO A LA ALARMA
 *
 **************************************************************************/
void resetAlm(){
    //ponemos hh/mm/ss a 0
    horaAlarma = 0;
    minAlarma = 0;
    secAlarma = 0;
}

//funcion principal
void main(void)
{

    WDTCTL = WDTPW+WDTHOLD;         // Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_24MHz();       //Ajustes del clock (Unified Clock System)
    init_botons();         //Configuramos botones y leds
    init_interrupciones();  //Configurar y activar las interrupciones de los botones
    init_LCD();             // Inicializamos la pantalla
    init_Timers();              // Inicializamos los timers del micro

    //linea++;
    halLcdPrintLine(saludo,linea, INVERT_TEXT); //escribimos saludo en la primera linea

    config_P7_LEDS(); //apagamos los 7 leds de la placa secundaria

    //Bucle principal (infinito):
    do{
        escribir(lineas,linea +1);
        sprintf(clock," CLK: %02d:%02d:%02d", horas,minutos,segundos);
        escribir(clock,linea +2);
        sprintf(alarm," ALM: %02d:%02d:%02d", horaAlarma,minAlarma,secAlarma);
        escribir(alarm,linea +3);
        escribir(lineas,linea +4);

        if (estado_anterior != estado){         // Dependiendo del valor del estado se encendera un LED u otro.{
            sprintf(cadena," Estado %d", estado);    // Imprimimos el valor del estado
            escribir(cadena,linea +5);
            estado_anterior = estado;          // Actualizamos el valor de estado_anterior, para que no esta siempre escribiendo.
            sprintf(cadena," Retard %dms", retraso); //Imprimimos el valor del retraso actual
            escribir(cadena,linea +8);


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
                case S1:
                    off_RGB(); //Apagamos el led RGB
                    P2OUT |= 0x50;  //Encendemos el led R y G
                    P5OUT |= 0x40;  //Encendemos el led B
                    activeAlarma = 1; //Activamos la alarma
                    escribir(aviso1,linea +6);
                    break;

                case S2:
                    off_RGB(); //Apagamos el led RGB
                    activeAlarma = 0; //Desactivamos la alarma
                    escribir(borrado,linea +6);
                    break;

                case UP:
                    off_RGB(); //Apagamos el led RGB
                    P2OUT |= 0x40;  //Encendemos el led G
                    P5OUT |= 0x40;  //Encendemos el led B

                    if (modClkAlm == 0){ //Si estamos en modo clock
                        addTime(); //sumara 1 al valor de la variable estadoTemps en la que nos encontremos
                    }

                    if (modClkAlm == 1){ // Si estamos en modo alarma
                        addTimeAlm(); //sumara 1 al valor de la variable estadoTemps en la que nos encontremos
                    }

                    if(retraso < maxretard){  //establecemos un retraso maximos para evitar problemas
                        retraso += 100;  //aumentamos el retraso de 100 en 100(parpadeo + lento)
                    }
                    break;

                case DOWN:
                    off_RGB(); //Apagamos el led RGB
                    P2OUT ^= 0x10;  //Encendemos el led G
                    P5OUT ^= 0x40;  //Encendemos el led B

                    if (modClkAlm == 0){ //Si estamos en modo clock
                        substTime(); //restara 1 al valor de la variable estadoTemps en la que nos encontremos
                    }

                    if (modClkAlm == 1){ //Si estamos en modo alarma
                        substTimeAlm(); //restara 1 al valor de la variable estadoTemps en la que nos encontremos
                    }

                    if(retraso > minretard){  //si el retraso actual es mayor que minretard (forma para que no se apague el led)
                        retraso -= 100;  //acortamos el retraso de 100 en 100, vamos haciendo que sea mas lento
                    }
                    break;

                case LEFT:
                    off_RGB(); //Apagamos el led RGB
                    P2OUT |= 0x50;  //Encendemos el led R y G
                    P5OUT |= 0x40;  //Encendemos el led B
                    P7OUT |= 0x80;  //Encendemos el led de más a la derecha
                    modClkAlm = 0; //Ponemos el modo clock
                    escribir(aviso2,linea +7);
                    posJ = LEFT; //para la interrupcion del timer A0
                    delay_t(retraso);
                    //knightRiderEffect(estado,50500);
                    break;

                case RIGHT:
                    off_RGB(); //Apagamos el led RGB
                    P2OUT |= 0x50;  //Encendemos el led R y G
                    P7OUT |= 0x01;  //Encendemos el led mas a la izquierda
                    modClkAlm = 1; //ponemos el modo alarma
                    escribir(aviso3,linea +7);
                    posJ = RIGHT; //para la interrupcion del timer A1
                    delay_t(retraso);
                    //knightRiderEffect(estado,50500);
                    break;

                case CENTER:
                    P2OUT ^= 0x50;  //invertimos los leds del P2
                    P5OUT ^= 0x40;  //invertimos los leds del P5
                    //Dependiendo del modo en que estemos
                    if (modClkAlm == 0){
                        tempsClock = (tempsClock + 1) % 3; //cambiamos la variable a modificar del clock (hora, minutos o segundos)
                    }
                    if (modClkAlm == 1){
                        tempsAlarma = (tempsAlarma + 1) % 3; //cambiamos la variable a modificar de la alarma (hora, minutos o segundos)
                    }
                    break;
                }
            }

    }while(1); //Condicion para que el bucle sea infinito
}

/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectara que boton se ha pulsado
 *
 * Sin Datos de entrada
 *
 * Sin datos de salida
 *
 * Actualizar el valor de la variable global estado
 *
 **************************************************************************/

//Subrutina para las interrupciones del puerto 3:
void PORT3_IRQHandler(void){//interrupcion del pulsador S2

    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= 0xDF;  //interrupciones del boton S2 en port 3 desactivadas
    estado_anterior=0;

    switch(flag){
    case 0x0C: //pin 5
        estado = S2;
        break;
    }

    P3IE |= 0x20;   //interrupciones puerto 3 activadas
}

//Subrutina para las interrupciones del puerto 4:
void PORT4_IRQHandler(void){  //interrupcion de los botones. Actualiza el valor de la variable global estado.

    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= 0x5D;   //interrupciones Joystick en port 4 desactivadas
    estado_anterior=0;

    switch(flag){
            case 0x04: //pin 1
                estado = CENTER; //center
                break;
            case 0x0C: //pin 5
                estado = RIGHT; //right
                break;
            case 0x10: //pin 7
                estado = LEFT; //Left
                break;
            }

    P4IE |= 0xA2;   //interrupciones puerto 4 activadas
}

//Subrutina para las interrupciones del puerto 5:
void PORT5_IRQHandler(void){  //interrupci�n de los botones. Actualiza el valor de la variable global estado.

    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas
    estado_anterior=0;

    switch(flag){
            case 0x04: //pin 1
                estado = S1; //center
                break;
            case 0x0A:// pin 4
                estado = UP;
                break;
            case 0x0C: //pin 5
                estado = DOWN; //right
                break;
            }

    P5IE |= 0x32;   //interrupciones puerto 5 activadas
}

void TA0_0_IRQHandler (void){
    TA0CCTL0 &= ~CCIE; //Inhabilitem la interrupcio al principi

    if(posJ == RIGHT){ //leds de izquierda a derecha
        //desplazamos hacia la derecha
        knightRiderEffect(posJ,retraso);
    }else if(posJ == LEFT){  //leds de derecha a izquierda
        //desplazamos hacia a la izquierda
        knightRiderEffect(posJ,retraso);
    }

    TA0CCTL0 &= ~CCIFG; //limpiamos el flag de la interrupcion
    TA0CCTL0 |= CCIE; //habilitamos de nuevo la interrupcion del timer A0
}

//Controlamos aqui el paso del tiempo en el clock para que ademas nos sirva cuando salta la alarma
void TA1_0_IRQHandler (void){
    TA1CCTL0 &= ~CCIE; //Inhabilitem la interrupcio al principi
    milisegundos += 1;

    //Hacemos que los segundos vayan pasando
    if (milisegundos == 1000){
        milisegundos = 0;
        segundos+=1;
    }

    //Añadimos un minuto al contador de minutos y volvemos a empezar
    if (segundos == 60){
        minutos += 1;
        segundos = 0;
    }

    //hacemos lo mismo con las horas
    if (minutos == 60){
        horas ++;
        minutos = 0;
    }

    //por ultimo si hemos llegado al maximo de horas, volvemos a empezar un nuevo dia
    if (horas == 24){
        horas = 0;
    }

    //he contemplado la posiblilidad de hacer un metodo booleano para esto, pero he decidido simplificar el codigo
    //si la hora coincide con la alarma hacemos que salte el aviso
    if (segundos == secAlarma && minutos == minAlarma && horas == horaAlarma && activeAlarma == 1){
        sprintf(cadena," Ring ring!!"); //avisem quan es l'hora de l'alarma
        escribir(cadena,linea + 8);
        activeAlarma = 2; //Para hacer que el mensaje sea temporal
    }

    if (activeAlarma == 2 && (secAlarma + 7) % 60 == segundos){ //vamos a hacer que el mensaje dure unos 7 segundos i despues se borre
        borrar(linea + 8);
        activeAlarma = 1;
    }

    TA1CCTL0 &= ~CCIFG; //limpiamos el flag de la interrupcion
    TA1CCTL0 |= CCIE; //habilitamos de nuevo la interrupcion del timer A1
}
