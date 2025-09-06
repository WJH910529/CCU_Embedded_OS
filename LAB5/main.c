/**************************************************************************//**
 * @file     main.c
 * @version  V3.0
 * $Revision: 2 $
 * $Date: 2016/02/29 LoRa Node v1.2
 *		Add server.
 *		for experiment, CMD SF change, add BW change
 * @brief
 *           LoRA Channel activity detection(CAD) test.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC100Series.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"
#include "string.h"

#define PLL_CLOCK           50000000

#define COMMAND_QUEUE_LENGTH 20

#define MODE1_BIT (1 << 0)
#define MODE2_BIT (1 << 1)
#define MODE3_BIT (1 << 3)
#define MODE4_BIT (1 << 4)

static void vTaskA(void* pvParameters);
static void vTaskB(void* pvParameters);
static void vTaskC(void* pvParameters);

QueueHandle_t xCommandQueue;
EventGroupHandle_t xEventGroup;
volatile EventBits_t currentCommand = 0;

void vStartThreadTasks( void );

/* Function prototype declaration */
void SYS_Init(void);
void UART0_Init(void);


/*****************************************
*
*        DIO Mapping interrupt
*
*****************************************/

void GPIO_Init()
{
		//INIT LEDS
		GPIO_SetMode(PC, BIT12, GPIO_PMD_OUTPUT);
		GPIO_SetMode(PC, BIT13, GPIO_PMD_OUTPUT);
		GPIO_SetMode(PC, BIT14, GPIO_PMD_OUTPUT);
		GPIO_SetMode(PC, BIT15, GPIO_PMD_OUTPUT);
	
		//INIT RGB LED
		GPIO_SetMode(PA,BIT12,GPIO_PMD_OUTPUT);
		GPIO_SetMode(PA,BIT13,GPIO_PMD_OUTPUT);
		GPIO_SetMode(PA,BIT14,GPIO_PMD_OUTPUT);
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184 MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);
	
		/* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));
    
    /* Enable external 12 MHz XTAL */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock rate as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);
		
    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);		

    /* Select HXT as the clock source of UART0 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));

    /* Enable SPI1 peripheral clock */
    CLK_EnableModuleClock(SPI0_MODULE);          //change SPI0_MODULE

    /* Select HCLK as the clock source of SPI1 */
    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL1_SPI0_S_HCLK, MODULE_NoMsk);

		/* Enable Timer 0 module clock */
		CLK_EnableModuleClock(TMR0_MODULE);
		
		/* Select Timer 0 module clock source */
		CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0_S_HXT, NULL);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CyclesPerUs automatically. */
    //SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP = SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD;

//		SYS->GPC_MFP = SYS_GPC_MFP_PC8_SPI1_SS0 | SYS_GPC_MFP_PC9_SPI1_CLK | SYS_GPC_MFP_PC10_SPI1_MISO0 | SYS_GPC_MFP_PC11_SPI1_MOSI0;
//		SYS->ALT_MFP = SYS_ALT_MFP_PC8_SPI1_SS0;

		
//		SYS->GPA_MFP = SYS_GPA_MFP_PA1_GPIO | SYS_GPA_MFP_PA2_GPIO | SYS_GPA_MFP_PA3_GPIO;
		
    /* Setup SPI0 multi-function pins */
    SYS->GPC_MFP = SYS_GPC_MFP_PC0_SPI0_SS0 | SYS_GPC_MFP_PC1_SPI0_CLK | SYS_GPC_MFP_PC2_SPI0_MISO0 | SYS_GPC_MFP_PC3_SPI0_MOSI0;
    SYS->ALT_MFP = SYS_ALT_MFP_PC0_SPI0_SS0 | SYS_ALT_MFP_PC1_SPI0_CLK | SYS_ALT_MFP_PC2_SPI0_MISO0 | SYS_ALT_MFP_PC3_SPI0_MOSI0;
}

void UART0_Init(void)
{
		SYS_ResetModule(UART0_RST);
	
    /* Configure UART0: 115200, 8-bit word, no parity bit, 1 stop bit. */
    UART_Open(UART0, 115200);
}

/**********************************************/
/*                Main function               */
/*                LoRa SPI sample             */
/*        Bit length of a transaction: 8      */
/* SPI1, SS0(PC.8), CLK(PC.9), MISO0(PC.10)   */
/*                MOSI0(PC.11)                */
/**********************************************/
void blink_left_to_right(int freq){
								PC12=1;
								PC13=1;
								PC14=1;
								PC15=1;	
	
								PC12=0;
								vTaskDelay(freq);
								PC12=1;						
								vTaskDelay(freq);
	
								PC13=0;
								vTaskDelay(freq);
								PC13=1;
								vTaskDelay(freq);
								
								PC14=0;
								vTaskDelay(freq);
								PC14=1;
								vTaskDelay(freq);
								
								PC15=0;
								vTaskDelay(freq);
								PC15=1;
								vTaskDelay(freq);
}	

