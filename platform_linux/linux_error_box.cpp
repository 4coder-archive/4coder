/*
 * Mr. 4th Dimention - Allen Webster
 *  (Mostly by insofaras)
 *
 * 18.07.2017
 *
 * Linux fatal error message box.
 *
 */

// TOP

// HACK(allen): // NOTE(inso): this was a quick hack, might need some cleanup.
internal void
system_error_box(char *msg){
    fprintf(stderr, "Fatal Error: %s\n", msg);
    //LOGF("Fatal Error: %s\n", msg);
    
    Display *dpy = XOpenDisplay(0);
    if (!dpy){
        exit(1);
    }
    
    const int num_cols = 50;
    int win_w = (num_cols + 10) * 9;
    int win_h = 140;
    
    {
        const char *start_p = msg, *space_p = NULL;
        for(const char* p = msg; *p; ++p){
            if (*p == ' ') space_p = p;
            if (*p == '\n' || p - start_p > num_cols){
                win_h += 18;
                start_p = space_p ? space_p + 1 : p;
                space_p = NULL;
            }
        }
    }
    
    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, win_w, win_h, 0, 0, 0x227A3B);
    XStoreName(dpy, w, "4coder Error");
    
    XSizeHints* sh = XAllocSizeHints();
    sh->flags = PMinSize;
    sh->min_width = win_w;
    sh->min_height = win_h;
    XSetWMNormalHints(dpy, w, sh);
    
    Atom type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    
    XChangeProperty(dpy, w, XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False), XA_ATOM, 32, PropModeReplace, (unsigned char*) &type, 1);
    
    Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &WM_DELETE_WINDOW, 1);
    
    linux_set_icon(dpy, w);
    
    XMapRaised(dpy, w);
    XSync(dpy, False);
    
    XSelectInput(dpy, w, ExposureMask | StructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask);
    
    XFontStruct* font = XLoadQueryFont(dpy, "-*-fixed-bold-*-*-*-*-140-*-*-*-*-iso8859-1");
    if (!font){
        exit(1);
    }
    
    XGCValues gcv;
    gcv.foreground = WhitePixel(dpy, 0);
    gcv.line_width = 2;
    gcv.font = font->fid;
    
    GC gc1 = XCreateGC(dpy, w, GCForeground | GCFont | GCLineWidth, &gcv);
    gcv.foreground = BlackPixel(dpy, 0);
    GC gc2 = XCreateGC(dpy, w, GCForeground | GCFont | GCLineWidth, &gcv);
    
    int button_trigger = 0;
    int button_hi = 0;
    int redraw = 1;
    
    XEvent ev;
    while (1){
        XNextEvent(dpy, &ev);
        
        if (ev.type == Expose) redraw = 1;
        
        if (ev.type == ConfigureNotify){
            redraw = 1;
            win_w = ev.xconfigure.width;
            win_h = ev.xconfigure.height;
        }
        
        XRectangle button_rect = { (short)(win_w/2-40), (short)(win_h*0.8f), 80, 20 };
        
        if (ev.type == MotionNotify){
            int new_hi = (ev.xmotion.x > button_rect.x &&
                          ev.xmotion.y > button_rect.y &&
                          ev.xmotion.x < button_rect.x + button_rect.width &&
                          ev.xmotion.y < button_rect.y + button_rect.height);
            
            if (new_hi != button_hi){
                button_hi = new_hi;
                redraw = 1;
            }
        }
        
        if (ev.type == KeyPress){
            KeySym sym = XLookupKeysym(&ev.xkey, 0);
            if (sym == XK_Escape || sym == XK_Return){
                exit(1);
            }
        }
        
        if (ev.type == ButtonPress && ev.xbutton.button == Button1){
            if (button_hi) button_trigger = 1;
            redraw = 1;
        }
        
        if (ev.type == ButtonRelease && ev.xbutton.button == Button1){
            if (button_trigger){
                if (button_hi){
                    exit(1);
                } else {
                    button_trigger = 0;
                }
            }
            redraw = 1;
        }
        
        if (ev.type == ClientMessage && ev.xclient.window == w && (Atom)ev.xclient.data.l[0] == WM_DELETE_WINDOW){
            exit(1);
        }
        
#define DRAW_STR(x, y, str, len) \
XDrawString(dpy, w, gc2, (x)+1, (y)+1, (str), (len)); \
XDrawString(dpy, w, gc1, (x)  , (y)  , (str), (len))
        
        if (redraw){
            redraw = 0;
            XClearWindow(dpy, w);
            
            const char* line_start = msg;
            const char* last_space = NULL;
            int y = 30;
            
            {
                const char title[] = "4coder - Fatal Error";
                int width = XTextWidth(font, title, sizeof(title)-1);
                int x = (win_w/2) - (width/2);
                DRAW_STR(x, y, title, sizeof(title)-1);
            }
            
            y += 36;
            int width = XTextWidth(font, "x", 1) * num_cols;
            int x = (win_w/2) - (width/2);
            
            for(const char* p = line_start; *p; ++p){
                if (*p == ' ') last_space = p;
                if (p - line_start > num_cols || *p == '\n' || !p[1]){
                    
                    const char* new_line_start = last_space + 1;
                    if (!last_space || *p == '\n' || !p[1]){
                        new_line_start = last_space = (p + !p[1]);
                    }
                    
                    DRAW_STR(x, y, line_start, last_space - line_start);
                    
                    line_start = new_line_start;
                    last_space = NULL;
                    y += 18;
                }
            }
            
            XDrawRectangles(dpy, w, gc1, &button_rect, 1);
            if (button_hi || button_trigger){
                XDrawRectangle(dpy, w, gc2, button_rect.x+1, button_rect.y+1, button_rect.width-2, button_rect.height-2);
            }
            
            DRAW_STR(button_rect.x + 20, button_rect.y + 15, "Drat!", 5);
        }
    }
#undef DRAW_STR
    
    exit(1);
}

// BOTTOM

