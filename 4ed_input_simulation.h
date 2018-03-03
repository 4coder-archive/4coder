/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.03.2018
 *
 * Input simulation data declarations
 *
 */

// TOP

#if !defined(FRED_INPUT_SIMULATION_H)
#define FRED_INPUT_SIMULATION_H

#include "4ed_input_simulation_event.h"

////////////////

struct Input_Simulation_Controls{
    b32 enforce_regular_mouse;
    i32 counter;
    Mouse_State prev_mouse;
};

////////////////

struct Simulation_Event_Stream_State{
    i32 index;
};

#endif

// BOTTOM

