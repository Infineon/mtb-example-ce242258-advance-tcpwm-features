/*****************************************************************************
* File Name        : main.c
*
* Description      : This is the source file for the secure main CM33 core
*                    (main_cm33_s) of the Advanced TCPWM Features example.
*
*                    This core initializes the peripherals (UART, TCPWM A/B/C,
*                    PPCA, EPU), boots the two PPCA cores (CPU0/CPU1),
*                    and then idles. PPCA Core 0 drives the PWM A/B interrupts;
*                    TCPWM C demonstrates the debug freeze/suspend feature.
*
* Related Document : See README.md
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


/******************************************************************************
 * Header Files
 *****************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/* These are the addresses where the core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START
#define CORE1_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START

#define PPCA0_IMAGE_SIZE       CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE       CYMEM_CM33_0_S_ppca1_code_SIZE

/* Shared memory addresses for inter-core communication */
/* PPCA cores write to these locations, main core reads from them */
/* Variables located in M4 shared memory space (16KB at 0x20040000-0x20043FFF from PPCA view) */
/* Main core accesses PPCA memory through PPCA peripheral base with memory windows: */
/* M1 (CPU0 data): 0x53020000, M3 (CPU1 data): 0x53040000, M4 (shared): 0x53050000 */
#define PPCA_CPU0_M4_VAR_ADDRESS   0x53050400  /* Written by PPCA Core 0 */
#define PPCA_CPU1_M4_VAR_ADDRESS   0x53050800  /* Written by PPCA Core 1 */


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* Debug UART variables */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug UART HAL object */

/*******************************************************************************
* Function Prototypes
*******************************************************************************/


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Entry point for the secure CM33 main core. Performs the following in order:
*    1. Initializes board peripherals (cybsp_init) and the debug UART.
*    2. Enables global interrupts; initializes the PPCA block and EPU.
*    3. Boots PPCA Core 0 (PWM A/B ISR handler) and Core 1 from flash.
*    4. Initializes and starts TCPWM A (frequency sweep), B (duty-cycle toggle),
*       and C (debug freeze/suspend demo), then idles in an infinite loop.
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
    cy_rslt_t result;
    cy_en_tcpwm_status_t status;
    cy_en_scb_uart_status_t init_status;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the SCB UART block with the generated configuration */
    init_status = Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    /* UART initialization failed. Stop program execution */
    if (init_status!=CY_SCB_UART_SUCCESS)
    {
         CY_ASSERT(0);
    }
    
    /* Enable UART */
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize redirecting of low level IO */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* retarget IO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    
    /* Transmit header to the terminal */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: Advance TCPWM features\r\n");
    printf("************************************************************\r\n\n");

    printf("1. 50%% duty cycle :- observe the output on LEDs D10 and D11.\r\n\n");
    printf("2. Immediate duty cycle updated :- observe the output on pin P5_1 and P5_0.\r\n\n");
    printf("3. Debug freeze and suspend :- observe the output on pin P6_2 and P6_3.\r\n\n");

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the PPCA block with the generated configuration and enable it.
     * The PPCA manages the two PPCA CM33 cores (CPU0 and CPU1). */
    Cy_PPCA_CNFG_Init(ppca_0_ppca_cnfg_0_HW, &ppca_0_ppca_cnfg_0_config);
    Cy_PPCA_Enable(ppca_0_ppca_cnfg_0_HW);

    /* EPU general settings */
    Cy_PPCA_EPU_EnableExclusiveAccess(EPU_HW, true);
    Cy_PPCA_EPU_Enable(EPU_HW);

    /* Load and boot PPCA Core 0 (ppca_cm33_0) and Core 1 (ppca_cm33_1) from flash.
     * Once started, both cores run concurrently and independently of this core. */
    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    /* Initialize PWM A: frequency sweep, driven by the PPCA Core 0 ISR */
    status = Cy_TCPWM_PWM_Init(TCPWM_A_HW, TCPWM_A_NUM, &TCPWM_A_config);

    /* PWM A initialization failed. Stop program execution */
    if(CY_TCPWM_SUCCESS != status)
    {
        CY_ASSERT(0);
    }

    /* Enable PWM A */
    Cy_TCPWM_PWM_Enable(TCPWM_A_HW, TCPWM_A_NUM);

    /* Configuring EPU processing unit */
    Cy_PPCA_EPU_PU_T1_Configure(PPCA_EPU_EPU, pu_1_INDEX, &pu_1_put1_config);
    Cy_PPCA_EPU_PU_T1_Enable(PPCA_EPU_EPU, pu_1_INDEX,CY_ENABLE_ASYNC_BYPASS);

    /* Configuring EPU combiner */
    Cy_PPCA_EPU_Combo_Configure(PPCA_EPU_EPU, comb4_INDEX, &comb4_combo_config);

    /* Initializing the PWM */
    status = Cy_TCPWM_PWM_Init(TCPWM_B_HW, TCPWM_B_NUM, &TCPWM_B_config);

    /* PWM B initialization failed. Stop program execution */
    if(CY_TCPWM_SUCCESS != status)
    {
        CY_ASSERT(0);
    }

    /* Enable PWM B */
    Cy_TCPWM_PWM_Enable(TCPWM_B_HW, TCPWM_B_NUM);

    /* Initialize PWM C: demonstrates the TCPWM debug freeze/suspend feature */
    status = Cy_TCPWM_PWM_Init(TCPWM_C_HW, TCPWM_C_NUM, &TCPWM_C_config);

    /* PWM C initialization failed. Stop program execution */
    if(CY_TCPWM_SUCCESS != status)
    {
        CY_ASSERT(0);
    }

    /* Enable PWM C */
    Cy_TCPWM_PWM_Enable(TCPWM_C_HW, TCPWM_C_NUM);

    /* start the PWM B */
    Cy_TCPWM_TriggerStart_Single(TCPWM_B_HW, TCPWM_B_NUM);

    /* start the PWM A */
    Cy_TCPWM_TriggerStart_Single(TCPWM_A_HW, TCPWM_A_NUM);

    /* Start PWM C to begin the debug freeze/suspend demonstration */
    Cy_TCPWM_TriggerStart_Single(TCPWM_C_HW, TCPWM_C_NUM);

    for(;;)
    {
        /* Idle loop — all PWM behavior is driven by the PPCA Core 0 ISRs */
    }
}

