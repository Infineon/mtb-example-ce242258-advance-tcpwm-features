/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for PPCA Core 1 (CM33 CPU1) of the
*              Advanced TCPWM Features example for ModusToolbox.
*
*              This core runs on PPCA CM33 CPU1 and is booted by the secure
*              main core (main_cm33_s) after all peripherals have been
*              initialized. This core is reserved for additional
*              parallel workloads and inter-core communication via shared memory.
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
#define PPCA_CPU1_M4_VAR_ADDRESS 0x20040800  /* Used by this core (CPU1) */

/*******************************************************************************
* Global Variables
********************************************************************************/


/*******************************************************************************
* Function Prototypes
********************************************************************************/


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Entry point for PPCA CM33 Core 1. This core is booted by the secure main
*  core after all shared peripherals have been initialized. It runs
*  concurrently with the secure main core and with PPCA Core 0.
*
*  Current responsibilities:
*    1. Reserved for future parallel processing tasks.
*    2. May write status data to PPCA_CPU1_M4_VAR_ADDRESS for the main core
*       to read via the PPCA peripheral memory window (0x53050800).
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

     for(;;)
     {
         /* Idle loop. Add application tasks here, or use WFI to reduce power
          * consumption while waiting for interrupt-driven events. */
     }
}
