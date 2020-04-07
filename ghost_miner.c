/*
--------------------------------------------------
    Created by Peter Zezima for RuneScape 3
--------------------------------------------------
	Will ghost mine a rock at max stamina
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/***************************************************
   ~~ Utils
*/
uint qRand(const uint min, const uint max)
{
    static time_t ls = 0;
    if(time(0) > ls)
    {
        srand(time(0));
        ls = time(0) + 33;
    }
    const int rv = rand();
    if(rv == 0)
        return min;
    return ( ((float)rv / RAND_MAX) * (max-min) ) + min;
}

int key_is_pressed(KeySym ks)
{
    Display *dpy = XOpenDisplay(":0");
    char keys_return[32];
    XQueryKeymap(dpy, keys_return);
    KeyCode kc2 = XKeysymToKeycode(dpy, ks);
    int isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
    XCloseDisplay(dpy);
    return isPressed;
}

void speakS(const char* text)
{
    char s[256];
    sprintf(s, "/usr/bin/espeak \"%s\"", text);
    if(system(s) <= 0)
        sleep(1);
}

/***************************************************
   ~~ Program Entry Point
*/
int main()
{
    printf("Created by Peter Zezima for RuneScape 3\n\nL-CTRL + L-ALT = AUTO CLICK ON/OFF\n\nThis is a silent click auto-clicker, it will click at the coordinate that you enabled the bot. As long as the RS window is focused, you will be able to click anywhere else on screen while it clicks.\n\n");
    speakS("RS3 Miner Bot Started.");

    Display *d;
    int si;
    unsigned int lx=0, ly=0;

    XEvent event;
    memset(&event, 0x00, sizeof(event));
    
    unsigned int enable = 0;
    time_t nt = time(0)+qRand(1, 6);

    while(1)
    {
        //Loop Delay (1,000 microsecond = 1 millisecond)
        usleep(100000); //qRand(4000000, 8000000)

        //Inputs / Keypress
        if(key_is_pressed(XK_Control_L) && key_is_pressed(XK_Alt_L))
        {
            if(enable == 0)
            {
                //Open Display 0
                d = XOpenDisplay((char *) NULL);
                if(d == NULL)
                    continue;

                //Get default screen
                si = XDefaultScreen(d);

                lx=0,ly=0;

                enable = 1;
                usleep(300000);
                printf("S-MINE: ON\n");
                speakS("on");
            }
            else
            {
                XCloseDisplay(d);

                lx=0,ly=0;

                enable = 0;
                printf("S-MINE: OFF\n");
                speakS("off");
            }
        }

        if(enable == 1 && key_is_pressed(XK_Z))
        {
            lx = event.xbutton.x;
            ly = event.xbutton.y;
            speakS("set");
        }
        
        //Enable / Disable entire bot
        if(enable == 1 && time(0) > nt)
        {
            //Reset mouse event
            memset(&event, 0x00, sizeof(event));

            //Ready to press down mouse 1
            event.type = ButtonPress;
            event.xbutton.button = Button1;
            event.xbutton.same_screen = True;
            
            //Find target window
            XQueryPointer(d, RootWindow(d, si), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
            event.xbutton.subwindow = event.xbutton.window;
            while(event.xbutton.subwindow)
            {
                event.xbutton.window = event.xbutton.subwindow;
                XQueryPointer(d, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
            }

            //Set starting position for click
            if(lx == 0)
            {
                lx = event.xbutton.x;
                ly = event.xbutton.y;
            }
            const int ox = event.xbutton.x, oy = event.xbutton.y;
            XWarpPointer(d, None, event.xbutton.window, 0, 0, 0, 0, lx, ly);
            XFlush(d);

            //Fire mouse down
            XSendEvent(d, PointerWindow, True, 0xfff, &event);
            XFlush(d);
            
            //Wait between 80 - 220 ms for humanisation
            usleep(qRand(80, 220));
            
            //Release mouse down
            event.type = ButtonRelease;
            event.xbutton.state = 0x100;
            XSendEvent(d, PointerWindow, True, 0xfff, &event);
            XFlush(d);

            //Move cursor back
            XWarpPointer(d, None, event.xbutton.window, 0, 0, 0, 0, ox, oy);
            XFlush(d);

            //Next time
            nt = time(0) + qRand(1, 3);
        }
    }

    //Done, never gets here in regular execution flow
    return 0;
}
