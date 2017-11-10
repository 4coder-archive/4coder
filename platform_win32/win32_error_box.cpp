/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Windows fatal error message box.
 *
 */

// TOP

internal void
system_error_box(char *msg, b32 shutdown = true){
    LOGF("error box: %s\n", msg);
    MessageBox_utf8(0, (u8*)msg, (u8*)"Error", 0);
    if (shutdown){
        exit(1);
    }
}

// BOTTOM

