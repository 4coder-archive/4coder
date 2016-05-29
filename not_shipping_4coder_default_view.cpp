
struct Custom_Vars{
    int initialized;
    Partition part;
};

enum View_Mode{
    ViewMode_File,
};

struct View_Vars{
    int id;
    View_Mode mode;
    
    GUI_Scroll_Vars scroll;
    i32_Rect scroll_region;
    
    int buffer_id;
};
inline View_Vars
view_vars_zero(){
    View_Vars vars = {0};
    return(vars);
}

extern "C" void
view_routine(Application_Links *app, int view_id){
    Custom_Vars *vars = (Custom_Vars*)app->memory;
    View_Vars view = {0};
    view.id = view_id;
    
    int show_scrollbar = 1;
    
    if (!vars->initialized){
        vars->initialized = 1;
        vars->part = make_part(app->memory, app->memory_size);
        push_struct(&vars->part, Custom_Vars);
    }
    
    for(;;){
        Event_Message message = {0};
        message = app->get_event_message(app);
        
        switch (message.type){
            case EM_Open_View:
            {
                view = view_vars_zero();
                view.id = view_id;
            }break;
            
            case EM_Frame:
            {
                GUI_Functions *guifn = app->get_gui_functions(app);
                GUI *gui = app->get_gui(app, view_id);
                
                guifn->begin(gui);
                guifn->top_bar(gui);
                
                switch (view.mode){
                    case ViewMode_File:
                    // TODO(allen): Overlapped widget
                    GUI_id scroll_id;
                    scroll_id.id[1] = view.mode;
                    scroll_id.id[0] = view.buffer_id;
                    
                    guifn->get_scroll_vars(gui, scroll_id, &view.scroll,
                                           &view.scroll_region);
                    guifn->begin_scrollable(gui, scroll_id, view.scroll,
                                            144.f, show_scrollbar);
                    guifn->file(gui, view.buffer_id);
                    guifn->end_scrollable(gui);
                    break;
                }
                
                guifn->end(gui);
                
                // TODO(allen): Put this code in charge of dispatching
                // to the command or command coroutine or whatever.
                
                // TODO(allen): Put this code in charge of when to process
                // the GUI with input and retrieve new layout data.
            }break;
            
            case EM_Close_View:
            {}break;
        }
    }
}

