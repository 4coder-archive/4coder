/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.02.2016
 *
 * GUI system for 4coder
 *
 */

// TOP

#ifndef FRED_GUI_H
#define FRED_GUI_H

struct Query_Slot{
    Query_Slot *next;
    Query_Bar *query_bar;
};

struct Query_Set{
    Query_Slot slots[8];
    Query_Slot *free_slot;
    Query_Slot *used_slot;
};

#endif

// BOTTOM

