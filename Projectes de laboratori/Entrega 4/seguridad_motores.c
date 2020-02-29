byte TxPacket(byte bID, byte bParameterLength, byte bInstruction) {
    byte bCount,bCheckSum,bPacketLength;
    volatile int i = 50;
    char error[] = "adr. no permitida";
    if ((gbpParameter[0]<5)&&(bInstruction==3)){//si se intenta escribir en una direccion <= 0x05,
        //emitir mensaje de error de direccion prohibida:
          halLcdPrintLine( error,8,textstyle);
          //y salir de la funcion sin mas:
          return 0;
    }
//y el resto de la funcion sigue igual...
//...
//...
}