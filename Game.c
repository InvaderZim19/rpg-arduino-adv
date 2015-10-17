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
/**
 * These function transitions between rooms. Each call should return SUCCESS if the current room has
 * an exit in the correct direction and the new room was able to be loaded, and STANDARD_ERROR
 * otherwise.
 * @return SUCCESS if the room CAN be navigated to and changing the current room to that new room
 *         succeeded.
 */

// Static Variables
static room_data room;

// Function Prototypes
static bool ParseFile(FILE *fp);

int GameGoNorth(void)
{
    sprintf(room.roomNum, "/room%d.txt", room.north);
    FILE *pFile = fopen(room.roomNum, "rb");
    if(ParseFile(pFile)){
        return SUCCESS;
    }
    return STANDARD_ERROR;
}

/**
 * @see GameGoNorth
 */
int GameGoEast(void)
{
    sprintf(room.roomNum, "/room%d.txt", room.east);
    FILE *pFile = fopen(room.roomNum, "rb");
    if(ParseFile(pFile)){
        return SUCCESS;
    }
    return STANDARD_ERROR;
}

/**
 * @see GameGoNorth
 */
int GameGoSouth(void)
{
    sprintf(room.roomNum, "/room%d.txt", room.south);
    FILE *pFile = fopen(room.roomNum, "rb");
    if(ParseFile(pFile)){
        return SUCCESS;
    }
    return STANDARD_ERROR;
}

/**
 * @see GameGoNorth
 */
int GameGoWest(void)
{
    sprintf(room.roomNum, "/room%d.txt", room.west);
    FILE *pFile = fopen(room.roomNum, "rb");
    if(ParseFile(pFile)){
        return SUCCESS;
    }
    return STANDARD_ERROR;
}

/**
 * This function sets up anything that needs to happen at the start of the game. This is just
 * setting the current room to STARTING_ROOM and loading it. It should return SUCCESS if it succeeds
 * and STANDARD_ERROR if it doesn't.
 * @return SUCCESS or STANDARD_ERROR
 */
int GameInit(void)
{
    sprintf(room.roomNum, "/room%d.txt", STARTING_ROOM);
    FILE *pFile = fopen(room.roomNum, "rb");
    if(ParseFile(pFile)){
        return SUCCESS;
    }
    return STANDARD_ERROR;
}

/**
 * Copies the appropriate room title as a NULL-terminated string into the provided character array.
 * The appropriate room means the first room description that the Player has the required items to
 * see. Only a NULL-character is copied if there was an error so that the resultant output string
 * length is 0.
 * @param desc A character array to copy the room title into. Should be GAME_MAX_ROOM_TITLE_LENGTH+1
 *             in length in order to allow for all possible titles to be copied into it.
 * @return The length of the string stored into `title`. Note that the actual number of chars
 *         written into `title` will be this value + 1 to account for the NULL terminating
 *         character.
 */
int GameGetCurrentRoomTitle(char *title)
{
    strncpy(title, room.title, sizeof(room.title));
    return strlen(room.title);
}

/**
 * GetCurrentRoomDescription() copies the description of the current room into the argument desc as
 * a C-style string with a NULL-terminating character. The room description is guaranteed to be less
 * -than-or-equal to GAME_MAX_ROOM_DESC_LENGTH characters, so the provided argument must be at least
 * GAME_MAX_ROOM_DESC_LENGTH + 1 characters long. Only a NULL-character is copied if there was an
 * error so that the resultant output string length is 0.
 * @param desc A character array to copy the room description into.
 * @return The length of the string stored into `desc`. Note that the actual number of chars
 *          written into `desc` will be this value + 1 to account for the NULL terminating
 *          character.
 */
int GameGetCurrentRoomDescription(char *desc)
{
    strncpy(desc, room.description, sizeof(room.description));
    return strlen(room.description);
}

/**
 * This function returns the exits from the current room in the lowest-four bits of the returned
 * uint8 in the order of NORTH, EAST, SOUTH, and WEST such that NORTH is in the MSB and WEST is in
 * the LSB. A bit value of 1 corresponds to there being a valid exit in that direction and a bit
 * value of 0 corresponds to there being no exit in that direction. The GameRoomExitFlags enum
 * provides bit-flags for checking the return value.
 *
 * @see GameRoomExitFlags
 *
 * @return a 4-bit bitfield signifying which exits are available to this room.
 */
uint8_t GameGetCurrentRoomExits(void)
{
    return room.roomExits;
}

static bool ParseFile(FILE *pFile) {
    uint8_t size;

    if(pFile == NULL) {
        FATAL_ERROR();
        return false;
    }

    fread(&size, sizeof(char), 1, pFile);
    fread(room.title, sizeof(char), size, pFile);
    room.title[size] = '\0';
    fread(&size, sizeof(char), 1, pFile);
    if(size == 0) {
        room.itemRequirements = 0;
    } else {
        fread(&room.itemRequirements, sizeof(char), size, pFile);
    }

    while(!FindInInventory(room.itemRequirements) || room.itemRequirements) {
        fread(&size, sizeof(char), 1, pFile);
        fseek(pFile, size, SEEK_CUR);

        fread(&size, sizeof(char), 1, pFile);
        fread(&room.itemsContained, sizeof(char), size, pFile);

        fseek(pFile, 4, SEEK_CUR);
        fread(&size, sizeof(char), 1, pFile);
        if(size == 0) {
            room.itemRequirements = 0;
        } else {
            fread(&room.itemRequirements, sizeof(char), size, pFile);
        }
    }
    fread(&size, sizeof(char), 1, pFile);
    fread(room.description, sizeof(char), size, pFile);
    room.description[size] = '\0';

    fread(&size, sizeof(char), 1, pFile);
    if(size != 0) {
        fread(&room.itemsContained, sizeof(char), size, pFile);
        AddToInventory(room.itemsContained);
    } else {
        room.itemsContained = 0;
    }

    fread(&room.north, sizeof(uint8_t), 1, pFile);
    fread(&room.east, sizeof(uint8_t), 1, pFile);
    fread(&room.south, sizeof(uint8_t), 1, pFile);
    fread(&room.west, sizeof(uint8_t), 1, pFile);

    room.roomExits = 0;

    if(room.north) {
        room.roomExits |= GAME_ROOM_EXIT_NORTH_EXISTS;
    }
    if(room.east) {
        room.roomExits |= GAME_ROOM_EXIT_NORTH_EXISTS;
    }
    if(room.south) {
        room.roomExits |= GAME_ROOM_EXIT_SOUTH_EXISTS;
    }
    if(room.west) {
        room.roomExits |= GAME_ROOM_EXIT_WEST_EXISTS;
    }

    fclose(pFile);
    return true;
}