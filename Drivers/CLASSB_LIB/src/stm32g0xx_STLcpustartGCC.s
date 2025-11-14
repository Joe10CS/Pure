/**
  ******************** (C) COPYRIGHT 2019 STMicroelectronics ********************
  * @file               : stm32g0xx_STLcpustartGCC.s
  * @author             : MCD Application Team
  * @version            : V2.3.0
  * @date               : 28-Jun-2019
  * @brief              : This file contains the Cortex-M0 CPU test to be done
  *                       at start-up.
  *******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2019 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m0
  .fpu softvfp
  .thumb

  /* Reference to the FailSafe routine to be executed in case of non-recoverable
     failure */
  .extern FailSafePOR

  /* reference to C variables for control flow monitoring */
  .extern CtrlFlowCnt
  .extern CtrlFlowCntInv

  .global  STL_StartUpCPUTest
  .global  conAA
  .global  con55

/**
 ******************************************************************************
 * @brief   Cortex-M0 CPU test during start-up
 *          If possible, branches are 16-bit only in dependence on BL instruction
 *          relative offset
 *          Test jumps directly to a Fail Safe routine in case of failure
 *          All registers are destroyed when exiting this function (including
 *          preserved registers R4 to R11) and excluding stack point R13
 * @param   None
 * @retval : CPUTEST_SUCCESS (=1) if test is ok
*/
  .section  .text.__TEST_PATTERNS
  .align 4
  .type  __TEST_PATTERNS, %object
  .size  __TEST_PATTERNS, .-__TEST_PATTERNS

__TEST_PATTERNS:
conAA:      .long     0xAAAAAAAA
con55:      .long     0x55555555
con80:      .long     0x80000000
conA8:      .long     0xAAAAAAA8
con54:      .long     0x55555554

  .section  .text.STL_StartUpCPUTest
  .type  STL_StartUpCPUTest, %function
  .size  STL_StartUpCPUTest, .-STL_StartUpCPUTest

