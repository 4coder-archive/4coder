enum Action_Type{
    DACT_OPEN,
    DACT_SAVE_AS,
    DACT_SAVE,
    DACT_NEW,
    DACT_SWITCH,
    DACT_TRY_KILL,
    DACT_KILL,
    DACT_CLOSE_MINOR,
    DACT_CLOSE_MAJOR,
    DACT_THEME_OPTIONS,
    DACT_KEYBOARD_OPTIONS,
};

struct Delayed_Action{
    Action_Type type;
    String string;
    Panel* panel;
    Editing_File* file;
};

struct Delay{
    Delayed_Action* acts;
    i32 count;
    i32 max;
};

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type){
    Delayed_Action *result;
    Assert(delay->count < delay->max);
    result = delay->acts + delay->count++;
    *result = {};
    result->type = type;
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
delayed_action_(Delay *delay, Action_Type type, String string, Panel* panel){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->string = string;
    result->panel = panel;
    return(result);
}

inline Delayed_Action*
delayed_action_(Delay *delay, Action_Type type, String string, Editing_File* file){
    Delayed_Action *result;
    result = delayed_action_(delay, type);
    result->string = string;
    result->file = file;
    return(result);
}

#define delayed_open(delay, ...) delayed_action_(delay, DACT_OPEN, __VA_ARGS__)
#define delayed_save_as(delay, ...) delayed_action_(delay, DACT_SAVE_AS, __VA_ARGS__)
#define delayed_save(delay, ...) delayed_action_(delay, DACT_SAVE, __VA_ARGS__)
#define delayed_new(delay, ...) delayed_action_(delay, DACT_NEW, __VA_ARGS__)
#define delayed_switch(delay, ...) delayed_action_(delay, DACT_SWITCH, __VA_ARGS__)
#define delayed_try_kill(delay, ...) delayed_action_(delay, DACT_TRY_KILL, __VA_ARGS__)
#define delayed_kill(delay, ...) delayed_action_(delay, DACT_KILL, __VA_ARGS__)
#define delayed_close_minor(delay, ...) delayed_action_(delay, DACT_CLOSE_MINOR, __VA_ARGS__)
#define delayed_close_major(delay, ...) delayed_action_(delay, DACT_CLOSE_MAJOR, __VA_ARGS__)
#define delayed_theme_options(delay, ...) delayed_action_(delay, DACT_THEME_OPTIONS, __VA_ARGS__)
#define delayed_keyboard_options(delay, ...) delayed_action_(delay, DACT_KEYBOARD_OPTIONS, __VA_ARGS__)
