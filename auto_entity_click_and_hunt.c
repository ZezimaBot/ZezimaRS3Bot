/*
--------------------------------------------------
    Created by Peter Zezima for RuneScape 3
--------------------------------------------------
	Will auto click entities, and has simple hunt.
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

#define uint unsigned int
#define click_delay 1 //qRand(2, 3)

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
    printf("Created by Peter Zezima for RuneScape 3\n\nL-CTRL + L-ALT = AUTO CLICK ON/OFF\nZ = AUTO HUNT ON/OFF\n\nsudo apt install espeak\n\n");
    speakS("RS3 Bot Started.");

    Display *d;
    int si;
    XEvent event;
    memset(&event, 0x00, sizeof(event));
    uint enable = 0;
    uint hunt = 0;
    uint flip = 0;
    const float base_radius = 54;
    const float max_radius = 280;
    float angle_step = 0.1;
    float radius_step = 0.22;
    float angle = 0;
    float radius = base_radius;
    uint clicks = 0;
    uint delaymicro = 10000;
    time_t nt = time(0)+qRand(1, 6);

    while(1)
    {
        //Loop Delay (1,000 microsecond = 1 millisecond)
        usleep(delaymicro); //10000

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

                enable = 1;
                delaymicro = 10000;
                usleep(300000);
                printf("AUTO-CLICK: ON\n");
                speakS("on");
            }
            else
            {
                XCloseDisplay(d);

                enable = 0;
                printf("AUTO-CLICK: OFF\n");
                speakS("off");
            }
        }

        if(enable == 1 && key_is_pressed(XK_Z))
        {
            if(hunt == 0)
            {
                delaymicro = 10000;
                hunt = 1;
                printf("AUTO-HUNT: ON\n");
                speakS("hunt on");
            }
            else
            {
                hunt = 0;
                printf("AUTO-HUNT: OFF\n");
                speakS("hunt off");
            }
        }

        // if(enable == 1 && key_is_pressed(XK_X))
        // {
        //     delaymicro = 33;
        //     angle_step = 0.44;
        //     radius_step = 0.6;
        //     speakS("FAST");
        // }

        // if(enable == 1 && key_is_pressed(XK_C))
        // {
        //     delaymicro = 10000;
        //     angle_step = 0.1;
        //     radius_step = 0.22;
        //     speakS("REGULAR");
        // }
        
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

            //Auto hunt
            if(hunt == 1)
            {
                const int x = radius * cos(angle);
                const int y = radius * sin(angle);

                XWindowAttributes attr;
                XGetWindowAttributes(d, event.xbutton.window, &attr);
                const int cx = attr.width/2;
                const int cy = attr.height/2;

                XWarpPointer(d, None, event.xbutton.window, 0, 0, 0, 0, cx + x, cy + y);
                XFlush(d);

                angle += angle_step * (1-(radius/max_radius));

                if(flip == 0)
                    radius += radius_step;
                else
                    radius -= radius_step;

                if(radius > max_radius || radius < base_radius)
                {
                    radius = base_radius;
                    flip = 0;
                    
                    //Damn a whole scan and nothing? lets randomly roam
                    if(clicks == 0)
                    {
                        const float na = qRand(0, 58);
                        const int px = max_radius * cos(na);
                        const int py = max_radius * sin(na);
                        XWarpPointer(d, None, event.xbutton.window, 0, 0, 0, 0, cx+px, cy+py);
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

                        //Next time
                        nt = time(0) + click_delay;
                    }

                    clicks = 0;
                }
            }

            //Get Image Block
            const uint cmx = 120, cmy = 3;
            XImage *img = XGetImage(d, event.xbutton.window, event.xbutton.x-60, event.xbutton.y+37, cmx, cmy, AllPlanes, XYPixmap);
            if(img != NULL)
            {
                uint rclick = 0;
                XColor c;
                for(uint y = 0; y < cmy; y++)
                {
                    for(uint x = 0; x < cmx; x++)
                    {
                        c.pixel = XGetPixel(img, x, y);
                        const unsigned char b = c.pixel & img->blue_mask;
                        const unsigned char g = (c.pixel & img->green_mask) >> 8;
                        const unsigned char r = (c.pixel & img->red_mask) >> 16;
                        if(r == 0 && g == 0 && b == 0)
                            rclick++;
                    }
                }
                XFree(img);
                if(rclick == cmx*cmy)
                {
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

                    if(flip == 0)
                    {
                        radius = max_radius;
                        clicks++;
                        flip = 1;
                    }
                    else
                    {
                        radius = base_radius;
                        clicks++;
                        flip = 0;
                    }

                    //Next time
                    nt = time(0) + click_delay;
                }

            }


        }
    }

    //Done, never gets here in regular execution flow
    return 0;
}
