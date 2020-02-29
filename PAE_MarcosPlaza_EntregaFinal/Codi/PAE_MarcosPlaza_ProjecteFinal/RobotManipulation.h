#ifndef ROBOTMANIPULATION_H_
#define ROBOTMANIPULATION_H_
#include "RobotFunctionalities.h"

#define WALL_I 0 // Creo que estas constantes sobran un poquito
#define WALL_D 1
#define MAXDIST 35 //Cambiar posteriormente
#define MINDIST 30

uint16_t speed;
bool muteSound;
uint8_t wall;
bool direction;

uint16_t getSpeed();
bool getSound();
uint8_t getWall();
bool getDirection();

void setSpeed(uint16_t);
void setSound(bool);
void setWall(uint8_t);
void setDirection(bool);
void setForward(bool);

void init();

void robotRoute(struct RxReturn structure);

void musica(void);

#endif /* ROBOTMANIPULATION_H_ */
