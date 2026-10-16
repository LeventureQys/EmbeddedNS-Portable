;/**************************************************************************//**
; * @file     startup_ARMCM4.s
; * @brief    CMSIS Core Device Startup File for
; *           ARMCM4 Device Series
; * @version  V5.00
; * @date     02. March 2016
; ******************************************************************************/
;/*
; * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00001000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000100

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp                ; Top of Stack
                DCD     Reset_Handler               ; Reset Handler
                DCD     NMI_Handler                 ; NMI Handler
                DCD     HardFault_Handler           ; Hard Fault Handler
                DCD     MemManage_Handler           ; MPU Fault Handler
                DCD     BusFault_Handler            ; Bus Fault Handler
                DCD     UsageFault_Handler          ; Usage Fault Handler
                DCD     0                           ; Reserved
                DCD     0                           ; Reserved
                DCD     0                           ; Reserved
                DCD     0                           ; Reserved
                DCD     SVC_Handler                 ; SVCall Handler
                DCD     DebugMon_Handler            ; Debug Monitor Handler
                DCD     0                           ; Reserved
                DCD     PendSV_Handler              ; PendSV Handler
                DCD     SysTick_Handler             ; SysTick Handler

                ; External Interrupts
                DCD     TIMER0_IRQHandler           ; TIMER 0 IRQ Handler
                DCD     TIMER1_IRQHandler           ; TIMER 1 IRQ Handler
                DCD     TIMER2_IRQHandler           ; TIMER 2 IRQ Handler
                DCD     TIMER3_IRQHandler           ; TIMER 3 IRQ Handler
                DCD     TIMER_DUAL_IRQHandler       ; DUAL TIMER IRQ Handler
                DCD     UART0_TX_IRQHandler         ; UART 0 TX IRQ Handler
                DCD     UART0_RX_IRQHandler         ; UART 0 RX IRQ Handler
                DCD     UART1_TX_IRQHandler         ; UART 1 TX IRQ Handler
                DCD     UART1_RX_IRQHandler         ; UART 1 RX IRQ Handler
                DCD     UART2_TX_IRQHandler         ; UART 2 TX IRQ Handler
                DCD     UART2_RX_IRQHandler         ; UART 2 RX IRQ Handler
                DCD     UART3_TX_IRQHandler         ; UART 2 TX IRQ Handler
                DCD     UART3_RX_IRQHandler         ; UART 2 RX IRQ Handler
                DCD     I2C_M_TX_IRQHandler         ; I2C Master TX IRQ Handler
                DCD     I2C_M_RX_IRQHandler         ; I2C Master RX IRQ Handler
                DCD     I2C_S_IRQHandler            ; I2C Slave IRQ Handler
                DCD     SPI1_TX_IRQHandler          ; SPI 1 TX IRQ Handler
                DCD     SPI1_RX_IRQHandler          ; SPI 1 RX IRQ Handler
                DCD     SPI2_TX_IRQHandler          ; SPI 2 TX IRQ Handler
                DCD     SPI2_RX_IRQHandler          ; SPI 2 RX IRQ Handler
                DCD     SPI3_TX_IRQHandler          ; SPI 3 TX IRQ Handler
                DCD     SPI3_RX_IRQHandler          ; SPI 3 RX IRQ Handler
                DCD     PWM_IRQHandler              ; PWM IRQ Handler
                DCD     EXT0_INT_IRQHandler         ; EXT 0 INT IRQ Handler
                DCD     EXT1_INT_IRQHandler         ; EXT 1 INT IRQ Handler
                DCD     ADC_IRQHandler              ; ADC IRQ Handler  	
                DCD     I2S_IRQHandler              ; I2S IRQ Handler
                DCD     I2S_S1_IRQHandler           ; I2S IRQ Handler
                DCD     I2S_S2_IRQHandler           ; I2S IRQ Handler
                DCD     SPI_IRQHandler              ; SPI Flash Controller IRQ Handler
                DCD     USB_IRQHandler              ; USB IRQ Handler
                DCD     SDC_IRQHandler              ; SDC IRQ Handler
                DCD     DMA0_IRQHandler             ; DMA 0 IRQ Handler
                DCD     DMA1_IRQHandler             ; DMA 1 IRQ Handler
                DCD     DSP_INT_IRQHandler          ; DSP INT IRQ Handler
                DCD     DSP_END_IRQHandler          ; DSP END IRQ Handler

                DCD     USER0_IRQHandler            ; user0 sofrware irq handler
                DCD     USER1_IRQHandler            ; user1 sofrware irq handler
                DCD     USER2_IRQHandler            ; user2 sofrware irq handler
                DCD     USER3_IRQHandler            ; user3 sofrware irq handler
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

;Reset_Handler   PROC
                ;EXPORT  Reset_Handler               [WEAK]
                ;IMPORT  SystemInit
                ;IMPORT  __main
                ;LDR     R0, =SystemInit
                ;BLX     R0
                ;LDR     R0, =__main
                ;BX      R0
                ;ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler                 [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler           [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler           [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler            [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler          [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler                 [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler            [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler              [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler             [WEAK]
                B       .
                ENDP

Default_Handler PROC
                EXPORT  TIMER0_IRQHandler           [WEAK]
                EXPORT  TIMER1_IRQHandler           [WEAK]
                EXPORT  TIMER2_IRQHandler           [WEAK]
                EXPORT  TIMER3_IRQHandler           [WEAK]
                EXPORT  TIMER_DUAL_IRQHandler       [WEAK]
                EXPORT  UART0_TX_IRQHandler         [WEAK]
                EXPORT  UART0_RX_IRQHandler         [WEAK]
                EXPORT  UART1_TX_IRQHandler         [WEAK]
                EXPORT  UART1_RX_IRQHandler         [WEAK]
                EXPORT  UART2_RX_IRQHandler         [WEAK]
                EXPORT  UART2_TX_IRQHandler         [WEAK]
                EXPORT  UART3_TX_IRQHandler         [WEAK]
                EXPORT  UART3_RX_IRQHandler         [WEAK]
                EXPORT  I2C_M_TX_IRQHandler         [WEAK]
                EXPORT  I2C_M_RX_IRQHandler         [WEAK]
                EXPORT  I2C_S_IRQHandler            [WEAK]
                EXPORT  SPI1_TX_IRQHandler          [WEAK]
                EXPORT  SPI1_RX_IRQHandler          [WEAK]
                EXPORT  SPI2_TX_IRQHandler          [WEAK]
                EXPORT  SPI2_RX_IRQHandler          [WEAK]
                EXPORT  SPI3_TX_IRQHandler          [WEAK]
                EXPORT  SPI3_RX_IRQHandler          [WEAK]
                EXPORT  PWM_IRQHandler              [WEAK]
                EXPORT  EXT0_INT_IRQHandler         [WEAK]
                EXPORT  EXT1_INT_IRQHandler         [WEAK]
                EXPORT  ADC_IRQHandler              [WEAK]
                EXPORT  I2S_IRQHandler              [WEAK]
                EXPORT  I2S_S1_IRQHandler           [WEAK]
                EXPORT  I2S_S2_IRQHandler           [WEAK]
                EXPORT  SPI_IRQHandler              [WEAK]
                EXPORT  USB_IRQHandler              [WEAK]
                EXPORT  SDC_IRQHandler              [WEAK]
                EXPORT  DMA0_IRQHandler             [WEAK]
                EXPORT  DMA1_IRQHandler             [WEAK]
                EXPORT  DSP_INT_IRQHandler          [WEAK]
                EXPORT  DSP_END_IRQHandler          [WEAK]
                EXPORT  USER0_IRQHandler            [WEAK]
                EXPORT  USER1_IRQHandler            [WEAK]
                EXPORT  USER2_IRQHandler            [WEAK]
                EXPORT  USER3_IRQHandler            [WEAK]

TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
TIMER3_IRQHandler
TIMER_DUAL_IRQHandler
UART0_TX_IRQHandler
UART0_RX_IRQHandler
UART1_TX_IRQHandler
UART1_RX_IRQHandler
UART2_TX_IRQHandler
UART2_RX_IRQHandler
UART3_TX_IRQHandler
UART3_RX_IRQHandler
I2C_M_RX_IRQHandler
I2C_M_TX_IRQHandler
I2C_S_IRQHandler 
SPI1_TX_IRQHandler 
SPI1_RX_IRQHandler 
SPI2_TX_IRQHandler
SPI2_RX_IRQHandler
SPI3_TX_IRQHandler
SPI3_RX_IRQHandler 
PWM_IRQHandler
EXT0_INT_IRQHandler
EXT1_INT_IRQHandler
ADC_IRQHandler
I2S_IRQHandler
I2S_S1_IRQHandler
I2S_S2_IRQHandler
SPI_IRQHandler
USB_IRQHandler
SDC_IRQHandler
DMA0_IRQHandler  
DMA1_IRQHandler  
DSP_INT_IRQHandler 
DSP_END_IRQHandler
USER0_IRQHandler
USER1_IRQHandler
USER2_IRQHandler
USER3_IRQHandler

                B       .

                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                AREA    BOOT, CODE, READONLY
                
Reset_Handler   PROC
                EXPORT  Reset_Handler               [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                IMPORT  BootCheck
                LDR     SP, =__initial_sp
                LDR     R0, =BootCheck
                BLX     R0
                CBZ     R0, RUN_DFU_ROM

                MOV     R2, R0
                LDR     R0, [R0]
                LDR     R1, =0x1FFE0000
                AND     R0, R0, R1
                CMP     R0, #0x10000000
                BNE     RUN_DFU_ROM

                MOV     R0, R2
                LDR     SP, [R0]
                LDR     R0, [R0, #4]
                BX      R0
RUN_DFU_ROM
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0

;               NOP
                ENDP

                END
