#include "RobotManipulation.h"

//En el main vamos a imprimir el menu y se va a dedicar a que a partir de la configuracion establecida
//por el usuario de llamen a las funciones que llaman a las capas inferiores

//Aqui se usaran los botones, por lo tanto deberemos mantener aqui los handlers de los botones

//Definimos los botones que vamos a utilizar
#define Button_S1 1
#define Button_S2 2
#define Jstick_Right 3
#define Jstick_Left 4
#define Jstick_Up 5
#define Jstick_Down 6
#define Jstick_Center 7

#define TXD2_READY (UCA2IFG & UCTXIFG)//Probar si esta linea sirve de algo

char configMsg[16] = " CONFIGURACION";//max 15 caracteres visibles
char cadena[16];
char izqMsg[16] = "IZQ";
char dchMsg[16] = "DCH";
char muteMsg[16] = "YES";
char noMuteMsg[16] = "NO";
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 0;
uint8_t estado = 0;
uint8_t estado_anterior = 8;
bool sentido = true;//establecemos el sentido de las ruedas por defecto hacia adelante
uint8_t menu = 0;
uint8_t selectedSubMenus = 0;
bool start = false;
bool aux = true;
bool aux2 = true;

void imprimirMenu(uint8_t);

void borrar(uint8_t Linea)
{
    halLcdPrintLine(borrado, Linea, NORMAL_TEXT); //escribimos una linea en blanco
}

void escribir(char String[], uint8_t Linea)

{
    halLcdPrintLine(String, Linea, NORMAL_TEXT); //Enviamos la String al LCD, sobreescribiendo la Linea indicada.
}

void imprimirMenu(uint8_t selected){
    uint8_t i = 2;

    for (i = 2; i < 10; i++){
        borrar(i);
    }
    linea = 2;

    switch(selected){
    case 0:
        sprintf(cadena, "VELOCIDAD %d", getSpeed());
        halLcdPrintLine(cadena,linea++, INVERT_TEXT);

        if(getWall() == 0){
            sprintf(cadena, "PARED %s", izqMsg);
        }else{
            sprintf(cadena, "PARED %s", dchMsg);
        }

        escribir(cadena,linea++);

        if(getSound()){
            sprintf(cadena, "SILENCIAR %s", muteMsg);
        }else{
            sprintf(cadena, "SILENCIAR %s", noMuteMsg);
        }

        escribir(cadena,linea++);

        sprintf(cadena, "EMPEZAR");
        escribir(cadena,linea++);
        break;

    case 1:
        sprintf(cadena, "VELOCIDAD %d", getSpeed());
        escribir(cadena,linea++);

        if(getWall() == 0){
            sprintf(cadena, "PARED %s", izqMsg);
        }else{
            sprintf(cadena, "PARED %s", dchMsg);
        }

        halLcdPrintLine(cadena,linea++,INVERT_TEXT);

        if(getSound()){
            sprintf(cadena, "SILENCIAR %s", muteMsg);
        }else{
            sprintf(cadena, "SILENCIAR %s", noMuteMsg);
        }

        escribir(cadena,linea++);
        sprintf(cadena, "EMPEZAR");
        escribir(cadena,linea++);
        break;

    case 2:
        sprintf(cadena, "VELOCIDAD %d", getSpeed());
        escribir(cadena,linea++);

        if(getWall() == 0){
            sprintf(cadena, "PARED %s", izqMsg);
        }else{
            sprintf(cadena, "PARED %s", dchMsg);
        }

        escribir(cadena,linea++);

        if(getSound()){
            sprintf(cadena, "SILENCIAR %s", muteMsg);
        }else{
            sprintf(cadena, "SILENCIAR %s", noMuteMsg);
        }

        halLcdPrintLine(cadena,linea++,INVERT_TEXT);
        sprintf(cadena, "EMPEZAR");
        escribir(cadena,linea++);
        break;

    case 3:
        sprintf(cadena, "VELOCIDAD %d", getSpeed());
        escribir(cadena,linea++);

        if(getWall() == 0){
            sprintf(cadena, "PARED %s", izqMsg);
        }else{
            sprintf(cadena, "PARED %s", dchMsg);
        }

        escribir(cadena,linea++);

        if(getSound()){
            sprintf(cadena, "SILENCIAR %s", muteMsg);
        }else{
            sprintf(cadena, "SILENCIAR %s", noMuteMsg);
        }

        escribir(cadena,linea++);
        sprintf(cadena, "EMPEZAR");
        halLcdPrintLine(cadena,linea++,INVERT_TEXT);
        break;
    }
}

void main(void){

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	init();
	halLcdPrintLine(configMsg,linea,NORMAL_TEXT); //escribimos saludo en la primera linea
    linea+=2;
    struct RxReturn estructura;
    uint8_t selected = 0;
    imprimirMenu(selected);


    musica();

	do{

	    estructura = leerSensor();

	    escribir("Sensor values",7);
        sprintf(cadena,"I%3d C%3d D%3d",estructura.StatusPacket[5],estructura.StatusPacket[6],estructura.StatusPacket[7]);
        escribir(cadena,8);

	    if(selected == 3 && start){
	       if(getSound() == false){
	           musica();
	           setSound(true);
	       }
	        robotRoute(estructura);
	    }

	    if(estado_anterior != estado){
	        estado_anterior = estado;

            switch(estado){
                case Button_S1:
                    if(selected == 1){
                        if(aux){
                            aux = false;
                            setWall(WALL_I);
                        }else{
                            aux = true;
                            setWall(WALL_D);
                        }
                    }

                    if(selected == 2){
                        if(aux2){
                            aux2 = false;
                            setSound(true);
                        }else{
                            aux2 = true;
                            setSound(false);
                        }
                    }

                    if(selected == 3){
                        start = true;
                    }
                    break;

                case Button_S2:
                    menu = 0;
                    selected = 0;
                    start = false;
                    if(!start){
                        setSound(false);
                        stopMovimientoRobot();
                        apagarLedsMotor();
                    }
                    break;

                case Jstick_Up:
                    if(selected > 0) selected --;
                    break;

                case Jstick_Down:
                    if (selected < 3) selected ++;
                    break;


	    }
            imprimirMenu(selected);
	}
	}while(1);
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
    /***********************************************
     * HASTA AQUI BLOQUE CASE
     ***********************************************/

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}
