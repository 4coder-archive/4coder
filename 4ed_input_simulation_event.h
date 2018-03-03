/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.03.2018
 *
 * Input simulation data declarations ~ events data
 *
 */

// TOP

#if !defined(FRED_INPUT_SIMULATION_EVENT_H)
#define FRED_INPUT_SIMULATION_EVENT_H

typedef u32 Simulation_Event_Type;
enum{
    SimulationEvent_Noop,
    SimulationEvent_DebugNumber,
    SimulationEvent_Key,
    SimulationEvent_MouseLeftPress,
    SimulationEvent_MouseLeftRelease,
    SimulationEvent_MouseRightPress,
    SimulationEvent_MouseRightRelease,
    SimulationEvent_MouseWheel,
    SimulationEvent_MouseXY,
    SimulationEvent_Exit,
};

struct Simulation_Event{
    i32 counter_index;
    Simulation_Event_Type type;
    union{
        i32 debug_number;
        struct{
            u32 code;
            u8 modifiers;
        } key;
        i32 wheel;
        struct{
            i32 x;
            i32 y;
        } mouse_xy;
    };
};

#endif

// BOTTOM

