/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Linux fatal error message box.
 *
 */

// TOP

internal void
system_error_box(char *msg){
    LOGF("Fatal Error: %s\n", msg);
    MessageBox_utf8(0, (u8*)msg, (u8*)"Error",0);
    exit(1);
}

// BOTTOM

