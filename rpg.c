#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "HardwareDefs.h"
#include "Adc.h"
#include "Buttons.h"
#include "Common.h"
#include "Game.h"
#include "Leds.h"
#include "Oled.h"
#include "OledDriver.h"
#include "Player.h"

// **** Set any macros or preprocessor directives here ****
#define MAX_OLED_PIXELS 84
#define TITLE_OLED_SPACE 200
#define MAX_ROOM_NUM_VALUE 50
#define FIRST_PG_DESCRIPTION_OLED_SPACE 100
#define DESCRIPTION_COPY 63
#define OFFSET 21

// **** Declare any data types here ****
typedef struct {
    char roomNum[15];
    char title[GAME_MAX_ROOM_TITLE_LENGTH];
    char description[GAME_MAX_ROOM_DESC_LENGTH];
    uint8_t itemRequirements;
    uint8_t itemsContained;
    uint8_t roomExits;
    uint8_t north;
    uint8_t east;
    uint8_t south;
    uint8_t west;
}room_data;

// **** Define any global or external variables here ****
room_data roomData;
static uint8_t buttonEvents;

// **** Declare any function prototypes here ****

// Configuration Bit settings
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 20 MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
#pragma config FPLLIDIV   = DIV_2     // Set the PLL input divider to 2
#pragma config FPLLMUL    = MUL_20    // Set the PLL multiplier to 20
#pragma config FPLLODIV   = DIV_1     // Don't modify the PLL output.
#pragma config FNOSC      = PRIPLL    // Set the primary oscillator to internal RC w/ PLL
#pragma config FSOSCEN    = OFF       // Disable the secondary oscillator
#pragma config IESO       = OFF       // Internal/External Switch O
#pragma config POSCMOD    = XT        // Primary Oscillator Configuration
#pragma config OSCIOFNC   = OFF       // Disable clock signal output
#pragma config FPBDIV     = DIV_4     // Set the peripheral clock to 1/4 system clock
#pragma config FCKSM      = CSECMD    // Clock Switching and Monitor Selection
#pragma config WDTPS      = PS1       // Specify the watchdog timer interval (unused)
#pragma config FWDTEN     = OFF       // Disable the watchdog timer
#pragma config ICESEL     = ICS_PGx2  // Allow for debugging with the Uno32
#pragma config PWP        = OFF       // Keep the program flash writeable
#pragma config BWP        = OFF       // Keep the boot flash writeable
#pragma config CP         = OFF       // Disable code protect

int main()
{
    // Configure the device for maximum performance but do not change the PBDIV
    // Given the options, this function will change the flash wait states, RAM
    // wait state and enable prefetch cache but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above..
    SYSTEMConfig(F_SYS, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // Auto-configure the PIC32 for optimum performance at the specified operating frequency.
    SYSTEMConfigPerformance(F_SYS);

    // osc source, PLL multipler value, PLL postscaler , RC divisor
    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_1, OSC_FRC_POST_1);

    // Configure the PB bus to run at 1/4th the CPU frequency, so 20MHz.
    OSCSetPBDIV(OSC_PB_DIV_4);

    // Enable multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
    INTEnableInterrupts();

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, F_PB / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

/******************************** Your custom code goes below here ********************************/
    int check;
    OledInit();
    AdcInit();
    LEDS_INIT();
    check = GameInit();

    if(check == STANDARD_ERROR) {
        FATAL_ERROR();
    }
    float currPage;
    float binSize;
    float titleSize;
    float descSize;
    float numPages;
    uint8_t roomExit;
    uint16_t adcValue = 0;

    while(1) {
        roomExit = GameGetCurrentRoomExits();
        LEDS_SET(roomExit);
        while(buttonEvents == 0) {
            descSize = GameGetCurrentRoomDescription(roomData.description);
            titleSize = GameGetCurrentRoomTitle(roomData.title);

            numPages = ((titleSize + descSize) / MAX_OLED_PIXELS);
            binSize = (ADC_MAX_VALUE / numPages);

            if(AdcChanged()) {
                adcValue = AdcRead();
            }

            currPage = (adcValue / binSize);
            if(currPage < 1) {
                char titleArray[TITLE_OLED_SPACE] = {0};
                char descriptionBuffer[FIRST_PG_DESCRIPTION_OLED_SPACE] = {0};

                strncpy(descriptionBuffer, roomData.description, DESCRIPTION_COPY);
                sprintf(titleArray, "%s\n%s", roomData.title, descriptionBuffer);

                OledClear(OLED_COLOR_BLACK);
                OledDrawString(titleArray);
            } else {
                char buffer[MAX_OLED_PIXELS] = {0};
                int buffIndex;
                buffIndex = (int)currPage * MAX_OLED_PIXELS;
                strncpy(buffer, (roomData.description + buffIndex - OFFSET), MAX_OLED_PIXELS);

                OledClear(OLED_COLOR_BLACK);
                OledDrawString(buffer);
            }
            OledUpdate();
        }

        if((buttonEvents & BUTTON_EVENT_4UP) && (roomExit & GAME_ROOM_EXIT_NORTH_EXISTS)) {
            GameGoNorth();
        } else if((buttonEvents & BUTTON_EVENT_3UP) && (roomExit & GAME_ROOM_EXIT_EAST_EXISTS)) {
            GameGoEast();
        } else if((buttonEvents & BUTTON_EVENT_2UP) && (roomExit & GAME_ROOM_EXIT_SOUTH_EXISTS)) {
            GameGoSouth();
        } else if((buttonEvents & BUTTON_EVENT_1UP) && (roomExit & GAME_ROOM_EXIT_WEST_EXISTS)) {
            GameGoWest();
        }
        buttonEvents = BUTTON_EVENT_NONE;
    }



/**************************************************************************************************/
    while (1);
}

/**
 * Timer2 interrupt. Checks for button events.
 */
void __ISR(_TIMER_2_VECTOR, ipl4) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;
    buttonEvents = ButtonsCheckEvents();
}