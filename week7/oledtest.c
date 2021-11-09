/******************************************************************************
 
  Filename: oledtest.c
 
    Simple test program for the OLED panel on the Matrix CK
    
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "libmc-oled.h"

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
#define BUFLEN 20

const struct {
	char *name;
	unsigned int code;
} color_code[] =
{{"black", ST7735_BLACK},
 {"blue", ST7735_BLUE},
 {"red", ST7735_RED},
 {"green", ST7735_GREEN},
 {"cyan", ST7735_CYAN},
 {"magenta", ST7735_MAGENTA},
 {"yellow", ST7735_YELLOW},
 {"white", ST7735_WHITE},
 {NULL, 0}};

int running = 1;
const char *delims = " \t,=";

unsigned int get_color (char *name)
{
	int i = 0;

	while (color_code[i].name)
	{
		if (!strcmp (name, color_code[i].name))
			return color_code[i].code;
		i++;
	}
	return 0;
}
int main (int argc, char *argv[])
{
	unsigned int row, col, width, height;
    unsigned int x_start, y_start;
    int value;
    char text[BUFLEN], string[BUFLEN];
/*
    Initialization
*/
    if (OLEDInit() == -1)
        FATAL;
/*
    Command loop
*/
    while (running)
    {
        printf ("OLED>");
        fgets (text, BUFLEN, stdin);

        switch (*text)
        {
            case 'b':   // set background color
                sscanf (text+2, "%s", string);
                BGcolor (get_color (string));
                break;
                    
            case 'f':   // set foreground color
                sscanf (text+2, "%s", string);
                FGcolor (get_color (string));
                break;
                    
            case 's':   // write string
                sscanf (text+2, "%d %d %s", &row, &col, string);
                if (OLEDDrawString (row, col, string) != strlen (string))
                    printf ("Part of string outside visible panel\n");
                break;
                    
            case 'n':   // write number
                sscanf (text+2, "%d %d %d %d", &row, &col, &value, &width);
                OLEDWriteNumber (row, col, value, width);
                break;

            case 'r':   // fill a rectangle
                sscanf (text+2, "%d %d %d %d", &x_start, &y_start, &width, &height);
                OLEDFillRect (x_start, y_start, width, height);
                break;
                    
            case 'q':   // quit program
                running = 0;
                break;
                
            default:
                continue;
        }
    }       
    OLEDDeInit();
    return 0;
}
