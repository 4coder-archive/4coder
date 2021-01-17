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
system_error_box(char *msg){
    //LOGF("error box: %s\n", msg);
    osx_error_dialogue(msg);
    exit(1);
}

// BOTTOM

