// Lab 4, uP2 Fall 2023
// Created: 2023-07-31
// Updated: 2023-08-01
// Lab 4 is intended to introduce you to more advanced RTOS concepts. In this, you will
// - implement blocking, yielding, sleeping
// - Thread priorities, aperiodic & periodic threads
// - IPC using FIFOs
// - Dynamic thread creation & deletion

/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"
#include <stdio.h>
#include <stdlib.h>

#include "./threads.h"

semaphore_t s;


void multimod_init();

/************************************MAIN*******************************************/
int main(void)
{
    // Sets clock speed to 80 MHz. You'll need it!
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    multimod_init();
    G8RTOS_Init();

    G8RTOS_AddThread(Idle_Thread, 254, "idle");
    G8RTOS_AddThread(Menu_Thread, 7, "but");
    G8RTOS_AddThread(Level_Thread, 8, "lev");
    G8RTOS_AddThread(Picture_Thread, 9, "pic");
    G8RTOS_AddThread(UpdateCurs_Thread, 5, "joy");
    G8RTOS_AddThread(Read_JoystickPress, 6, "prs");
    G8RTOS_AddThread(UART_Thread, 2, "uart");
    //G8RTOS_AddThread(Calc_Ang, 3, "calc");
    G8RTOS_AddThread(read_acc, 5, "acc");
    //G8RTOS_AddThread(Ret_Pixels, 10, "pix");


    G8RTOS_Add_PeriodicEvent(Get_Joystick, 50, 30);
    G8RTOS_Add_PeriodicEvent(Draw_Curs, 50, 25);
    G8RTOS_Add_PeriodicEvent(Draw_Lev, 50, 20);
    //G8RTOS_Add_PeriodicEvent(Draw_Board, 100, 75);


    G8RTOS_InitSemaphore(&sem_I2CA, 1);
    G8RTOS_InitSemaphore(&sem_SPIA, 1);
    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_Joystick_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_KillCube, 0);

    G8RTOS_InitSemaphore(&sem_Menu, 1);
    G8RTOS_InitSemaphore(&sem_Lev, 0);
    G8RTOS_InitSemaphore(&sem_Pic, 0);
    G8RTOS_InitSemaphore(&sem_Calc, 0);
    G8RTOS_InitSemaphore(&sem_UART, 1);
    G8RTOS_InitSemaphore(&sem_Draw, 0);

    G8RTOS_InitFIFO(SPAWNCOOR_FIFO);
    G8RTOS_InitFIFO(JOYSTICK_FIFO);

    G8RTOS_Add_APeriodicEvent(GPIOE_Handler, 4, INT_GPIOE);
    G8RTOS_Add_APeriodicEvent(GPIOD_Handler, 5, INT_GPIOD);

    //G8RTOS_AddThread(testThread, 1, "test");

    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/

void multimod_init() {
    ST7789_Init();
    BMI160_Init();
    OPT3001_Init();
    PCA9956b_Init();
    UART_Init();
    UART_BeagleBone_Init();

    MultimodButtons_Init();
    JOYSTICK_Init();
    JOYSTICK_IntEnable();
}

