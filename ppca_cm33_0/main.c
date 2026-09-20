/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for PPCA Core 0 (CM33 CPU0) of the
*              Advanced TCPWM Features example for ModusToolbox.
*
*              This core runs on PPCA CM33 CPU0 and is booted by the secure
*              main core (main_cm33_s) after all peripherals are ready.
*              It services two TCPWM interrupts:
*                - PWM A terminal-count ISR: continuously sweeps the frequency
*                  by decrementing the period register on every overflow event.
*                - PWM B CC1 ISR: performs a glitch-free duty-cycle toggle
*                  between 10% and 90% every 300 capture events.
*
* Related Document: See README.md
*
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
********************************************************************************/

#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>

/*******************************************************************************
* Macros
********************************************************************************/

/* Shared memory addresses in M4 shared memory space (0x20040000-0x20043FFF) */
/* All cores can access M4 shared memory for inter-core communication */
#define PPCA_CPU0_M4_VAR_ADDRESS 0x20040400  /* Used by this core (CPU0) */

/*******************************************************************************
* Global Variables
********************************************************************************/
/* The PWM A interrupt configuration structure */
const cy_stc_sysint_t pwm_a_intrCfg =
{
     .intrSrc = TCPWM_A_IRQ,
     .intrPriority = 1u
};

/* The PWM B interrupt configuration structure */
const cy_stc_sysint_t pwm_b_intrCfg =
{
     .intrSrc = TCPWM_B_IRQ,
     .intrPriority = 2u
};

/* Current period of PWM A (timer counts). Starts at the maximum value
 * (lowest frequency). The ISR decrements it on every terminal-count event
 * to sweep the frequency upward. Resets to 100,000,000 when it reaches
 * the minimum threshold of 10,000,000. */
volatile uint32_t period = 100000000;

/* Counter value of the TCPWM */
volatile uint32_t counter = 300;

/* Enumeration representing the two duty-cycle states for PWM B outputs */
enum duty_t {duty_10, duty_90};

/* Tracks the active duty-cycle state of PWM B (initialised at 10%) */
enum duty_t duty =  duty_10;

/*******************************************************************************
* Function Prototypes
********************************************************************************/

/* Interrupt handler for PWM A */
void pwm_a_intr_handler(void);

/* Interrupt handler for PWM B */
void pwm_b_intr_handler(void);
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Entry point for PPCA CM33 Core 0. Booted by the secure main core after all
*  shared peripherals have been initialized. Sets up and enables the NVIC
*  interrupts for PWM A and PWM B, then enters an infinite idle loop while
*  all PWM behavior is driven from the ISRs:
*    1. Registers and enables the PWM A terminal-count interrupt (frequency sweep).
*    2. Registers and enables the PWM B CC1 interrupt (duty-cycle toggle).
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_en_sysint_status_t status;

    /* Initialize the PWM A terminal-count interrupt (drives the frequency sweep) */
    status = Cy_SysInt_Init(&pwm_a_intrCfg, pwm_a_intr_handler);
    
    /* Interrupt initialization failed. */
    if(CY_SYSINT_SUCCESS != status)
    {
        CY_ASSERT(0);
    }

    /* Clear the pending interrupt */
    NVIC_ClearPendingIRQ(pwm_a_intrCfg.intrSrc);
    
    /* Enable the PWM A interrupt */
    NVIC_EnableIRQ(pwm_a_intrCfg.intrSrc);

    /* Initialize the PWM B CC1 interrupt (drives the duty-cycle toggle) */
    status = Cy_SysInt_Init(&pwm_b_intrCfg, pwm_b_intr_handler);
    
    /* Interrupt initialization failed. */
    if(CY_SYSINT_SUCCESS != status)
    {
        CY_ASSERT(0);
    }
    
    /* Clear any pending PWM B interrupt before enabling */
    NVIC_ClearPendingIRQ(pwm_b_intrCfg.intrSrc);

    /* Enable the PWM B interrupt */
    NVIC_EnableIRQ(pwm_b_intrCfg.intrSrc);

    /* Enable global interrupts so the ISRs can fire */
    __enable_irq();

     for(;;)
     {
         /* Idle loop — all PWM logic is handled in the ISRs above */
     }
}

/*******************************************************************************
* Function Name: pwm_a_intr_handler
********************************************************************************
* Summary:
* Interrupt service routine for PWM A
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void pwm_a_intr_handler(void)
{
    /* Read and clear the PWM A interrupt status flags */
    uint32_t interrupts = Cy_TCPWM_GetInterruptStatusMasked(TCPWM_A_HW, TCPWM_A_NUM);
    Cy_TCPWM_ClearInterrupt(TCPWM_A_HW, TCPWM_A_NUM, interrupts);

    /* Frequency sweep logic:
     * Decrement the period on every terminal-count event to increase the
     * output frequency step by step. When the period drops to the minimum
     * threshold (10,000,000 counts), reset it to the starting value
     * (100,000,000 counts) to restart the sweep from the lowest frequency. */
    if(period <= 10000000)
    {
        period = 100000000;  /* Reset to lowest frequency (longest period) */
    }
    period = period - 937500;  /* Decrement step per cycle (alt. step: 1875000) */

    /* Write the new period to the TCPWM hardware to change the PWM A frequency */
    Cy_TCPWM_PWM_SetPeriod1(TCPWM_A_HW, TCPWM_A_NUM, period);
}

/*******************************************************************************
* Function Name: pwm_b_intr_handler
********************************************************************************
* Summary:
* Interrupt service routine for PWM B
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void pwm_b_intr_handler(void)
{
    /* Read and clear the PWM B interrupt status flags */
    uint32_t interrupts = Cy_TCPWM_GetInterruptStatusMasked(TCPWM_B_HW, TCPWM_B_NUM);
    Cy_TCPWM_ClearInterrupt(TCPWM_B_HW, TCPWM_B_NUM, interrupts);

    /* Act only on the CC1 (Compare/Capture 1) event */
    if (0UL != (CY_TCPWM_INT_ON_CC1 & interrupts))
    {
        /* Countdown: toggle the duty cycle only every 300 CC1 events.
         * The counter is pre-decremented; the body executes only when it
         * reaches 0. It is then reloaded to 300 for the next cycle. */
        while(--counter == 0)
        {
            if(duty == duty_10)
            {
                /* Glitch-free immediate update: only apply the new compare
                 * value when the counter has already passed the current
                 * compare0 point, ensuring a clean transition with no
                 * output glitch on the current period. */
                if(Cy_TCPWM_Counter_GetCounter(TCPWM_B_HW, TCPWM_B_NUM) < Cy_TCPWM_PWM_GetCompare0Val(TCPWM_B_HW, TCPWM_B_NUM))
                {
                    /* compare0 = 100000 (10% of period=1,000,000) → 90% active duty */
                    Cy_TCPWM_PWM_SetCompare0Val(TCPWM_B_HW, TCPWM_B_NUM, 100000);
                    duty = duty_90;
                }
            }
            else
            {
                /* compare0 = 900000 (90% of period=1,000,000) → 10% active duty */
                Cy_TCPWM_PWM_SetCompare0Val(TCPWM_B_HW, TCPWM_B_NUM, 900000);
                duty = duty_10;
            }
            counter = 300;  /* Reload the countdown for the next toggle event */
        }
    }
}
