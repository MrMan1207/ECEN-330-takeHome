#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define DELAY_MS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

// TODO: Finish car part constants
#define CAR_W 60
#define CAR_H 32

// big rect
#define BODY_X0 0
#define BODY_Y0 12
#define BODY_X1 59
#define BODY_Y1 24

// small rect
#define CAB_X0 1
#define CAB_Y0 0
#define CAB_X1 39
#define CAB_Y1 11

// hood
#define HOOD_X0 40
#define HOOD_Y0 9
#define HOOD_X1 40
#define HOOD_Y1 11
#define HOOD_X2 59
#define HOOD_Y2 11

// windowL
#define WINDOWL_X0 3
#define WINDOWL_Y0 1
#define WINDOWL_X1 18
#define WINDOWL_Y1 8

// windowR
#define WINDOWR_X0 21
#define WINDOWR_Y0 1
#define WINDOWR_X1 37
#define WINDOWR_Y1 8

// wheels
#define R_HUB 4
#define R_TIRE 7

// wheelL
#define WHEELL_X0 11
#define WHEELL_Y0 24
// wheelR
#define WHEELR_X0 48
#define WHEELR_Y0 24





/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{ // function to draw car 
	// TODO: Implement car procedurally with lcd geometric primitives.
	lcd_fillRect2 (x + BODY_X0, y + BODY_Y0, x + BODY_X1, y + BODY_Y1, CAR_CLR);
	lcd_fillRect2 (x + CAB_X0, y + CAB_Y0, x + CAB_X1, y + CAB_Y1, CAR_CLR);
	lcd_fillTriangle (x + HOOD_X0, y + HOOD_Y0, x + HOOD_X1, y + HOOD_Y1, x + HOOD_X2, y + HOOD_Y2, CAR_CLR);
	lcd_fillRoundRect2 (x + WINDOWL_X0, y + WINDOWL_Y0, x + WINDOWL_X1, y + WINDOWL_Y1, 2, WINDOW_CLR);
	lcd_fillRoundRect2 (x + WINDOWR_X0, y + WINDOWR_Y0, x + WINDOWR_X1, y + WINDOWR_Y1, 2, WINDOW_CLR);
	lcd_fillCircle (x + WHEELL_X0, y + WHEELL_Y0, R_TIRE, TIRE_CLR);
	lcd_fillCircle (x + WHEELR_X0, y + WHEELR_Y0, R_TIRE, TIRE_CLR);
	lcd_fillCircle (x + WHEELL_X0, y + WHEELL_Y0, R_HUB, HUB_CLR);
	lcd_fillCircle (x + WHEELR_X0, y + WHEELR_Y0, R_HUB, HUB_CLR);
}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels



// Application main
void app_main(void)
{
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString (0, 0, "Hello World!", TITLE_CLR);
	DELAY_MS(WAIT);


	// TODO: Exercise 1 - Draw car in one location.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString (0, 0, "Exercise 1", TITLE_CLR);
	drawCar(OBJ_X, OBJ_Y);
	DELAY_MS(WAIT);

	// TODO: Exercise 2 - Draw moving car (Method 1), one pass across display.
	// Clear the entire display and redraw all objects each iteration.
	// Use a loop and increment x by OBJ_MOVE each iteration.
	// Start x off screen (negative coordinate).
	lcd_fillScreen(BACKGROUND_CLR);
	char str[STR_BUF_LEN];

	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{ // draws and moves car by coloring over whole screen
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString (0, 0, "Exercise 2", TITLE_CLR);
		drawCar(x, OBJ_Y);
		sprintf(str, "%3ld", x);
		lcd_drawString (0, LCD_H - FONT_H, str, STATUS_CLR);
	}


	// TODO: Exercise 3 - Draw moving car (Method 2), one pass across display.
	// Move by erasing car at old position, then redrawing at new position.
	// Objects that don't change or move are drawn once.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString (0, 0, "Exercise 3", TITLE_CLR);

	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{ // moves and draws car by coloring over old car
		lcd_fillRect2 (x - OBJ_MOVE, OBJ_Y, x + CAR_W - OBJ_MOVE, OBJ_Y + CAR_H, BACKGROUND_CLR);
		lcd_fillRect2 (0, LCD_H - FONT_H, STATUS_W, LCD_H, BACKGROUND_CLR);
		drawCar(x, OBJ_Y);
		sprintf(str, "%3ld", x);
		lcd_drawString (0, LCD_H - FONT_H, str, STATUS_CLR);
		DELAY_MS(20);
	}


	// TODO: Exercise 4 - Draw moving car (Method 3), one pass across display.
	// First, draw all objects into a cleared, off-screen frame buffer.
	// Then, transfer the entire frame buffer to the screen.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_frameEnable ();

	for (coord_t x = -CAR_W; x <= LCD_W; x += OBJ_MOVE)
	{ // draws and moves car through buffer frame
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString (0, 0, "Exercise 4", TITLE_CLR);
		drawCar(x, OBJ_Y);
		sprintf(str, "%3ld", x);
		lcd_drawString (0, LCD_H - FONT_H, str, STATUS_CLR);
		lcd_writeFrame();
	}

	lcd_frameDisable ();

	// TODO: Exercise 5 - Draw an animated Pac-Man moving across the display.
	// Use Pac-Man sprites instead of the car object.
	// Cycle through each sprite when moving the Pac-Man character.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_frameEnable ();
	uint16_t i = 0;
	const uint8_t pidx[] = {0, 1, 2, 1};

	for (;;) 
	{ // forever loop to always loop pacman
		for (coord_t x = -PAC_W; x <= LCD_W; x += OBJ_MOVE)
		{ // draws and moves pac man
			lcd_fillScreen(BACKGROUND_CLR);
			lcd_drawString (0, 0, "Exercise 5", TITLE_CLR);
			lcd_drawBitmap (x, OBJ_Y, pac[pidx[i++ % sizeof(pidx)]], PAC_W, PAC_H, YELLOW);
			sprintf(str, "%3ld", x);
			lcd_drawString (0, LCD_H - FONT_H, str, STATUS_CLR);
			lcd_writeFrame();
		}
	}

	lcd_frameDisable ();
}
