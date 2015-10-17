// Standard libraries
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

/**
 * Adds the specified item to the player's inventory if the inventory isn't full.
 * @param item The item number to be stored: valid values are 0-255.
 * @return SUCCESS if the item was added, STANDARD_ERRROR if the item couldn't be added.
 */
static uint8_t inventory[INVENTORY_SIZE];
static uint8_t index = 0;

int AddToInventory(uint8_t item)
{
    if((item >= 0) && (item <= 255)) {
        inventory[index] = item;
        ++index;
        return SUCCESS;
    } else if(index >= INVENTORY_SIZE) {
        return STANDARD_ERROR;
    } else {
        return STANDARD_ERROR;
    }
}

/**
 * Check if the given item exists in the player's inventory.
 * @param item The number of the item to be added: valid values are 0-255.
 * @return SUCCESS if it was found or STANDARD_ERROR if it wasn't.
 */
int FindInInventory(uint8_t item)
{
    int i;
    if((item <= 255) && (item >= 0)) {
        for(i = 0; i < INVENTORY_SIZE; ++i) {
            if(inventory[i] == item) {
                return SUCCESS;
            }
        }
    } else {
        return STANDARD_ERROR;
    }

    return STANDARD_ERROR;
}
