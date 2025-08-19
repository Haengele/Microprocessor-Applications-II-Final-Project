// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"
#include "./MiscFunctions/Shapes/inc/cube.h"
#include "./MiscFunctions/LinAlg/inc/linalg.h"
#include "./MiscFunctions/LinAlg/inc/quaternions.h"
#include "./MiscFunctions/LinAlg/inc/vect3d.h"
#include "./driverlib/interrupt.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


/*********************************Global Variables**********************************/

float_t curs_x = 50;
float_t curs_y = 50;

float_t prev_curs_x = 50;
float_t prev_curs_y = 50;

uint16_t sel_x[3];
uint16_t sel_y[3];
uint16_t sel_ind = 0;
bool sel_clear = false;
float_t sel_ang;

//uint16_t image[240][280];
Pixel_Packet_t packet;

Pixel_t res_on_clr[1800];
uint16_t clrInd = 0;
Pixel_t res_on_mv[150];
uint8_t mvInd = 0;
uint16_t mvX[150];
uint16_t mvY[150];
uint8_t tmpMvInd = 0;


int16_t lev_x;
int16_t lev_y;
int16_t lev_z;
bool zdown = true;

char prevLev[15];


bool cleared = true;


bool menu = true;
bool level = false;
bool pict = false;


/*********************************Global Variables**********************************/

/*************************************Threads***************************************/

void Idle_Thread(void) {
    time_t t;
    srand((unsigned) time(&t));
    while(1);
}



void UpdateCurs_Thread(void) {
    // Initialize / declare any variables here
    int16_t joyY = 0;
    int16_t joyX = 0;
    float fx = 0.0;
    float fy = 0.0;
    while(1) {
        // Get result from joystick
        joyX = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
        joyY = G8RTOS_ReadFIFO(JOYSTICK_FIFO);

        // If joystick axis within deadzone, set to 0. Otherwise normalize it.

        if(abs(joyX) < 256)
            fx = 0;
        else
           fx = ((float)joyX)/(float)512;
        if(abs(joyY) < 256)
            fy = 0;
        else
            fy = ((float)joyY)/(float)512;

        if(curs_x - fx >= 15 && curs_x - fx < 225)
            curs_x -= fx;
        if(curs_y + fy >= 15 && curs_y + fy < 265)
            curs_y += fy;


        // sleep
        sleep(10);
    }
}



