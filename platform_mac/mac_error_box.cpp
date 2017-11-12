/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 26.09.2017
 *
 * Mac error box implementation.
 *
 */

// TOP

internal void
system_error_box(char *msg, b32 shutdown = true){
    LOGF("error box: %s\n", msg);
    osx_error_dialogue(msg);
    if (shutdown){
        exit(1);
    }
}

// BOTTOM

