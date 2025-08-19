// threads.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Threads

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include "./G8RTOS/G8RTOS.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define SPAWNCOOR_FIFO      0
#define JOYSTICK_FIFO       1

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) \
{  \
    int16_t t = a; \
    a = b; \
    b = t; \
}
#endif

/*************************************Defines***************************************/

/***********************************Semaphores**************************************/

semaphore_t sem_I2CA;
semaphore_t sem_SPIA;
semaphore_t sem_PCA9555_Debounce;
semaphore_t sem_Joystick_Debounce;

semaphore_t sem_Menu;
semaphore_t sem_Lev;
semaphore_t sem_Pic;
semaphore_t sem_Draw;
semaphore_t sem_UART;


/***********************************Semaphores**************************************/

/***********************************Structures**************************************/

typedef struct __attribute__((__packed__)) Pixel_Packet {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    //uint8_t garbage;
    //bool last;
} Pixel_Packet_t;

typedef struct __attribute__((__packed__)) Pixel {
    uint16_t col;
    uint8_t x;
    uint8_t y;
} Pixel_t;


/***********************************Structures**************************************/


/*******************************Background Threads**********************************/

void Idle_Thread(void);
//void Read_Buttons(void);
void Read_JoystickPress(void);

void Menu_Thread(void);
void Level_Thread(void);
void Picture_Thread(void);
void read_acc(void);

void UpdateCurs_Thread(void);

void Calc_Ang(void);
void UART_Thread(void);

void Ret_Pixels(void);

/*******************************Background Threads**********************************/

/********************************Periodic Threads***********************************/

void Get_Joystick(void);
void Draw_Curs(void);
void Draw_Lev(void);

/********************************Periodic Threads***********************************/

/*******************************Aperiodic Threads***********************************/

void GPIOE_Handler(void);
void GPIOD_Handler(void);

/*******************************Aperiodic Threads***********************************/

void draw_Curs_Help(uint16_t x, uint16_t y, uint16_t color);
uint16_t getPix(uint16_t x, uint16_t y);
void mvRes(void);
void Get_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif /* THREADS_H_ */

