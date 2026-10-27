#include <stdio.h>
#include <stdlib.h> // rand

#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"
#include "missile.h"
#include "plane.h"
#include "game.h"
#include "config.h"

#define SILO1 (LCD_W / 4)
#define SILO2 (LCD_W / 2)
#define SILO3 (LCD_W * (3/4))

#define ZONE1 (LCD_W * (3/8))
#define ZONE2 (LCD_W * (5/8))
#define ZONE3 (LCD_W)

#define ENEMY_Y_LAUNCH (LCD_H / 4)


// state enum
enum state {idle, moving, explodingGrowing, explodingShrinking, impacted};

/******************** Missile Init & Launch Functions ********************/

// Different _launch_ functions are used depending on the missile type.

// Initialize the missile as an idle missile. When initialized to the idle
// state, a missile doesn't appear nor does it move. The launch flag should
// also be set to false. Other missile_t members will be set up at launch.
void missile_init(missile_t *missile)
{
    missile->currentState = idle;
    missile->launch = false;
    missile->explode_me = false
}


// Launch the missile as a player missile. This function takes an (x, y)
// destination of the missile (as specified by the user). The origin is the
// closest "firing location" to the destination (there are three firing
// locations evenly spaced along the bottom of the screen).
void missile_launch_player(missile_t *missile, coord_t x_dest, coord_t y_dest)
{
    missile->type = MISSILE_TYPE_PLAYER;
    missile->x_dest = x_dest;
    missile->y_dest = y_dest;

    if(x_dest <= ZONE1) missile->x_origin = SILO1;
    else if(x_dest <= ZONE2) missile->x_origin = SILO2;
    else missile->x_origin = SILO3;
    missile->y_origin = LCD_H;

    finalize_launch(missile);

}

// Launch the missile as an enemy missile. This will randomly choose the
// origin and destination of the missile. The origin is somewhere near the
// top of the screen, and the destination is the very bottom of the screen.
void missile_launch_enemy(missile_t *missile)
{
    missile->type = MISSILE_TYPE_ENEMY;

    missile->x_origin = rand() % LCD_W;
    missile->y_origin = rand() % ENEMY_Y_LAUNCH;
     
    missile->x_dest = rand() % LCD_W;
    missile->y_dest = LCD_H;


    finalize_launch(missile);
}

// Launch the missile as a plane missile. This function takes the (x, y)
// location of the plane as an argument and uses it as the missile origin.
// The destination is randomly chosen along the bottom of the screen.
void missile_launch_plane(missile_t *missile, coord_t x_orig, coord_t y_orig)
{
    missile->type = MISSILE_TYPE_PLANE;

    missile->x_origin = x_orig;
    missile->y_origin = y_orig;
     
    missile->x_dest = rand() % LCD_W;
    missile->y_dest = LCD_H;

    finalize_launch(missile);

}

/******************** Missile Control & Tick Functions ********************/

// Used to indicate that a moving missile should be detonated. This occurs
// when an enemy or a plane missile is located within an explosion zone.
void missile_explode(missile_t *missile)
{  
    missile->explode_me = true;
}

// Tick the state machine for a single missile.
void missile_tick(missile_t *missile)
{
    // ************ TRANSITION ***************
    switch (missile->currentState)
    {
        case idle:
            if (missile->launch) missile->currentState = moving;
        
        break;
        
        case moving:
            if(missile->explode_me) missile->currentState = explodingGrowing;
            else if(missile_is_impacted(missile)) missile->currentState = impacted;
        
        break;
        
        case explodingGrowing:
            // ***************** GET WORKING HERE ********************
        
        break;
        
        case explodingShrinking:
        
        break;
        
        case impacted:

        break;
    }

    // ************ ACTION ***************
    switch (missile->currentState)
    {
        case idle:
            // check if launch
        
        break;
        
        case moving:
        
        break;
        
        case explodingGrowing:
        
        break;
        
        case explodingShrinking:
        
        break;
        
        case impacted:

        break;
    }


}

/******************** Missile Status Functions ********************/

// Return the current missile position through the pointers *x,*y.
void missile_get_pos(missile_t *missile, coord_t *x, coord_t *y)
{
    x = missile->x_current;
    y = missile->y_current;
}

// Return the missile type.
missile_type_t missile_get_type(missile_t *missile)
{
    return missile->type;
}

// Return whether the given missile is moving.
bool missile_is_moving(missile_t *missile)
{
    return missile->currentState == moving;
}

// Return whether the given missile is exploding. If this missile
// is exploding, it can explode another intersecting missile.
bool missile_is_exploding(missile_t *missile)
{   
    return (missile->currentState == explodingGrowing) || (missile->currentState == explodingShrinking);
}

// Return whether the given missile is idle.
bool missile_is_idle(missile_t *missile)
{
    return missile->currentState == idle;
}

// Return whether the given missile is impacted.
bool missile_is_impacted(missile_t *missile)
{
    return (missile->y_current >= LCD_H);
}

// Return whether an object (e.g., missile or plane) at the specified
// (x,y) position is colliding with the given missile. For a collision
// to occur, the missile needs to be exploding and the specified
// position needs to be within the explosion radius.
bool missile_is_colliding(missile_t *missile, coord_t x, coord_t y)
{
    if(!missile_is_exploding(missile)) return false;
    
    float dist = (x - missile->x_current) * (x - missile->x_current) + (y - missile->y_current) * (y - missile->y_current);
    float radius2 = missile->radius * missile->radius;
    return (dist <= radius2) ? true : false;

}


/******************** Missile Helper Functions ********************/


// helper funtion to set all the launch setups for each missile type
static void finalize_launch(missile_t *missile)
{
    missile->length = 0;
    missile->launch = true; 
    missile->explode_me = false;
    missile->x_current = x_origin;
    missile->y_current = y_origin;

    // logic to get total distance
    float ydist = missile->y_origin - missile->y_current;
    float xdist = missile->x_origin - missile->x_current;
    missile->total_length = sqrtf((ydist * ydist) + (xdist * xdist));
}