void Menu_Thread() {
    // Initialize / declare any variables here
    uint8_t buttons;
    char text[40];
    while(1) {

        G8RTOS_WaitSemaphore(&sem_Menu);
        sleep(100);

        sprintf(text, "SW1: Level Function");
        ST7789_DrawText(&FontStyle_Emulogic, text, 45, 180, ST7789_WHITE, ST7789_BLACK);
        sprintf(text, "SW2: Picture Function");
        ST7789_DrawText(&FontStyle_Emulogic, text, 40, 140, ST7789_WHITE, ST7789_BLACK);

        sprintf(text, "SW1 will return to menu");
        ST7789_DrawText(&FontStyle_Emulogic, text, 25, 80, ST7789_WHITE, ST7789_BLACK);
        sprintf(text, "from function");
        ST7789_DrawText(&FontStyle_Emulogic, text, 70, 65, ST7789_WHITE, ST7789_BLACK);


        // Wait for a signal to read the buttons on the Multimod board.
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);

        // Sleep to debounce
        sleep(10);
        // Read the buttons status on the Multimod board.
        buttons = ~(MultimodButtons_Get());
        // Process the buttons and determine what actions need to be performed.
        if(buttons & SW1)
        {
            while(~(MultimodButtons_Get()) & SW1)
                sleep(5);
            ST7789_Fill(ST7789_BLACK);
            menu = false;
            level = true;
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            G8RTOS_SignalSemaphore(&sem_Lev);
            G8RTOS_SignalSemaphore(&sem_Lev);
            G8RTOS_WaitSemaphore(&sem_Menu);
        }
        else if(buttons & SW2)
        {
            while(~(MultimodButtons_Get()) & SW2)
                sleep(5);
            ST7789_Fill(ST7789_BLACK);
            menu = false;
            pict = true;
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            G8RTOS_SignalSemaphore(&sem_Pic);
            G8RTOS_WaitSemaphore(&sem_Menu);
        }
        else if(buttons & SW3)
        {
            while(~(MultimodButtons_Get()) & SW3)
                sleep(5);
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else if(buttons & SW4)
        {
            while(~(MultimodButtons_Get()) & SW4)
                sleep(5);
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else
        {
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        G8RTOS_SignalSemaphore(&sem_Menu);

    }
}

void Level_Thread(void)
{
    uint8_t buttons;
    while(1) {

        G8RTOS_WaitSemaphore(&sem_Lev);
        UARTprintf("Now in Level Process\n");
        // Wait for a signal to read the buttons on the Multimod board.
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);

        // Sleep to debounce
        sleep(10);
        // Read the buttons status on the Multimod board.
        buttons = ~(MultimodButtons_Get());
        // Process the buttons and determine what actions need to be performed.
        if(buttons & SW1)
        {
            while(~(MultimodButtons_Get()) & SW1)
                sleep(5);
            menu = true;
            level = false;
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            G8RTOS_SignalSemaphore(&sem_Menu);
            G8RTOS_WaitSemaphore(&sem_Lev);
            G8RTOS_WaitSemaphore(&sem_Lev);
        }
        else if(buttons & SW2)
        {
            while(~(MultimodButtons_Get()) & SW2)
                sleep(5);
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else if(buttons & SW3)
        {
            while(~(MultimodButtons_Get()) & SW3)
                sleep(5);
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else if(buttons & SW4)
        {
            while(~(MultimodButtons_Get()) & SW4)
                sleep(5);

            zdown = !zdown;
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else
        {
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }

        G8RTOS_SignalSemaphore(&sem_Lev);

    }
}

void read_acc()
{
    while(true)
    {
        G8RTOS_WaitSemaphore(&sem_Lev);

        G8RTOS_WaitSemaphore(&sem_I2CA);
        lev_x = BMI160_AccelXGetResult();
        lev_y = BMI160_AccelYGetResult();
        lev_z = BMI160_AccelZGetResult();
        G8RTOS_SignalSemaphore(&sem_I2CA);

        G8RTOS_SignalSemaphore(&sem_Lev);
        sleep(50);
    }
}



void Picture_Thread(void)
{
    uint8_t buttons;
    uint32_t IBit_State;
    while(1) {

        G8RTOS_WaitSemaphore(&sem_Draw);
        UARTprintf("Now in Picture Process\n");
        // Wait for a signal to read the buttons on the Multimod board.
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);

        // Sleep to debounce
        sleep(10);
        // Read the buttons status on the Multimod board.
        buttons = ~(MultimodButtons_Get());
        // Process the buttons and determine what actions need to be performed.
        if(buttons & SW1)
        {
            while(~(MultimodButtons_Get()) & SW1)
                sleep(5);

            uint8_t x1 = 255 >> 8;
            uint8_t x2 = 255 & 0xFF;

            uint8_t y1 = 255 >> 8;
            uint8_t y2 = 255 & 0xFF;


            IBit_State = StartCriticalSection();

            UARTCharPut(UART4_BASE, x1+1);
            UARTCharPut(UART4_BASE, x2);
            UARTCharPut(UART4_BASE, y1);
            UARTCharPut(UART4_BASE, y2);

            EndCriticalSection(IBit_State);


            menu = true;
            pict = false;
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            G8RTOS_SignalSemaphore(&sem_Menu);
            G8RTOS_WaitSemaphore(&sem_Draw);
        }
        else if(buttons & SW2)
        {
            while(~(MultimodButtons_Get()) & SW2)
                sleep(5);
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else if(buttons & SW3)
        {
            while(~(MultimodButtons_Get()) & SW3)
                sleep(5);
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else if(buttons & SW4)
        {
            while(~(MultimodButtons_Get()) & SW4)
                sleep(5);
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        else
        {
            // Clear the interrupt
            GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            // Re-enable the interrupt so it can occur again.
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        }
        G8RTOS_SignalSemaphore(&sem_Draw);

    }
}




void Read_JoystickPress() {
    // Initialize / declare any variables here
    uint8_t press;
    Pixel_t pix;
    while(1) {
        // Wait for a signal to read the joystick press
        G8RTOS_WaitSemaphore(&sem_Joystick_Debounce);
        if(pict)
        {
            // Sleep to debounce
            sleep(10);

            // Read the joystick switch status on the Multimod board.
            press = JOYSTICK_GetPress();

            if(press)
            {
                if(sel_ind < 3)
                {
                    sel_x[sel_ind] = (uint16_t)curs_x;
                    sel_y[sel_ind] = (uint16_t)curs_y;

                    for(uint16_t xi = sel_x[sel_ind] - 2; xi <= sel_x[sel_ind]+2; xi++)
                    {
                        for(uint16_t yi = sel_y[sel_ind] - 2; yi <= sel_y[sel_ind]+2; yi++)
                        {
                            pix.col = getPix(xi, yi);
                            pix.x = xi;
                            pix.y = yi;
                            res_on_clr[clrInd] = pix;
                            clrInd++;
                        }
                    }
                    sel_ind++;

                    if(sel_ind == 3)
                    {
                        G8RTOS_AddThread(Calc_Ang, 3, "calc");
                        Get_Line(sel_x[0], sel_y[0], sel_x[1], sel_y[1]);
                        Get_Line(sel_x[1], sel_y[1], sel_x[2], sel_y[2]);

                    }

                }
                else
                {
                    sel_clear = true;
                }
            }
        }
        // Clear the interrupt
        GPIOIntClear(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
        // Re-enable the interrupt so it can occur again.
        GPIOIntEnable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
    
    }
}

void Calc_Ang(void)
{
    float_t ang1, ang2;

    ang1 = 180*(atan2((float)(sel_y[1]-sel_y[0]), (float)(sel_x[1]-sel_x[0])))/3.1415926535;
    ang2 = 180*(atan2((float)(sel_y[1]-sel_y[2]), (float)(sel_x[1]-sel_x[2])))/3.1415926535;
    sel_ang = ang1-ang2;

    G8RTOS_KillSelf();
    while(true);

}

void UART_Thread()
{
    uint32_t IBit_State;
    //char str[5];
    //uint16_t ind = 0;
    //IntEnable(INT_UART4);
    //uint8_t red, green, blue;
    uint16_t col;
    uint8_t* ptr = (uint8_t*) &packet;
    uint8_t cnt = 0;
    while(1){
        G8RTOS_WaitSemaphore(&sem_Pic);
        IBit_State = StartCriticalSection();

        char text[25];
        sprintf(text, "Waiting for image...");
        ST7789_DrawText(&FontStyle_Emulogic, text, 40, 140, ST7789_WHITE, ST7789_BLACK);

        for (int y = 279; y >= 0; y--)
        {
            for(int x = 0; (x < 240) /*&& !(y==0 && x==239)*/; x++)
            {
                /*while (!UARTCharsAvail(UART4_BASE));
                // Read a character from the UART4 module and return it
                red = UARTCharGet(UART4_BASE);
                //UARTIntClear(UART4_BASE, UART_INT_RX);
                while (!UARTCharsAvail(UART4_BASE));
                // Read a character from the UART4 module and return it
                green = UARTCharGet(UART4_BASE);
                //UARTIntClear(UART4_BASE, UART_INT_RX);
                while (!UARTCharsAvail(UART4_BASE));
                // Read a character from the UART4 module and return it
                blue = UARTCharGet(UART4_BASE);
                //UARTIntClear(UART4_BASE, UART_INT_RX);*/


               ptr = (uint8_t*) &packet;
               char c;
               //uint32_t ui32Status = UARTIntStatus(UART4_BASE, true); // Get interrupt status
               while (!UARTCharsAvail(UART4_BASE));
               while(UARTCharsAvail(UART4_BASE) || cnt < 3)
               {
                   c = UARTCharGet(UART4_BASE);
                   *(ptr++) = c;
                   if(!UARTCharsAvail(UART4_BASE))
                   {
                       cnt++;
                   }
               }
               cnt = 0;


                col = ((packet.r & 0xF8) << 8) | ((packet.g & 0xFC) << 3) | (packet.b >> 3);

                ST7789_DrawPixel(x, y, col);
            }
        }
        EndCriticalSection(IBit_State);
        G8RTOS_SignalSemaphore(&sem_Draw);



    }
}


void Ret_Pixels(void)
{
    Pixel_t pix;
    uint32_t IBit_State;
    while(true)
    {
        if(pict)
        {
            for(uint8_t i = 0; i < tmpMvInd && mvInd < 50; i++)
            {
                IBit_State = StartCriticalSection();
                pix.col = getPix(mvX[i], mvY[i]);
                pix.x = mvX[i];
                pix.y = mvY[i];
                res_on_mv[mvInd] = pix;
                // Check if tmpMvInd has been updated. If so restart.
                if(mvInd == 0 && i != 0)
                {
                    EndCriticalSection(IBit_State);
                    break;
                }
                mvInd++;
                EndCriticalSection(IBit_State);
            }
            sleep(10);
        }
        else
        {
            sleep(100);
        }
    }
}

void Get_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    Pixel_t pix;
    bool steep = abs((int16_t)(y1 - y0)) > abs((int16_t)(x1 - x0));
    if (steep)
    {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1)
    {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs((int16_t)(y1 - y0));

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (; x0 <= x1; x0++)
    {
        if (steep)
        {
            //ST7789_DrawPixel(y0, x0, color);
            pix.col = getPix(y0, x0);
            pix.x = y0;
            pix.y = x0;
        }
        else
        {
            //ST7789_DrawPixel(x0, y0, color);
            pix.col = getPix(x0, y0);
            pix.x = x0;
            pix.y = y0;
        }
        res_on_clr[clrInd] = pix;
        clrInd++;
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}


/********************************Periodic Threads***********************************/

void Draw_Lev(void)
{

    if(level)
    {
        cleared = false;
        int16_t opp, adj;
        if(zdown)
        {
            opp = sqrt(lev_x*lev_x + lev_y*lev_y);
            adj = lev_z;
        }
        else
        {
            opp = sqrt(lev_x*lev_x + lev_z*lev_z);
            adj = lev_y;
        }
        float_t ang = 180*(atan2(opp, adj))/3.1415926535;
        int16_t whole = (int16_t)ang;
        int16_t dec = abs((abs((int32_t)(ang*100))) - abs((((int32_t)whole)*100)));
        //UARTprintf("Angle: %d.%d\n", whole, dec);
        char text[15];
        sprintf(text, "Angle: %d.%d\n", whole, dec);
        ST7789_DrawText(&FontStyle_Emulogic, prevLev, 80, 140, ST7789_BLACK, ST7789_BLACK);
        ST7789_DrawText(&FontStyle_Emulogic, text, 80, 140, ST7789_WHITE, ST7789_BLACK);
        strcpy(prevLev, text);

    }
    else
    {
        if(!cleared && !pict)
        {
            sel_ind = 0;
            mvInd = 0;
            clrInd = 0;
            ST7789_Fill(ST7789_BLACK);
            cleared = true;
        }
    }
}





void draw_Curs_Help(uint16_t x, uint16_t y, uint16_t color)
{
    ST7789_DrawRectangle(x-1, y+3, 3, 10, color);
    for(uint16_t xi = x-1; xi <=x+1; xi++)
    {
        for(uint16_t yi = y+3; yi <=y+13; yi++)
        {
            //pix.col = getPix(xi, yi);
            //pix.x = xi;
            //pix.y = yi;
            //res_on_mv[mvInd] = pix;
            //mvInd++;
            mvX[tmpMvInd] = xi;
            mvY[tmpMvInd] = yi;
            tmpMvInd++;
        }
    }
    ST7789_DrawRectangle(x-1, y-13, 3, 10, color);
    for(uint16_t xi = x-1; xi <=x+1; xi++)
    {
        for(uint16_t yi = y-13; yi <=y-3; yi++)
        {
            mvX[tmpMvInd] = xi;
            mvY[tmpMvInd] = yi;
            tmpMvInd++;
        }
    }
    ST7789_DrawRectangle(x+3, y-1, 10, 3, color);
    for(uint16_t xi=x+3; xi<=x+13; xi++)
    {
        for(uint16_t yi = y-1; yi <=y+1; yi++)
        {
            mvX[tmpMvInd] = xi;
            mvY[tmpMvInd] = yi;
            tmpMvInd++;
        }
    }
    ST7789_DrawRectangle(x-13, y-1, 10, 3, color);
    for(uint16_t xi=x-13; xi<=x-3; xi++)
    {
        for(uint16_t yi = y-1; yi <=y+1; yi++)
        {
            mvX[tmpMvInd] = xi;
            mvY[tmpMvInd] = yi;
            tmpMvInd++;
        }
    }

}

void mvRes()
{
    Pixel_t pix;
    for(uint8_t j = mvInd; j < tmpMvInd; j++)
    {
        pix.col = getPix(mvX[j], mvY[j]);
        pix.x = mvX[j];
        pix.y = mvY[j];
        res_on_mv[mvInd] = pix;
        mvInd++;
    }

    for(uint16_t i = 0; i < mvInd; i++)
    {
        ST7789_DrawPixel(res_on_mv[i].x, res_on_mv[i].y, res_on_mv[i].col);
    }
    mvInd = 0;
    tmpMvInd = 0;
}

uint16_t getPix(uint16_t x, uint16_t y)
{
    y = 280 - y;
    while(UARTCharsAvail(UART4_BASE))
    {
        UARTCharGet(UART4_BASE);
    }
    uint8_t x1 = x >> 8;
    uint8_t x2 = x & 0xFF;

    uint8_t y1 = y >> 8;
    uint8_t y2 = y & 0xFF;

    uint32_t IBit_State = StartCriticalSection();

    UARTCharPut(UART4_BASE, x1+1);
    UARTCharPut(UART4_BASE, x2);
    UARTCharPut(UART4_BASE, y1);
    UARTCharPut(UART4_BASE, y2);

    uint8_t cnt = 0;
    uint8_t* ptr = (uint8_t*) &packet;
    char c;
    //uint32_t ui32Status = UARTIntStatus(UART4_BASE, true); // Get interrupt status
    while (!UARTCharsAvail(UART4_BASE));
    while(UARTCharsAvail(UART4_BASE) || cnt < 3)
    {
        c = UARTCharGet(UART4_BASE);
        *(ptr++) = c;
        if(!UARTCharsAvail(UART4_BASE))
        {
            cnt++;
        }
    }
    EndCriticalSection(IBit_State);
    uint16_t col = ((packet.r & 0xF8) << 8) | ((packet.g & 0xFC) << 3) | (packet.b >> 3);
    return col;
}


void Draw_Curs()
{
    if(pict)
    {
        cleared = false;
        mvRes();
        draw_Curs_Help((uint16_t)curs_x, (uint16_t)curs_y, ST7789_WHITE);
        prev_curs_x = curs_x;
        prev_curs_y = curs_y;

        if(sel_ind > 2)
        {
            ST7789_DrawLine(sel_x[0], sel_y[0], sel_x[1], sel_y[1], ST7789_BLUE);
            ST7789_DrawLine(sel_x[1], sel_y[1], sel_x[2], sel_y[2], ST7789_BLUE);

            char text[10];
            int32_t intang = (int32_t)sel_ang;
            sprintf(text, "%d", intang);
            ST7789_DrawText(&FontStyle_Emulogic, text, 200, 240, ST7789_WHITE, ST7789_BLACK);


        }

        for(int i = 0; i < sel_ind; i++)
        {
            ST7789_DrawRectangle(sel_x[i]-2, sel_y[i]-2, 5, 5, ST7789_RED);
        }


        if(sel_clear)
        {
            for(int i = 0; i < clrInd; i++)
            {
                ST7789_DrawPixel(res_on_clr[i].x, res_on_clr[i].y, res_on_clr[i].col);
            }
            clrInd = 0;
            char text[10];
            int32_t intang = (int32_t)sel_ang;
            sprintf(text, "%d", intang);
            ST7789_DrawText(&FontStyle_Emulogic, text, 200, 240, ST7789_BLACK, ST7789_BLACK);
            sel_ind = 0;
            sel_clear = false;
            sel_ang = -1.0;
        }
    }
    else
    {
        if(!cleared && !level)
        {
            sel_ind = 0;
            mvInd = 0;
            clrInd = 0;
            ST7789_Fill(ST7789_BLACK);
            cleared = true;
        }
    }
}

void Get_Joystick(void) {
    // Read the joystick

    if(pict)
    {
        uint16_t X = JOYSTICK_GetY();
        uint16_t Y = JOYSTICK_GetX();
        // Send through FIFO.
        G8RTOS_WriteFIFO(JOYSTICK_FIFO, ((int32_t)X)-2048);
        G8RTOS_WriteFIFO(JOYSTICK_FIFO, ((int32_t)Y)-2048);
    }
}



/*******************************Aperiodic Threads***********************************/

void GPIOE_Handler() {
    // Disable interrupt
    GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);

}

void GPIOD_Handler() {
    // Disable interrupt
    GPIOIntDisable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_Joystick_Debounce);
}
