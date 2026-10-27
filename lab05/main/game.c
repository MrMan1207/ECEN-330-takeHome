// Header files for the game in ecen330/lab05/main
#include "config.h" // - game configuration, colors, board size, etc.
#include "board.h" // - keeps track of marks on the board
#include "graphics.h" // - functions to draw the grid, X's, O's, and messages
#include "nav.h" // - a grid navigator used to select a space to mark
#include "com.h" // - a communication channel for sending and receiving bytes (you will implement .c file)
#include "game.h" // - state machine implementing the game (you will implement .c file)
// Header  files for existing components in ecen330/components
#include "hw.h" // - hardware mapping of buttons to IO pins
#include "lcd.h" // - display drawing primitives
#include "joy.h" //- get joystick displacement
#include "pin.h" // - configure, get, and set IO pins

#include <stdio.h>
    

#define BUF_C_MASK 0xF
#define R_BIT_SHIFT 4
#define RESTART_BYTE 0xFF

// global variables
enum state {init_st, new_game_st, wait_mark_st, mark_st, wait_restart_st} curr_state;
int8_t r, c;
mark_t curr_mark;
uint8_t mark_flag, end_flag, restart_flag, buf_write, buf_read;


// Initialize the game logic.
void game_init(void)
{   
    // state starts in init_st
    curr_state = init_st;
}

// Update the game logic.
void game_tick(void)
{
    // ******************* state tranistion *********************
    switch(curr_state)
    {
        case init_st:  // transition
            // no action or check, just staight to start
            curr_state = new_game_st;
            break;

        case new_game_st:  // transition
            // set the flags and x to start
            mark_flag = 0;
            end_flag = 0;
            restart_flag = 0;
            curr_mark = X_m;

            // clear the buffers
            while(com_read(&buf_read,1) > 0);
            buf_read = 0;
            buf_write = 0;

            curr_state = wait_mark_st;

            // mealy show the message for x to start game
            char *str = "X to start";
            graphics_drawMessage(str, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            break;        

        case  wait_mark_st:  // transition
            // check if com_read
            if (com_read(&buf_read, 1) == 1)
            {
                // unpack the byte
                r = buf_read >> 4;
                c = (buf_read & BUF_C_MASK);
                // check if valid place, only go to place mark if so
                if(board_set(r, c, curr_mark)) curr_state = mark_st;
            }

            // if not recieved, get the place then send data 
            else if(mark_flag) 
            {
                curr_state = mark_st;

                // send bytes
                buf_write = (r << R_BIT_SHIFT);
                buf_write |= (c & BUF_C_MASK);

                com_write(&buf_write, 1);
            }
            
            mark_flag = 0;
            break;      

        case  mark_st:  // transition
            // change the state to restart on flag or next mark on not end
            curr_state = (end_flag) ? wait_restart_st : wait_mark_st;

            // reset the buffers to clear them
            buf_read = 0;
            buf_write = 0;

            // mealy show the message for next player
            if(!end_flag)
            {
                char *str = (curr_mark == X_m) ? "X, your turn" : "O, your turn";
                graphics_drawMessage(str, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            }          
            break;    

        case  wait_restart_st:  // transition
            // restart checker 
            uint8_t res = 0x00;

            // check for restart byte
            if (com_read(&buf_read, 1) == 1)
            {
                res = buf_read;
                if(res == RESTART_BYTE) curr_state = new_game_st;
            }            
           // watch the flag for restart button, then sned the restart byte
            else if(restart_flag)
            {
                curr_state = new_game_st;
                buf_write = RESTART_BYTE;

                com_write(&buf_write, 1);
            } 
            break;
    }




    // ********************** state action ************************
    switch(curr_state)
    {
        case init_st:   // action
            // no action 
            break;

        case new_game_st:   // action
            // draw a clear screen
            lcd_fillScreen(CONFIG_BACK_CLR);
            graphics_drawGrid(CONFIG_GRID_CLR);
            board_clear();
            // set curr_mark and loc to center
            curr_mark = X_m;
            nav_set_loc(1, 1);
            break;    

        case  wait_mark_st:   // action
            // wait for the A button to place mark
            if(!pin_get_level(HW_BTN_A))
            {
                nav_get_loc(&r, &c);
                // only set mark if none is there
                if (board_get(r, c) == no_m) 
                {
                    board_set(r, c, curr_mark);
                    mark_flag = 1;
                }
            } 
            break;

        case  mark_st:   // action
            // Draw an X or O on the display
            if(curr_mark == X_m) graphics_drawX(r, c, CONFIG_MARK_CLR);
            if(curr_mark == O_m) graphics_drawO(r, c, CONFIG_MARK_CLR);

            // Check the board for a win and write who wins
            if (board_winner(curr_mark)) 
            {
                end_flag = 1;

                char *str = (curr_mark == X_m) ? "X WINS" : "O WINS";
                graphics_drawMessage(str, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            }

            // check the board for a draw and write if draw
            else if(board_mark_count() == CONFIG_BOARD_SPACES) 
            {
                end_flag = 1;

                char *str = "Its a draw";
                graphics_drawMessage(str, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            }

            // flip the mark
            curr_mark = (curr_mark == X_m) ? O_m : X_m;
            break;        

        case  wait_restart_st:   // action
            // wait for the start button to restart
            if(!pin_get_level(HW_BTN_START)) restart_flag = 1;
            break;

    }
}