void blink_right_to_left(int freq){
								PC12=1;
								PC13=1;
								PC14=1;
								PC15=1;
	
								PC15=0;
								vTaskDelay(freq);
								PC15=1;
								vTaskDelay(freq);
	
								PC14=0;
								vTaskDelay(freq);
								PC14=1;
								vTaskDelay(freq);
								
								PC13=0;
								vTaskDelay(freq);
								PC13=1;
								vTaskDelay(freq);
								
								PC12=0;
								vTaskDelay(freq);
								PC12=1;
								vTaskDelay(freq);
}	


void blink_all(int freq){
		int i;
		PC12=PC13=PC14=PC15=1;
	
		vTaskDelay(freq);
		PC12=PC13=PC14=PC15=0;
		vTaskDelay(freq);
		PC12=PC13=PC14=PC15=1;

}

void turn_RGBLED_off(){
	vTaskDelay(1);
	PA12 = 1; // Blue component
	PA13 = 1; // Green component
	PA14 = 1; // Red component
}

void All_Bits_Reset(){
	xEventGroupClearBits(xEventGroup, MODE1_BIT);
	xEventGroupClearBits(xEventGroup, MODE2_BIT);
	xEventGroupClearBits(xEventGroup, MODE3_BIT);
	xEventGroupClearBits(xEventGroup, MODE4_BIT);

}


int main(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Init system, IP clock and multi-function I/O. */
    SYS_Init();
    /* Lock protected registers */
    SYS_LockReg();
		GPIO_Init();  // LED initial
		UART0_Init();
	
		printf("\n\n----------------LAB5-Basic----------------\r\n");
		xCommandQueue = xQueueCreate(COMMAND_QUEUE_LENGTH,sizeof(char[10]));
		xEventGroup = xEventGroupCreate();

		vStartThreadTasks();
		vTaskStartScheduler();

		while(1);

}


void vStartThreadTasks( void )
{
		xTaskCreate(vTaskA, "vTaskA",128,NULL,1,( xTaskHandle * ) NULL );  //pxTaskCode , pcName,usStackDepth,pvParameters,uxPriority,pxCreatedTask
		xTaskCreate(vTaskB, "vTaskB",128,NULL,1,( xTaskHandle * ) NULL );
		xTaskCreate(vTaskC, "vTaskC",128,NULL,1,( xTaskHandle * ) NULL );
}

static void vTaskA(void* pvParameters)
{
    char command[10];

    while (1) {
        printf("Enter a command: ");
        scanf("%s", command);
				printf("\nYour command is : %s\n",command);
				All_Bits_Reset();		
			
        if (strcmp(command, "mode1") == 0) {
            xEventGroupSetBits(xEventGroup, MODE1_BIT);
        } else if (strcmp(command, "mode2") == 0) {
            xEventGroupSetBits(xEventGroup, MODE2_BIT);
        } else if (strcmp(command, "mode3") ==0){
						xEventGroupSetBits(xEventGroup, MODE3_BIT);
				} else if (strcmp(command, "mode4") ==0){
						xEventGroupSetBits(xEventGroup, MODE4_BIT);
				} else {
            printf("Invalid command!\n");
						turn_RGBLED_off();
        }
    }
}

static void vTaskB(void* pvParameters)
{
		while(1)
		{
				EventBits_t bits = xEventGroupWaitBits(xEventGroup, MODE1_BIT | MODE2_BIT | MODE3_BIT | MODE4_BIT , pdFALSE , pdFALSE , portMAX_DELAY); 
			//( const EventGroupHandle_t xEventGroup,const EventBits_t uxBitsToWaitFor,  const BaseType_t xClearOnExit,  const BaseType_t xWaitForAllBits,  TickType_t xTicksToWait );

				if ((bits & MODE1_BIT) != 0) {
						blink_left_to_right(90);
						//xEventGroupClearBits(xEventGroup, MODE1_BIT);
        }
				else if((bits & MODE2_BIT) != 0){
						blink_right_to_left(90);
				}
				else if((bits & MODE3_BIT) != 0){
						PC12=0;
						PC13=0;
						PC14=0;
						PC15=0;	
				}
				else if((bits & MODE4_BIT) != 0){
						blink_all(90);				
				}
		}
}

static void vTaskC(void* pvParameters)
{
		while(1)
		{
				EventBits_t bits = xEventGroupWaitBits(xEventGroup, MODE1_BIT | MODE2_BIT | MODE3_BIT | MODE4_BIT , pdFALSE , pdFALSE , portMAX_DELAY);
				if ((bits & MODE1_BIT) != 0) {
						PA14=0;
						//xEventGroupClearBits(xEventGroup, MODE1_BIT);
        }				
				else if((bits & MODE2_BIT) != 0){
						PA13=0;
				}
				else if((bits & MODE3_BIT) != 0){
						PA12=0;
				}
				else if((bits & MODE4_BIT) != 0){
						PA12=0;
						PA13=0;
						PA14=0;
				}
		}
}


/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