STL_StartUpCPUTest:
    PUSH {R4-R6}               /* Safe critical registers */

    MOVS R0, #0x00      	  
    UXTB R0,R0
    ADDS R0,#0				   /* Set Z(ero) Flag */
    BNE CPUTestFail            /* Fails if Z clear */
    BMI CPUTestFail            /* Fails if N is set */
    SUBS R0,#1                 /* Set N(egative) Flag */
    BPL CPUTestFail            /* Fails if N clear */
    ADDS R0,#2                 /* Set C(arry) Flag and do not set Z */
    BCC CPUTestFail            /* Fails if C clear */
    BEQ CPUTestFail            /* Fails if Z is set */
    BMI CPUTestFail            /* Fails if N is set */

    LDR R0,=con80      	  	   /* Prepares Overflow test */
    LDR R0,[R0]
    ADDS R0, R0, R0            /* Set V(overflow) Flag */
    BVC CPUTestFail            /* Fails if V clear */

    /*  This is for control flow test (ENTRY point) */
    LDR R0,=CtrlFlowCnt
    /* Assumes R1 OK If not, error will be detected by R1 test and Ctrl flow test later on */
    LDR R1,[R0]
    ADDS R1,R1,#0x3	           /* CtrlFlowCnt += OxO3 */
    STR R1,[R0]

    PUSH {LR}                 /* test R14 (LR) - based on LR modification by ADDS */
    POP {R1}                  /* original value of LR is kept at R1 */
    MOV R0,R1
    MVNS R0,R0
    ADDS R0,#1
    ADD R14,R14,R0            /* LR is cleared */
    LDR R0, =conAA
    LDR R2,[R0]
    LDR R0,[R0]
    ADD R14,R14,R0            /* 0xAAAAAAAA is stored to LR */
    PUSH {LR}
    POP {R0}
    CMP R0,R2
    BNE CPUTestFail
    MVNS R0,R0
    ADDS R0,#1
    ADD R14,R14,R0            /* LR is cleared */
    LDR R0, =con55
    LDR R2,[R0]
    LDR R0,[R0]
    ADD R14,R14,R0            /* 0x55555555 is stored to LR */
    PUSH {LR}
    POP {R0}
    CMP R0,R2
    BNE CPUTestFail
    MVNS R0,R0
    ADDS R0,#1
    ADD R14,R14,R0            /* LR is cleared */
    ADD R14,R14,R1            /* LR is filled by its original value */

    /* Register R1 */
    LDR R0, =conAA
    LDR R1,[R0]
    LDR R0,[R0]
    CMP R0,R1
    BNE CPUTestFail
    LDR R0, =con55
    LDR R1,[R0]
    LDR R0,[R0]
    CMP R0,R1
    BNE CPUTestFail
    MOVS R1, #0x1              /* For ramp test */

    /* Register R2 */
    LDR R0, =conAA
    LDR R2,[R0]
    LDR R0,[R0]
    CMP R0,R2
    BNE CPUTestFail
    LDR R0, =con55
    LDR R2,[R0]
    LDR R0,[R0]
    CMP R0,R2
    BNE CPUTestFail
    MOVS R2, #0x2              /* For ramp test */

    /* Register R3 */
    LDR R0, =conAA
    LDR R3,[R0]
    LDR R0,[R0]
    CMP R0,R3
    BNE CPUTestFail
    LDR R0, =con55
    LDR R3,[R0]
    LDR R0,[R0]
    CMP R0,R3
    BNE CPUTestFail
    MOVS R3, #0x3              /* For ramp test */

    /* Register R4 */
    LDR R0, =conAA
    LDR R4,[R0]
    LDR R0,[R0]
    CMP R0,R4
    BNE CPUTestFail
    LDR R0, =con55
    LDR R4,[R0]
    LDR R0,[R0]
    CMP R0,R4
    BNE CPUTestFail
    MOVS R4, #0x4              /* For ramp test */

    /* Register R5 */
    LDR R0, =conAA
    LDR R5,[R0]
    LDR R0,[R0]
    CMP R0,R5
    BNE CPUTestFail
    LDR R0, =con55
    LDR R5,[R0]
    LDR R0,[R0]
    CMP R0,R5
    BNE CPUTestFail
    MOVS R5, #0x5              /* For ramp test */

    /* Register R6 */
    LDR R0, =conAA
    LDR R6,[R0]
    LDR R0,[R0]
    CMP R0,R6
    BNE CPUTestFail
    LDR R0, =con55
    LDR R6,[R0]
    LDR R0,[R0]
    CMP R0,R6
    BNE CPUTestFail
    MOVS R6, #0x6              /* For ramp test */

    /* Register R7 */
    LDR R0, =conAA
    LDR R7,[R0]
    LDR R0,[R0]
    CMP R0,R7
    BNE CPUTestFail
    LDR R0, =con55
    LDR R7,[R0]
    LDR R0,[R0]
    CMP R0,R7
    BNE CPUTestFail
    MOVS R7, #0x7              /* For ramp test */

    /* Register R8 */
    LDR R0, =conAA
    LDR R0,[R0]
    MOV R8,R0
    CMP R0,R8
    BNE CPUTestFail
    BAL CPUTstCont

CPUTestFail:
    BLAL FailSafePOR

