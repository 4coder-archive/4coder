enum Action_Type{
    DACT_SET_LINE,
    DACT_SWITCH,
    DACT_TRY_KILL,
    DACT_KILL,
    DACT_TOUCH_FILE,
    DACT_CLOSE,
};

struct Delayed_Action{
    Action_Type type;
    String string;
    Panel* panel;
    Editing_File* file;
    i32 integer;
};

struct Delay{
    General_Memory* general;
    Delayed_Action* acts;
    i32 count;
    i32 max;
};

internal String
str_alloc_copy(General_Memory *general, String str){
    String result;
    result.memory_size = str.memory_size + 1;
    result.size = str.size;
    result.str = (char*)general_memory_allocate(general, result.memory_size, 0);
    memcpy(result.str, str.str, str.size);
    result.str[result.size] = 0;
    return(result);
}

inline Delayed_Action
delayed_action_zero(){
    Delayed_Action result = {(Action_Type)0};
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type){
    Delayed_Action *result;
    if (delay->count == delay->max){
        delay->max *= 2;
        delay->acts = (Delayed_Action*)general_memory_reallocate(delay->general, delay->acts, delay->count*sizeof(Delayed_Action), delay->max*sizeof(Delayed_Action), 0);
    }
    result = delay->acts + delay->count++;
    *result = delayed_action_zero();
    result->type = type;
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, String string){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->string = str_alloc_copy(delay->general, string);
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, Panel* panel){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->panel = panel;
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, Editing_File* file){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->file = file;
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, Editing_File* file, Panel* panel){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->file = file;
    result->panel = panel;
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, String string, Panel* panel){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->string = str_alloc_copy(delay->general, string);
    result->panel = panel;
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, String string, Editing_File* file){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->string = str_alloc_copy(delay->general, string);
    result->file = file;
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, Panel* panel, i32 integer){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->panel = panel;
    result->integer = integer;
    return(result);
}

inline Delayed_Action*
delayed_action_repush(Delay *delay, Delayed_Action *act){
    Delayed_Action *new_act = delayed_action_(delay, (Action_Type)0);
    *new_act = *act;
    if (act->string.str){
        new_act->string = str_alloc_copy(delay->general, act->string);
    }
    return(new_act);
}

#define delayed_set_line(delay, ...) delayed_action_(delay, DACT_SET_LINE, ##__VA_ARGS__)
#define delayed_switch(delay, ...) delayed_action_(delay, DACT_SWITCH, ##__VA_ARGS__)
#define delayed_try_kill(delay, ...) delayed_action_(delay, DACT_TRY_KILL, ##__VA_ARGS__)
#define delayed_kill(delay, ...) delayed_action_(delay, DACT_KILL, ##__VA_ARGS__)
#define delayed_touch_file(delay, ...) delayed_action_(delay, DACT_TOUCH_FILE, ##__VA_ARGS__)
#define delayed_close(delay, ...) delayed_action_(delay, DACT_CLOSE, ##__VA_ARGS__)
