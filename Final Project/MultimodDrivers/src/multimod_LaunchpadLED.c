// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_LaunchpadLED.h"

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/pin_map.h>
#include <driverlib/pwm.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// LaunchpadButtons_Init
// Initializes the GPIO port & pins necessary for the button switches on the
// launchpad. Also configures it so that the LEDs are controlled via PWM signal.
// Initial default period of 400.
// Return: void
void LaunchpadLED_Init() {
    // Enable clock to port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Enable PWM module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    // Configure necessary pins as PWM
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinConfigure(GPIO_PF1_M1PWM5);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2,PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3,PWM_GEN_MODE_NO_SYNC);
    // Set the default pulse width (duty cycles).
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, PWM_Per);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, PWM_Per);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, PWM_Per);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, PWM_Per);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, PWM_Per);
    // Enable the PWM generators
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);
    // Enable PWM output
    PWMOutputState(PWM1_BASE, (PWM_OUT_5_BIT|PWM_OUT_6_BIT|PWM_OUT_7_BIT), true);
}

// LaunchpadLED_PWMSetDuty
// Sets the duty cycle of the PWM generator associated with the LED.
// Return: void
void LaunchpadLED_PWMSetDuty(LED_Color_t LED, float duty) {
    uint32_t pulse_width = (duty * PWM_Per);

    // If pulse width < 1, set as 1
    if (pulse_width < 1) {
        pulse_width = 1;
    }

    // If pulse width > set period, cap it
    if (pulse_width > PWM_Per) {
        pulse_width = PWM_Per - 1;
    }

    // Depending on chosen LED(s), adjust corresponding duty cycle of the PWM output
    // Your code below.

    uint32_t color;
    if (LED == GPIO_PIN_1)
        color =PWM_OUT_5;
    else if (LED == GPIO_PIN_2)
        color = PWM_OUT_6;
    else
        color = PWM_OUT_7;
    PWMPulseWidthSet(PWM1_BASE, color, pulse_width);


    return;
}

/********************************Public Functions***********************************/