CPUTstCont:
    LDR R0, =con55
    LDR R0,[R0]
    MOV R8,R0
    CMP R0,R8
    BNE CPUTestFail
    MOVS R0, #0x08             /* For ramp test */
    MOV	R8,R0

    /* Register R9 */
    LDR R0, =conAA
    LDR R0,[R0]
    MOV R9,R0
    CMP R0,R9
    BNE CPUTestFail
    LDR R0, =con55
    LDR R0,[R0]
    MOV R9,R0
    CMP R0,R9
    BNE CPUTestFail
    MOVS R0, #0x09             /* For ramp test */
    MOV	R9,R0

    /* Register R10 */
    LDR R0, =conAA
    LDR R0,[R0]
    MOV R10,R0
    CMP R0,R10
    BNE CPUTestFail
    LDR R0, =con55
    LDR R0,[R0]
    MOV R10,R0
    CMP R0,R10
    BNE CPUTestFail
    MOVS R0, #0x0A             /* For ramp test */
    MOV	R10,R0

    /* Register R11 */
    LDR R0, =conAA
    LDR R0,[R0]
    MOV R11,R0
    CMP R0,R11
    BNE CPUTestFail
    LDR R0, =con55
    LDR R0,[R0]
    MOV R11,R0
    CMP R0,R11
    BNE CPUTestFail
    MOVS R0, #0x0B             /* For ramp test */
    MOV	R11,R0

    /* Register R12 */
    LDR R0, =conAA
    LDR R0,[R0]
    MOV R12,R0
    CMP R0,R12
    BNE CPUTestFail
    LDR R0, =con55
    LDR R0,[R0]
    MOV R12,R0
    CMP R0,R12
    BNE CPUTestFail
    MOVS R0, #0x0C             /* For ramp test */
    MOV	R12,R0
    LDR R0, =CPUTstCont

    /* Ramp pattern verification	(R0 is not tested) */
    CMP R1, #0x01
    BNE CPUTestFail
    CMP R2, #0x02
    BNE CPUTestFail
    CMP R3, #0x03
    BNE CPUTestFail
    CMP R4, #0x04
    BNE CPUTestFail
    CMP R5, #0x05
    BNE CPUTestFail
    CMP R6, #0x06
    BNE CPUTestFail
    CMP R7, #0x07
    BNE CPUTestFail
    MOVS R0, #0x08
    CMP R0,R8
    BNE CPUTestFail
    MOVS R0, #0x09
    CMP R0,R9
    BNE CPUTestFail
    MOVS R0, #0x0A
    CMP R0,R10
    BNE CPUTestFail
    MOVS R0, #0x0B
    CMP R0,R11
    BNE CPUTestFail
    MOVS R0, #0x0C
    CMP R0,R12
    BNE CPUTestFail

    /* Process Stack pointer (banked Register R13) */
    MRS R0,PSP           /* Save process stack value */
    LDR R1, =conA8       /* Test is different (PSP is word aligned, 2 LSB cleared) */
    LDR R1,[R1]
    MSR PSP,R1           /* load process stack value */
    MRS R2,PSP           /* Get back process stack value */
    CMP R2,R1            /* Verify value */
    BNE CPUTestFail
    LDR R1, =con54       /* Test is different (PSP is word aligned, 2 LSB cleared) */
    LDR R1,[R1]
    MSR PSP,R1           /* load process stack value */
    MRS R2,PSP           /* Get back process stack value */
    CMP R2,R1            /* Verify value */
    BNE CPUTestFail
    MSR PSP, R0          /* Restore process stack value */

    /* Stack pointer (Register R13) */
    MRS R0,MSP           /* Save stack pointer value */
    LDR R1, =conA8       /* Test is different (SP is word aligned, 2 LSB cleared) */
    LDR R1,[R1]
    MSR MSP,R1			 /* load SP value */
    MRS R2,MSP			 /* Get back SP value */
    CMP R2,R1			 /* Verify value */
    BNE CPUTestFail
    LDR R1, =con54
    LDR R1,[R1]			 /* load SP value */
    MSR MSP,R1			 /* Get back SP value */
    MRS R2,MSP			 /* Verify value */
    CMP R2,R1
    BNE CPUTestFail
    MSR MSP,R0			 /* Restore stack pointer value */

    /* Control flow test (EXIT point) */
    LDR R0,=CtrlFlowCntInv
    LDR R1,[R0]
    SUBS R1,R1,#0x3	     /* CtrlFlowCntInv -= OxO3 */
    STR R1,[R0]

    POP {R4-R6}			 /* Restore critical registers */

    /* If next instruction is not executed, R0 will hold a value different from 0 */
    MOVS R0, #0x1        /* CPUTEST_SUCCESS */
    BX LR                /* return to the caller */

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE*****/
