/* NOTE(casey): This code is _extremely_ bad and is mostly just me hacking things
   around to put in features I want in advance of 4coder having them properly.
   Most of the time I haven't even taken enough time to read the 4coder API
   to know what I'm actually even doing.  So if you decide to use the code in
   here, be advised that it might be super crashy or break something or cause you 
   to lose work or who knows what else!

   DON'T SAY I WE DIDN'T WARN YA: This custom extension provided "as is" without
   warranty of any kind, either express or implied, including without
   limitation any implied warranties of condition, uninterrupted use,
   merchantability, fitness for a particular purpose, or non-infringement.
*/

/* BUGS

Assertion: w:\apps\4coder\source\build\4ed_app.dll
           w:\apps\4coder\source\code\4ed_file_view.cpp
           Line: 3261
           Expression: view->prev_cursor_pos == view_get_cursor_pos(view)


Crash in buffer_get_line_index_range - it doesn't check for lines == 0, which can be the case?

*/

/* TODO(casey): Here are our current issues

   - High priority:
     - Buffer switching still seems a little bit broken.  I find I can't reliably hit switch-return
       and switch to the most recently viewed file that wasn't one of the two currently viewed buffers?
     - High-DPI settings break rendering and all fonts just show up as solid squares
     - Pretty sure auto-indent has some bugs.  Things that should be pretty easy to indent
       properly even from only a few surrounding lines seem to be indented improperly at the moment
     - Multi-line comments should default to indenting to the indentation of the line prior?
     - Would like the option to indent to hanging parentheses, equals signs, etc. instead of
       always just "one tab in from the previous line".
       - Actually, maybe just expose the dirty state, so that the user can decide whether to
         save or not?  Not sure...
     - Replace:
       - Needs to be case-insensitive, or at least have the option to be
       - Needs to replace using the case of the thing being replaced, or at least have the option to do so
     - Auto-complete doesn't pick nearby words first, it seems, which makes it much slower to use?
     - Bug with not being able to switch-to-corresponding-file in another buffer
       without accidentally bringing up the file open dialog?
     - Up/down arrows and mouse clicks on wrapped lines don't seem to work properly with several wraps.
       (eg., a line wrapped to more than 2 physical lines on the screen often doesn't work anymore,
        with up or down jumping to totally wrong places, and mouse clicking jumping to wrong places
        as well - similarly, scrolling breaks, in that it thinks it has "hit the end" of the buffer
        when you cursor down, but the cursor and the rest of the wrapped lines are actually off
        the bottom of the screen)

   - Search:
     - Should highlight all matches in the buffer
     - Seems to buggily break out of the search sometimes for no reason?  (eg., you hit the end and it just drops out of the search instead of stopping?)
       - Tracked this one down: I think it is because spurious mousewheel or other inputs break
         out of the search.  How can this be prevented?

   - Display:
     - When switching _back_ to a buffer, it seems like it loses the scroll position, instead preferring
       to center the cursor?  This is undesirable IMO...
     - I'd like to be able to hide the mark in text entry mode, and show the whole highlighted
       region in edit mode - perhaps even with a magic split at the top or bottom that shows where the mark
       is if it's off screen?
     - There are often repaint bugs with 4coder coming to the front / unminimizing, etc.
       I think this might have something to do with the way you're doing lots of semaphore
       locking but I haven't investigated yet.
     - Need a word-wrap mode that wraps at word boundaries instead of characters
     - Need to be able to set a word wrap length at something other than the window
?FIXED First go-to-line for a file seems to still just go to the beginning of the buffer?
       Not sure Allen's right about the slash problem, but either way, we need some
       way to fix it.
     - NOTE / IMPORTANT / TODO highlighting?  Ability to customize?  Whatever.
     - Some kind of parentheses highlighting?  I can write this myself, but I
       would need some way of adding highlight information to the buffer.
     - Need a way of highlighting the current line like Emacs does for the benefit
       of people on The Stream(TM)
     - Some kind of matching brace display so in long ifs, etc., you can see
       what they match (maybe draw it directly into the buffer?)

   - Indentation:
     - Multiple // lines don't seem to indent properly.  The first one will go to the correct place, but the subsequent ones will go to the first column regardless?
     - Need to have better indentation / wrapping control for typing in comments. 
       Right now it's a bit worse than Emacs, which does automatically put you at
       the same margin as the prev. line (4coder just goes back to column 1).  It'd
       be nice if it go _better_ than Emacs, with no need to manually flow comments,
       etc.
     - It should never reindent text in comments that it doesn't know how to indent - eg., in a comment block, it shouldn't decide to move things around if it doesn't know what they are
     - Sometimes when I hit [ it inserts a [ _and_ a space?  I think this is related to the auto-indent?

   - Buffer management: 
     - I'd like to be able to set a buffer to "auto-revert", so it reloads automatically whenever it changes externally
     - If you undo back to where there are no changes, the "buffer changed" flag should be cleared
     - Seems like there's no way to switch to buffers whose names are substrings of other
       buffers' names without using the mouse?
       - Also, mouse-clicking on buffers doesn't seem to work reliably?  Often it just goes to a 
         blank window?
     - Buffer switch display should always show the buffer _it will switch to when you hit return_
       as the first buffer in the list.

   - File system
     - When switching to a buffer that has changed on disk, notify?  Really this can just
       be some way to query the modification flag and then the customization layer can do it?
     - Still can't seem to open a zero-length file?
     - I'd prefer it if file-open could create new files, and that I could get called on that
       so I can insert my boilerplate headers on new files
     - I'd prefer it if file-open deleted per-character instead of deleting entire path sections

   - Need auto-complete for things like "arbitrary command", with options listed, etc.,
     so this should either be built into 4ed, or the custom DLL should have the ability
     to display possible completions and iterate over internal cmdid's, etc.  Possibly
     the latter, for maximal ability of customizers to add their own commands?

   - Macro recording/playback

   - Arbitrary cool features:
     - Once you can highlight things in 4coder buffers, I could make it so that my 
       metacompiler output _ranges_ for errors, so it highlights the whole token rather
       than putting the cursor in a spot.
     - Highlight on the screen what the completion would be if you hit TAB now (eg., if the string appears elsewhere on the screen)
     - LOC count for the buffer and for all buffers summed shown in the title bar?
     - Show auto-parsed #if/if/for/while/etc. statements at else and closing places.
     - Automatic highlighting of the region in side the parentheses / etc.
     - You should just implement a shell inside 4coder which can call all the 4coder
       stuff as well as execute system stuff, so that from now on you just write
       scripts "in 4coder", etc., so they are always portable everywhere 4coder runs?

   - Things I should write:
     - Ability to do "file open from same directory as the current buffer"
     - Spell-checker
     - To-do list dependent on project?
     - Repeat last replace?
     - Maybe search could be a permanent thing, so instead of initiating a search,
       you're just _changing_ the search term with MODAL-S, and then there's _always_
       a next-of-these-in... and that could go through buffers in order, to...
*/

// NOTE(casey): Microsoft/Windows is poopsauce.

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "4coder_default_include.cpp"

#ifndef Assert
#define internal static
#define Assert assert 
#endif

struct Parsed_Error
{
    int exists;

    String target_file_name;
    int target_line_number;
    int target_column_number;

    int source_buffer_id;
    int source_position;
};

static bool GlobalEditMode;
static char *GlobalCompilationBufferName = "*compilation*";

// TODO(casey): If 4coder gets variables at some point, this would go in a variable.
static char BuildDirectory[4096] = "./";

enum token_type
{
    Token_Unknown,

    Token_OpenParen,    
    Token_CloseParen,    
    Token_Asterisk,
    Token_Minus,
    Token_Plus,
    Token_ForwardSlash,
    Token_Percent,
    Token_Colon,
    Token_Number,
    Token_Comma,

    Token_EndOfStream,
};
struct token
{
    token_type Type;

    size_t TextLength;
    char *Text;
};

struct tokenizer
{
    char *At;
};

inline bool
IsEndOfLine(char C)
{
    bool Result = ((C == '\n') ||
                   (C == '\r'));

    return(Result);
}

inline bool
IsWhitespace(char C)
{
    bool Result = ((C == ' ') ||
                   (C == '\t') ||
                   (C == '\v') ||
                   (C == '\f') ||
                   IsEndOfLine(C));

    return(Result);
}

inline bool
IsAlpha(char C)
{
    bool Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')));

    return(Result);
}

inline bool
IsNumeric(char C)
{
    bool Result = ((C >= '0') && (C <= '9'));

    return(Result);
}

static void
EatAllWhitespace(tokenizer *Tokenizer)
{
    for(;;)
    {
        if(IsWhitespace(Tokenizer->At[0]))
        {
            ++Tokenizer->At;
        }
        else if((Tokenizer->At[0] == '/') &&
                (Tokenizer->At[1] == '/'))
        {
            Tokenizer->At += 2;
            while(Tokenizer->At[0] && !IsEndOfLine(Tokenizer->At[0]))
            {
                ++Tokenizer->At;
            }
        }
        else if((Tokenizer->At[0] == '/') &&
                (Tokenizer->At[1] == '*'))
        {
            Tokenizer->At += 2;
            while(Tokenizer->At[0] &&
                  !((Tokenizer->At[0] == '*') &&
                    (Tokenizer->At[1] == '/')))
            {
                ++Tokenizer->At;
            }

            if(Tokenizer->At[0] == '*')
            {
                Tokenizer->At += 2;
            }
        }
        else
        {
            break;
        }
    }
}

static token
GetToken(tokenizer *Tokenizer)
{
    EatAllWhitespace(Tokenizer);

    token Token = {};
    Token.TextLength = 1;
    Token.Text = Tokenizer->At;
    char C = Tokenizer->At[0];
    ++Tokenizer->At;
    switch(C)
    {
        case 0: {--Tokenizer->At; Token.Type = Token_EndOfStream;} break;

        case '(': {Token.Type = Token_OpenParen;} break;
        case ')': {Token.Type = Token_CloseParen;} break;
        case '*': {Token.Type = Token_Asterisk;} break;
        case '-': {Token.Type = Token_Minus;} break;
        case '+': {Token.Type = Token_Plus;} break;
        case '/': {Token.Type = Token_ForwardSlash;} break;
        case '%': {Token.Type = Token_Percent;} break;
        case ':': {Token.Type = Token_Colon;} break;
        case ',': {Token.Type = Token_Comma;} break;

        default:
        {
            if(IsNumeric(C))
            {
                // TODO(casey): Real number
                Token.Type = Token_Number;
                while(IsNumeric(Tokenizer->At[0]) ||
                      (Tokenizer->At[0] == '.') ||
                      (Tokenizer->At[0] == 'f'))
                {
                    ++Tokenizer->At;
                    Token.TextLength = Tokenizer->At - Token.Text;
                }
            }
            else
            {
                Token.Type = Token_Unknown;
            }
        } break;        
    }

    return(Token);
}

static token
PeekToken(tokenizer *Tokenizer)
{
    tokenizer Tokenizer2 = *Tokenizer;
    token Result = GetToken(&Tokenizer2);
    return(Result);
}

inline bool
IsH(String extension)
{
    bool Result = (match(extension, make_lit_string("h")) ||
                   match(extension, make_lit_string("hpp")) ||
                   match(extension, make_lit_string("hin")));

    return(Result);
}

inline bool
IsCPP(String extension)
{
    bool Result = (match(extension, make_lit_string("c")) ||
                   match(extension, make_lit_string("cpp")) ||
                   match(extension, make_lit_string("cin")));

    return(Result);
}

inline bool
IsINL(String extension)
{
    bool Result = (match(extension, make_lit_string("inl")) != 0);

    return(Result);
}

inline bool
IsCode(String extension)
{
    bool Result = (IsH(extension) || IsCPP(extension) || IsINL(extension));

    return(Result);
}


CUSTOM_COMMAND_SIG(casey_open_in_other)
{
    exec_command(app, cmdid_change_active_panel);
    exec_command(app, cmdid_interactive_open);
}

CUSTOM_COMMAND_SIG(casey_clean_and_save)
{
    exec_command(app, cmdid_clean_all_lines);
    exec_command(app, cmdid_eol_nixify);
    exec_command(app, cmdid_save);
}

CUSTOM_COMMAND_SIG(casey_newline_and_indent)
{
    exec_command(app, write_character);
    exec_command(app, auto_tab_line_at_cursor);
}

CUSTOM_COMMAND_SIG(casey_open_file_other_window)
{
    exec_command(app, cmdid_change_active_panel);
    exec_command(app, cmdid_interactive_open);
}

CUSTOM_COMMAND_SIG(casey_switch_buffer_other_window)
{
    exec_command(app, cmdid_change_active_panel);
    exec_command(app, cmdid_interactive_switch_buffer);
}

internal void
DeleteAfterCommand(struct Application_Links *app, unsigned long long CommandID)
{
    unsigned int access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);

    int pos2 = view.cursor.pos;
    if (CommandID < cmdid_count){
        exec_command(app, (Command_ID)CommandID);
    }
    else{
        exec_command(app, (Custom_Command_Function*)CommandID);
    }
    refresh_view(app, &view);
    int pos1 = view.cursor.pos;

    Range range = make_range(pos1, pos2);

    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    app->buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
}

CUSTOM_COMMAND_SIG(casey_delete_token_left)
{
    DeleteAfterCommand(app, (unsigned long long)seek_white_or_token_left);
}

CUSTOM_COMMAND_SIG(casey_delete_token_right)
{
    DeleteAfterCommand(app, (unsigned long long)seek_white_or_token_right);
}

CUSTOM_COMMAND_SIG(casey_kill_to_end_of_line)
{
    unsigned int access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);

    int pos2 = view.cursor.pos;
    exec_command(app, seek_end_of_line);
    refresh_view(app, &view);
    int pos1 = view.cursor.pos;

    Range range = make_range(pos1, pos2);
    if(pos1 == pos2)
    {
        range.max += 1;
    }

    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    app->buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    exec_command(app, auto_tab_line_at_cursor);
}

CUSTOM_COMMAND_SIG(casey_paste_and_tab)
{
    exec_command(app, paste);
    exec_command(app, auto_tab_range);
}

CUSTOM_COMMAND_SIG(casey_seek_beginning_of_line_and_tab)
{
    exec_command(app, seek_beginning_of_line);
    exec_command(app, auto_tab_line_at_cursor);
}

CUSTOM_COMMAND_SIG(casey_seek_beginning_of_line)
{
    exec_command(app, auto_tab_line_at_cursor);
    exec_command(app, seek_beginning_of_line);
}

struct switch_to_result
{
    bool Switched;
    bool Loaded;
    View_Summary view;
    Buffer_Summary buffer;
};

inline void
SanitizeSlashes(String Value)
{
    for(int At = 0;
        At < Value.size;
        ++At)
    {
        if(Value.str[At] == '\\')
        {
            Value.str[At] = '/';
        }
    }
}

inline switch_to_result
SwitchToOrLoadFile(struct Application_Links *app, String FileName, bool CreateIfNotFound = false)
{
    switch_to_result Result = {};

    SanitizeSlashes(FileName);
    
    unsigned int access = AccessAll;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer_by_name(app, FileName.str, FileName.size, access);

    Result.view = view;
    Result.buffer = buffer;

    if(buffer.exists)
    {
        app->view_set_buffer(app, &view, buffer.buffer_id, 0);
        Result.Switched = true;
    }
    else
    {
        if(app->file_exists(app, FileName.str, FileName.size) || CreateIfNotFound)
        {
            // NOTE(allen): This opens the file and puts it in &view
            // This returns false if the open fails.
            view_open_file(app, &view, expand_str(FileName), false);
            
            Result.buffer = app->get_buffer_by_name(app, FileName.str, FileName.size, access);            
            
            Result.Loaded = true;
            Result.Switched = true;
        }
    }

    return(Result);
}

CUSTOM_COMMAND_SIG(casey_load_todo)
{
    String ToDoFileName = make_lit_string("w:/handmade/code/todo.txt");
    SwitchToOrLoadFile(app, ToDoFileName, true);
}

CUSTOM_COMMAND_SIG(casey_build_search)
{
    int keep_going = 1;
    int old_size;
    // TODO(allen): It's fine to get memory this way for now, eventually
    // we should properly suballocating from app->memory.
    String dir = make_string(app->memory, 0, app->memory_size);
    dir.size = app->directory_get_hot(app, dir.str, dir.memory_size);

    while (keep_going)
    {
        old_size = dir.size;
        append(&dir, "build.bat");

        if (app->file_exists(app, dir.str, dir.size))
        {
            dir.size = old_size;
            memcpy(BuildDirectory, dir.str, dir.size);
            BuildDirectory[dir.size] = 0;

            return;
        }

        dir.size = old_size;

        if (app->directory_cd(app, dir.str, &dir.size, dir.memory_size, literal("..")) == 0)
        {
            keep_going = 0;
        }
    }

    // TODO(casey): How do I print out that it found or didn't find something?
}

CUSTOM_COMMAND_SIG(casey_find_corresponding_file)
{
    unsigned int access = AccessProtected;
    View_Summary view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);

    String extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
    if (extension.str)
    {       
        char *HExtensions[] =
        {
            "hpp",
            "hin",
            "h",
        };

        char *CExtensions[] =
        {
            "c",
            "cin",
            "cpp",
        };

        int ExtensionCount = 0;
        char **Extensions = 0;
        if(IsH(extension))
        {
            ExtensionCount = ArrayCount(CExtensions);
            Extensions = CExtensions;
        }
        else if(IsCPP(extension) || IsINL(extension))
        {
            ExtensionCount = ArrayCount(HExtensions);
            Extensions = HExtensions;
        }

        int MaxExtensionLength = 3;
        int Space = (int)(buffer.file_name_len + MaxExtensionLength);
        String FileNameStem = make_string(buffer.file_name, (int)(extension.str - buffer.file_name), 0);
        String TestFileName = make_string(app->memory, 0, Space);   
        for(int ExtensionIndex = 0;
            ExtensionCount;
            ++ExtensionIndex)
        {
            TestFileName.size = 0;
            append(&TestFileName, FileNameStem);
            append(&TestFileName, Extensions[ExtensionIndex]);

            if(SwitchToOrLoadFile(app, TestFileName, ((ExtensionIndex + 1) == ExtensionCount)).Switched)
            {
                break;
            }
        }
    }
}

CUSTOM_COMMAND_SIG(casey_find_corresponding_file_other_window)
{
    unsigned int access = AccessProtected;
    View_Summary old_view = app->get_active_view(app, access);
    Buffer_Summary buffer = app->get_buffer(app, old_view.buffer_id, access);

    exec_command(app, cmdid_change_active_panel);
    View_Summary new_view = app->get_active_view(app, AccessAll);
    app->view_set_buffer(app, &new_view, buffer.buffer_id, 0);

//    exec_command(app, casey_find_corresponding_file);
}

CUSTOM_COMMAND_SIG(casey_save_and_make_without_asking)
{
    exec_command(app, cmdid_change_active_panel);
    
    Buffer_Summary buffer = {};
    
    unsigned int access = AccessAll;
    for(buffer = app->get_buffer_first(app, access);
        buffer.exists;
        app->get_buffer_next(app, &buffer, access))
    {
        app->save_buffer(app, &buffer, buffer.file_name, buffer.file_name_len, 0);
    }
    
    // NOTE(allen): The parameter pushing made it a little easier
    // to deal with this particular pattern where two similar strings
    // were both used.  Now both strings need to exist at the same
    // time on the users side.
    
    int size = app->memory_size/2;
    String dir = make_string(app->memory, 0, size);
    String command = make_string((char*)app->memory + size, 0, size);
    
    append(&dir, BuildDirectory);
    for(int At = 0;
        At < dir.size;
        ++At)
    {
        if(dir.str[At] == '/')
        {
            dir.str[At] = '\\';
        }
    }
    
    append(&command, dir);
    
    if(append(&command, "build.bat"))
    {
        unsigned int access = AccessAll;
        View_Summary view = app->get_active_view(app, access);
        app->exec_system_command(app, &view,
                                 buffer_identifier(GlobalCompilationBufferName, (int)strlen(GlobalCompilationBufferName)),
                                 dir.str, dir.size,
                                 command.str, command.size,
                                 CLI_OverlapWithConflict);
    }
    exec_command(app, cmdid_change_active_panel);
}

internal bool
casey_errors_are_the_same(Parsed_Error a, Parsed_Error b)
{
    bool result = ((a.exists == b.exists) && compare(a.target_file_name, b.target_file_name) && (a.target_line_number == b.target_line_number));

    return(result);
}

internal void
casey_goto_error(Application_Links *app, Parsed_Error e)
{
    if(e.exists)
    {
        switch_to_result Switch = SwitchToOrLoadFile(app, e.target_file_name, false);
        if(Switch.Switched)
        {
            app->view_set_cursor(app, &Switch.view, seek_line_char(e.target_line_number, e.target_column_number), 1);
        }

        View_Summary compilation_view = get_first_view_with_buffer(app, e.source_buffer_id);
        if(compilation_view.exists)
        {
            app->view_set_cursor(app, &compilation_view, seek_pos(e.source_position), 1);
        }
    }
}

internal Parsed_Error
casey_parse_error(Application_Links *app, Buffer_Summary buffer, View_Summary view)
{
    Parsed_Error result = {};

    refresh_view(app, &view);
    int restore_pos = view.cursor.pos;

    // TODO(allen): view_compute_cursor can get these
    // positions without ever changing the position of the cursor.
    app->view_set_cursor(app, &view, seek_line_char(view.cursor.line, 1), 1);
    int start = view.cursor.pos;

    app->view_set_cursor(app, &view, seek_line_char(view.cursor.line, 65536), 1);
    int end = view.cursor.pos;

    app->view_set_cursor(app, &view, seek_pos(restore_pos), 1);

    int size = end - start;

    char *ParsingRegion = (char *)malloc(size + 1);
//    char *ParsingRegion = (char *)app->push_memory(app, size + 1);
    app->buffer_read_range(app, &buffer, start, end, ParsingRegion);
    ParsingRegion[size] = 0;
    tokenizer Tokenizer = {ParsingRegion};
    for(;;)
    {
        token Token = GetToken(&Tokenizer);
        if(Token.Type == Token_OpenParen)
        {
            token LineToken = GetToken(&Tokenizer);
            if(LineToken.Type == Token_Number)
            {
                token CloseToken = GetToken(&Tokenizer);

                int column_number = 0;
                if(CloseToken.Type == Token_Comma)
                {
                    token ColumnToken = GetToken(&Tokenizer);
                    if(ColumnToken.Type == Token_Number)
                    {
                        column_number = atoi(ColumnToken.Text);
                        CloseToken = GetToken(&Tokenizer);
                    }
                }

                if(CloseToken.Type == Token_CloseParen)
                {
                    token ColonToken = GetToken(&Tokenizer);
                    if(ColonToken.Type == Token_Colon)
                    {
                        // NOTE(casey): We maybe found an error!
                        int line_number = atoi(LineToken.Text);

                        char *Seek = Token.Text;
                        while(Seek != ParsingRegion)
                        {
                            if(IsEndOfLine(*Seek))
                            {
                                while(IsWhitespace(*Seek))
                                {
                                    ++Seek;
                                }
                                break;
                            }

                            --Seek;
                        }

                        result.exists = true;
                        result.target_file_name = make_string(Seek, (int)(Token.Text - Seek));;
                        result.target_line_number = line_number;
                        result.target_column_number = column_number;
                        result.source_buffer_id = buffer.buffer_id;
                        result.source_position = start + (int)(ColonToken.Text - ParsingRegion);

                        break;
                    }
                }
            }
        }
        else if(Token.Type == Token_EndOfStream)
        {
            break;
        }
    }

    return(result);
}

internal void
casey_seek_error_dy(Application_Links *app, int dy)
{
    Buffer_Summary Buffer = app->get_buffer_by_name(app, GlobalCompilationBufferName, (int)strlen(GlobalCompilationBufferName), AccessAll);
    View_Summary compilation_view = get_first_view_with_buffer(app, Buffer.buffer_id);

    // NOTE(casey): First get the current error (which may be none, if we've never parsed before)
    Parsed_Error StartingError = casey_parse_error(app, Buffer, compilation_view);

    // NOTE(casey): Now hunt for the previous distinct error
    for(;;)
    {
        int prev_pos = compilation_view.cursor.pos;
        app->view_set_cursor(app, &compilation_view, seek_line_char(compilation_view.cursor.line + dy, 0), 1);
        if(compilation_view.cursor.pos != prev_pos)
        {
            Parsed_Error Error = casey_parse_error(app, Buffer, compilation_view);
            if(Error.exists && !casey_errors_are_the_same(StartingError, Error))
            {
                casey_goto_error(app, Error);
                break;
            }
        }
        else
        {
            break;
        }
    }
}

CUSTOM_COMMAND_SIG(casey_goto_previous_error)
{
    casey_seek_error_dy(app, -1);
}

CUSTOM_COMMAND_SIG(casey_goto_next_error)
{
    casey_seek_error_dy(app, 1);
}

CUSTOM_COMMAND_SIG(casey_imenu)
{
    // TODO(casey): Implement!
}

//
// TODO(casey): Everything below this line probably isn't possible yet
//

CUSTOM_COMMAND_SIG(casey_call_keyboard_macro)
{
    // TODO(casey): Implement!
}

CUSTOM_COMMAND_SIG(casey_begin_keyboard_macro_recording)
{
    // TODO(casey): Implement!
}

CUSTOM_COMMAND_SIG(casey_end_keyboard_macro_recording)
{
    // TODO(casey): Implement!
}

CUSTOM_COMMAND_SIG(casey_fill_paragraph)
{
    // TODO(casey): Implement!
}

enum calc_node_type
{
    CalcNode_UnaryMinus,

    CalcNode_Add,
    CalcNode_Subtract,
    CalcNode_Multiply,
    CalcNode_Divide,
    CalcNode_Mod,

    CalcNode_Constant,
};
struct calc_node
{
    calc_node_type Type;
    double Value;
    calc_node *Left;
    calc_node *Right;
};

internal double
ExecCalcNode(calc_node *Node)
{
    double Result = 0.0f;

    if(Node)
    {
        switch(Node->Type)
        {
            case CalcNode_UnaryMinus: {Result = -ExecCalcNode(Node->Left);} break;
            case CalcNode_Add: {Result = ExecCalcNode(Node->Left) + ExecCalcNode(Node->Right);} break;
            case CalcNode_Subtract: {Result = ExecCalcNode(Node->Left) - ExecCalcNode(Node->Right);} break;
            case CalcNode_Multiply: {Result = ExecCalcNode(Node->Left) * ExecCalcNode(Node->Right);} break;
            case CalcNode_Divide: {/*TODO(casey): Guard 0*/Result = ExecCalcNode(Node->Left) / ExecCalcNode(Node->Right);} break;
            case CalcNode_Mod: {/*TODO(casey): Guard 0*/Result = fmod(ExecCalcNode(Node->Left), ExecCalcNode(Node->Right));} break;
            case CalcNode_Constant: {Result = Node->Value;} break;
            default: {Assert(!"AHHHHH!");}
        }
    }

    return(Result);
}

internal void
FreeCalcNode(calc_node *Node)
{
    if(Node)
    {
        FreeCalcNode(Node->Left);
        FreeCalcNode(Node->Right);
        free(Node);
    }
}

internal calc_node *
AddNode(calc_node_type Type, calc_node *Left = 0, calc_node *Right = 0)
{
    calc_node *Node = (calc_node *)malloc(sizeof(calc_node));
    Node->Type = Type;
    Node->Value = 0;
    Node->Left = Left;
    Node->Right = Right;    
    return(Node);
}

internal calc_node *
ParseNumber(tokenizer *Tokenizer)
{
    calc_node *Result = AddNode(CalcNode_Constant);

    token Token = GetToken(Tokenizer);
    Result->Value = atof(Token.Text);

    return(Result);
}

internal calc_node *
ParseConstant(tokenizer *Tokenizer)
{
    calc_node *Result = 0;

    token Token = PeekToken(Tokenizer);
    if(Token.Type == Token_Minus)
    {
        Token = GetToken(Tokenizer);
        Result = AddNode(CalcNode_UnaryMinus);
        Result->Left = ParseNumber(Tokenizer);
    }
    else
    {
        Result = ParseNumber(Tokenizer);
    }

    return(Result);
}

internal calc_node *
ParseMultiplyExpression(tokenizer *Tokenizer)
{
    calc_node *Result = 0;

    token Token = PeekToken(Tokenizer);
    if((Token.Type == Token_Minus) ||
       (Token.Type == Token_Number))
    {
        Result = ParseConstant(Tokenizer);
        token Token = PeekToken(Tokenizer);
        if(Token.Type == Token_ForwardSlash)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Divide, Result, ParseNumber(Tokenizer));
        }
        else if(Token.Type == Token_Asterisk)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Multiply, Result, ParseNumber(Tokenizer));
        }
    }

    return(Result);
}

internal calc_node *
ParseAddExpression(tokenizer *Tokenizer)
{
    calc_node *Result = 0;

    token Token = PeekToken(Tokenizer);
    if((Token.Type == Token_Minus) ||
       (Token.Type == Token_Number))
    {
        Result = ParseMultiplyExpression(Tokenizer);
        token Token = PeekToken(Tokenizer);
        if(Token.Type == Token_Plus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Add, Result, ParseMultiplyExpression(Tokenizer));
        }
        else if(Token.Type == Token_Minus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Subtract, Result, ParseMultiplyExpression(Tokenizer));
        }
    }

    return(Result);
}

internal calc_node *
ParseCalc(tokenizer *Tokenizer)
{
    calc_node *Node = ParseAddExpression(Tokenizer);

    return(Node);
}

CUSTOM_COMMAND_SIG(casey_quick_calc)
{
    unsigned int access = AccessOpen;
    View_Summary view = app->get_active_view(app, access);

    Range range = get_range(&view);

    size_t Size = range.max - range.min;
    char *Stuff = (char *)malloc(Size + 1);
    Stuff[Size] = 0;

    Buffer_Summary buffer = app->get_buffer(app, view.buffer_id, access);
    app->buffer_read_range(app, &buffer, range.min, range.max, Stuff);

    tokenizer Tokenizer = {Stuff};
    calc_node *CalcTree = ParseCalc(&Tokenizer);
    double ComputedValue = ExecCalcNode(CalcTree);
    FreeCalcNode(CalcTree);

    char ResultBuffer[256];
    int ResultSize = sprintf(ResultBuffer, "%f", ComputedValue);

    app->buffer_replace_range(app, &buffer, range.min, range.max, ResultBuffer, ResultSize);

    free(Stuff);
}

internal void
OpenProject(Application_Links *app, char *ProjectFileName)
{
    int TotalOpenAttempts = 0;

    FILE *ProjectFile = fopen(ProjectFileName, "r");
    if(ProjectFile)
    {
        fgets(BuildDirectory, sizeof(BuildDirectory) - 1, ProjectFile);
        size_t BuildDirSize = strlen(BuildDirectory);
        if((BuildDirSize) && (BuildDirectory[BuildDirSize - 1] == '\n'))
        {
            --BuildDirSize;
        }

        if((BuildDirSize) && (BuildDirectory[BuildDirSize - 1] != '/'))
        {
            BuildDirectory[BuildDirSize++] = '/';
            BuildDirectory[BuildDirSize] = 0;
        }

        char SourceFileDirectoryName[4096];
        char FileDirectoryName[4096];
        while(fgets(SourceFileDirectoryName, sizeof(SourceFileDirectoryName) - 1, ProjectFile))
        {
            // NOTE(allen|a3.4.4): Here we get the list of files in this directory.
            // Notice that we free_file_list at the end.
            String dir = make_string(FileDirectoryName, 0, sizeof(FileDirectoryName));
            append(&dir, SourceFileDirectoryName);
            if(dir.size && dir.str[dir.size-1] == '\n')
            {
                --dir.size;
            }

            if(dir.size && dir.str[dir.size-1] != '/')
            {
                dir.str[dir.size++] = '/';
            }

            File_List list = app->get_file_list(app, dir.str, dir.size);
            int dir_size = dir.size;

            for (int i = 0; i < list.count; ++i)
            {
                File_Info *info = list.infos + i;
                if (!info->folder)
                {
                    String extension = file_extension(info->filename);
                    if (IsCode(extension))
                    {
                        // NOTE(allen): There's no way in the 4coder API to use relative
                        // paths at the moment, so everything should be full paths.  Which is
                        // managable.  Here simply set the dir string size back to where it
                        // was originally, so that new appends overwrite old ones.
                        dir.size = dir_size;
                        append(&dir, info->filename);
                        
                        view_open_file(app, 0, dir.str, dir.size, true);
                        ++TotalOpenAttempts;
                    }
                }
            }

            app->free_file_list(app, list);
        }

        fclose(ProjectFile);
    }
}    

CUSTOM_COMMAND_SIG(casey_execute_arbitrary_command)
{
    Query_Bar bar;
    char space[1024], more_space[1024];
    bar.prompt = make_lit_string("Command: ");
    bar.string = make_fixed_width_string(space);

    if (!query_user_string(app, &bar)) return;
    app->end_query_bar(app, &bar, 0);

    if(match(bar.string, make_lit_string("project")))
    {
//        exec_command(app, open_all_code);
    }
    else if(match(bar.string, make_lit_string("open menu")))
    {
        exec_command(app, cmdid_open_menu);
    }
    else
    {
        bar.prompt = make_fixed_width_string(more_space);
        append(&bar.prompt, make_lit_string("Unrecognized: "));
        append(&bar.prompt, bar.string);
        bar.string.size = 0;

        app->start_query_bar(app, &bar, 0);
        app->get_user_input(app, EventOnAnyKey | EventOnButton, 0);
    }
}

internal void
UpdateModalIndicator(Application_Links *app)
{
    Theme_Color normal_colors[] = 
    {
        {Stag_Cursor, 0x40FF40},
        {Stag_At_Cursor, 0x161616},
        {Stag_Mark, 0x808080},
        {Stag_Margin, 0x262626},
        {Stag_Margin_Hover, 0x333333},
        {Stag_Margin_Active, 0x404040},
        {Stag_Bar, 0xCACACA}
    };

    Theme_Color edit_colors[] = 
    {
        {Stag_Cursor, 0xFF0000},
        {Stag_At_Cursor, 0x00FFFF},
        {Stag_Mark, 0xFF6F1A},
        {Stag_Margin, 0x33170B},
        {Stag_Margin_Hover, 0x49200F},
        {Stag_Margin_Active, 0x934420},
        {Stag_Bar, 0x934420}
    };

    if (GlobalEditMode){
        app->set_theme_colors(app, edit_colors, ArrayCount(edit_colors));
    }
    else{
        app->set_theme_colors(app, normal_colors, ArrayCount(normal_colors));
    }
}
CUSTOM_COMMAND_SIG(begin_free_typing)
{
    GlobalEditMode = false;
    UpdateModalIndicator(app);
}

CUSTOM_COMMAND_SIG(end_free_typing)
{
    GlobalEditMode = true;
    UpdateModalIndicator(app);
}

#define DEFINE_FULL_BIMODAL_KEY(binding_name,edit_code,normal_code) \
CUSTOM_COMMAND_SIG(binding_name) \
{ \
    if(GlobalEditMode) \
    { \
        edit_code;            \
    } \
    else \
    { \
        normal_code; \
    } \
}

#define DEFINE_BIMODAL_KEY(binding_name,edit_code,normal_code) DEFINE_FULL_BIMODAL_KEY(binding_name,exec_command(app,edit_code),exec_command(app,normal_code))
#define DEFINE_MODAL_KEY(binding_name,edit_code) DEFINE_BIMODAL_KEY(binding_name,edit_code,write_character)

//    cmdid_paste_next ?
//    cmdid_timeline_scrub ?
//    cmdid_history_backward,
//    cmdid_history_forward,
//    cmdid_toggle_line_wrap,
//    cmdid_close_minor_view,

DEFINE_MODAL_KEY(modal_space, set_mark);
DEFINE_MODAL_KEY(modal_back_slash, casey_clean_and_save);
DEFINE_MODAL_KEY(modal_single_quote, casey_call_keyboard_macro);
DEFINE_MODAL_KEY(modal_comma, casey_goto_previous_error);
DEFINE_MODAL_KEY(modal_period, casey_fill_paragraph);
DEFINE_MODAL_KEY(modal_forward_slash, cmdid_change_active_panel);
DEFINE_MODAL_KEY(modal_semicolon, cursor_mark_swap); // TODO(casey): Maybe cmdid_history_backward?
DEFINE_BIMODAL_KEY(modal_open_bracket, casey_begin_keyboard_macro_recording, write_and_auto_tab);
DEFINE_BIMODAL_KEY(modal_close_bracket, casey_end_keyboard_macro_recording, write_and_auto_tab);
DEFINE_MODAL_KEY(modal_a, write_character); // TODO(casey): Arbitrary command + casey_quick_calc
DEFINE_MODAL_KEY(modal_b, cmdid_interactive_switch_buffer);
DEFINE_MODAL_KEY(modal_c, casey_find_corresponding_file);
DEFINE_MODAL_KEY(modal_d, casey_kill_to_end_of_line);
DEFINE_MODAL_KEY(modal_e, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_f, casey_paste_and_tab);
DEFINE_MODAL_KEY(modal_g, goto_line);
DEFINE_MODAL_KEY(modal_h, auto_tab_range);
DEFINE_MODAL_KEY(modal_i, move_up);
DEFINE_MODAL_KEY(modal_j, seek_white_or_token_left);
DEFINE_MODAL_KEY(modal_k, move_down);
DEFINE_MODAL_KEY(modal_l, seek_white_or_token_right);
DEFINE_MODAL_KEY(modal_m, casey_save_and_make_without_asking);
DEFINE_MODAL_KEY(modal_n, casey_goto_next_error);
DEFINE_MODAL_KEY(modal_o, query_replace);
DEFINE_MODAL_KEY(modal_p, replace_in_range);
DEFINE_MODAL_KEY(modal_q, copy);
DEFINE_MODAL_KEY(modal_r, reverse_search); // NOTE(allen): I've modified my default search so you can use it now.
DEFINE_MODAL_KEY(modal_s, search);
DEFINE_MODAL_KEY(modal_t, casey_load_todo);
DEFINE_MODAL_KEY(modal_u, cmdid_undo);
DEFINE_MODAL_KEY(modal_v, casey_switch_buffer_other_window);
DEFINE_MODAL_KEY(modal_w, cut);
DEFINE_MODAL_KEY(modal_x, casey_find_corresponding_file_other_window);
DEFINE_MODAL_KEY(modal_y, cmdid_redo);
DEFINE_MODAL_KEY(modal_z, cmdid_interactive_open);

DEFINE_MODAL_KEY(modal_1, casey_build_search); // TODO(casey): Shouldn't need to bind a key for this?
DEFINE_MODAL_KEY(modal_2, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_3, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_4, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_5, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_6, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_7, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_8, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_9, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_0, cmdid_kill_buffer);
DEFINE_MODAL_KEY(modal_minus, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_equals, casey_execute_arbitrary_command);

DEFINE_BIMODAL_KEY(modal_backspace, casey_delete_token_left, backspace_char);
DEFINE_BIMODAL_KEY(modal_up, move_up, move_up);
DEFINE_BIMODAL_KEY(modal_down, move_down, move_down);
DEFINE_BIMODAL_KEY(modal_left, seek_white_or_token_left, move_left); 
DEFINE_BIMODAL_KEY(modal_right, seek_white_or_token_right, move_right);
DEFINE_BIMODAL_KEY(modal_delete, casey_delete_token_right, delete_char);
DEFINE_BIMODAL_KEY(modal_home, casey_seek_beginning_of_line, casey_seek_beginning_of_line_and_tab);
DEFINE_BIMODAL_KEY(modal_end, seek_end_of_line, seek_end_of_line);
DEFINE_BIMODAL_KEY(modal_page_up, cmdid_page_up, seek_whitespace_up);
DEFINE_BIMODAL_KEY(modal_page_down, cmdid_page_down, seek_whitespace_down);
DEFINE_BIMODAL_KEY(modal_tab, cmdid_word_complete, cmdid_word_complete);

OPEN_FILE_HOOK_SIG(casey_file_settings)
{
    // NOTE(allen|a4): As of alpha 4 hooks can have parameters which are
    // received through functions like this app->get_parameter_buffer.
    // This is different from the past where this hook got a buffer
    // from app->get_active_buffer.
    unsigned int access = AccessAll;
    //Buffer_Summary buffer = app->get_parameter_buffer(app, 0, access);
    Buffer_Summary buffer = app->get_buffer(app, buffer_id, access);

    int treat_as_code = 0;
    int treat_as_project = 0;

    if (buffer.file_name && buffer.size < (16 << 20))
    {
        String ext = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        treat_as_code = IsCode(ext);
        treat_as_project = match(ext, make_lit_string("prj"));
    }
    
    app->buffer_set_setting(app, &buffer, BufferSetting_Lex, treat_as_code);
    app->buffer_set_setting(app, &buffer, BufferSetting_WrapLine, !treat_as_code);
    app->buffer_set_setting(app, &buffer, BufferSetting_MapID, mapid_file);
    
    if(treat_as_project)
    {
        OpenProject(app, buffer.file_name);
        // NOTE(casey): Don't actually want to kill this, or you can never edit the project.
//        exec_command(app, cmdid_kill_buffer);

    }

    return(0);
}

bool
CubicUpdateFixedDuration1(float *P0, float *V0, float P1, float V1, float Duration, float dt)
{
    bool Result = false;

    if(dt > 0)
    {
        if(Duration < dt)
        {
            *P0 = P1 + (dt - Duration)*V1;
            *V0 = V1;
            Result = true;
        }
        else
        {
            float t = (dt / Duration);
            float u = (1.0f - t);

            float C0 = 1*u*u*u;
            float C1 = 3*u*u*t;
            float C2 = 3*u*t*t;
            float C3 = 1*t*t*t;

            float dC0 = -3*u*u;
            float dC1 = -6*u*t + 3*u*u;
            float dC2 =  6*u*t - 3*t*t;
            float dC3 =  3*t*t;

            float B0 = *P0;
            float B1 = *P0 + (Duration / 3.0f) * *V0;
            float B2 = P1 - (Duration / 3.0f) * V1;
            float B3 = P1;

            *P0 = C0*B0 + C1*B1 + C2*B2 + C3*B3;
            *V0 = (dC0*B0 + dC1*B1 + dC2*B2 + dC3*B3) * (1.0f / Duration);
        }
    }

    return(Result);
}

struct Casey_Scroll_Velocity
{
    float x, y, t;
};

Casey_Scroll_Velocity casey_scroll_velocity_[16] = {0};
Casey_Scroll_Velocity *casey_scroll_velocity = casey_scroll_velocity_;

SCROLL_RULE_SIG(casey_smooth_scroll_rule){
	dt = 1.0f/60.0f;
    Casey_Scroll_Velocity *velocity = casey_scroll_velocity + view_id;
    int result = 0;
    if(is_new_target)
    {
        if((*scroll_x != target_x) ||
            (*scroll_y != target_y))
        {
            velocity->t = 0.1f;
        }
    }

    if(velocity->t > 0)
    {
        result = !(CubicUpdateFixedDuration1(scroll_x, &velocity->x, target_x, 0.0f, velocity->t, dt) ||
                CubicUpdateFixedDuration1(scroll_y, &velocity->y, target_y, 0.0f, velocity->t, dt));
    }

    velocity->t -= dt;
    if(velocity->t < 0)
    {
        velocity->t = 0;
        *scroll_x = target_x;
        *scroll_y = target_y;
    }

    return(result);
}

#include <windows.h>
#pragma comment(lib, "user32.lib")
static HWND GlobalWindowHandle;
static WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};
internal BOOL CALLBACK win32_find_4coder_window(HWND Window, LPARAM LParam)
{
    BOOL Result = TRUE;

    char TestClassName[256];
    GetClassName(Window, TestClassName, sizeof(TestClassName));
    if((strcmp("4coder-win32-wndclass", TestClassName) == 0) && 
       ((HINSTANCE)GetWindowLongPtr(Window, GWLP_HINSTANCE) == GetModuleHandle(0)))
    {
        GlobalWindowHandle = Window;
        Result = FALSE;
    }

    return(Result);
}

internal void
win32_toggle_fullscreen(void)
{
#if 0
    // NOTE(casey): This follows Raymond Chen's prescription
    // for fullscreen toggling, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx

    HWND Window = GlobalWindowHandle;
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                            MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                            MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                            MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                        SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
#else
    ShowWindow(GlobalWindowHandle, SW_MAXIMIZE);
#endif
}

HOOK_SIG(casey_start)
{
    exec_command(app, cmdid_hide_scrollbar);
    exec_command(app, cmdid_open_panel_vsplit);
    exec_command(app, cmdid_hide_scrollbar);
    exec_command(app, cmdid_change_active_panel);

    app->change_theme(app, literal("Handmade Hero"));
    app->change_font(app, literal("Liberation Mono"));

    Theme_Color colors[] =
    {
        {Stag_Default, 0xA08563},
        // {Stag_Bar, },
        // {Stag_Bar_Active, },
        // {Stag_Base, },
        // {Stag_Pop1, },
        // {Stag_Pop2, },
        // {Stag_Back, },
        // {Stag_Margin, },
        // {Stag_Margin_Hover, },
        // {Stag_Margin_Active, },
        // {Stag_Cursor, },
        // {Stag_At_Cursor, },
        // {Stag_Highlight, },
        // {Stag_At_Highlight, },
        {Stag_Comment, 0x7D7D7D},
        {Stag_Keyword, 0xCD950C},
        // {Stag_Str_Constant, },
        // {Stag_Char_Constant, },
        // {Stag_Int_Constant, },
        // {Stag_Float_Constant, },
        // {Stag_Bool_Constant, },
        {Stag_Preproc, 0xDAB98F},
        {Stag_Include, 0x6B8E23},
        // {Stag_Special_Character, },
        // {Stag_Highlight_Junk, },
        // {Stag_Highlight_White, },
        // {Stag_Paste, },
        // {Stag_Undo, },
        // {Stag_Next_Undo, },
    };
    app->set_theme_colors(app, colors, ArrayCount(colors));

    win32_toggle_fullscreen();

    return(0);
}

extern "C" GET_BINDING_DATA(get_bindings)
{
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;

    set_hook(context, hook_start, casey_start);
    set_open_file_hook(context, casey_file_settings);
    set_scroll_rule(context, casey_smooth_scroll_rule);

    EnumWindows(win32_find_4coder_window, 0);

    begin_map(context, mapid_global);
    {
        bind(context, 'z', MDFR_NONE, cmdid_interactive_open);
        bind(context, 'x', MDFR_NONE, casey_open_in_other);
        bind(context, 't', MDFR_NONE, casey_load_todo);
        bind(context, '/', MDFR_NONE, cmdid_change_active_panel);
        bind(context, 'b', MDFR_NONE, cmdid_interactive_switch_buffer);
        bind(context, key_page_up, MDFR_NONE, search);
        bind(context, key_page_down, MDFR_NONE, reverse_search);
        bind(context, 'm', MDFR_NONE, casey_save_and_make_without_asking);
        
        // NOTE(allen): Added this so mouse would keep working rougly as before.
        // Of course now there could be a modal click behavior if that will be useful.
        // As well as right click.
        bind(context, key_mouse_left, MDFR_NONE, click_set_cursor);
    }        
    end_map(context);

    begin_map(context, mapid_file);

    bind_vanilla_keys(context, write_character);

    bind(context, key_insert, MDFR_NONE, begin_free_typing);
    bind(context, '`', MDFR_NONE, begin_free_typing);
    bind(context, key_esc, MDFR_NONE, end_free_typing);
    bind(context, '\n', MDFR_NONE, casey_newline_and_indent);
    bind(context, '\n', MDFR_SHIFT, casey_newline_and_indent);

    // NOTE(casey): Modal keys come here.
    bind(context, ' ', MDFR_NONE, modal_space);
    bind(context, ' ', MDFR_SHIFT, modal_space);

    bind(context, '\\', MDFR_NONE, modal_back_slash);
    bind(context, '\'', MDFR_NONE, modal_single_quote);
    bind(context, ',', MDFR_NONE, modal_comma);
    bind(context, '.', MDFR_NONE, modal_period);
    bind(context, '/', MDFR_NONE, modal_forward_slash);
    bind(context, ';', MDFR_NONE, modal_semicolon);
    bind(context, '[', MDFR_NONE, modal_open_bracket);
    bind(context, ']', MDFR_NONE, modal_close_bracket);
    bind(context, '{', MDFR_NONE, write_and_auto_tab);
    bind(context, '}', MDFR_NONE, write_and_auto_tab);
    bind(context, 'a', MDFR_NONE, modal_a);
    bind(context, 'b', MDFR_NONE, modal_b);
    bind(context, 'c', MDFR_NONE, modal_c);
    bind(context, 'd', MDFR_NONE, modal_d);
    bind(context, 'e', MDFR_NONE, modal_e);
    bind(context, 'f', MDFR_NONE, modal_f);
    bind(context, 'g', MDFR_NONE, modal_g);
    bind(context, 'h', MDFR_NONE, modal_h);
    bind(context, 'i', MDFR_NONE, modal_i);
    bind(context, 'j', MDFR_NONE, modal_j);
    bind(context, 'k', MDFR_NONE, modal_k);
    bind(context, 'l', MDFR_NONE, modal_l);
    bind(context, 'm', MDFR_NONE, modal_m);
    bind(context, 'n', MDFR_NONE, modal_n);
    bind(context, 'o', MDFR_NONE, modal_o);
    bind(context, 'p', MDFR_NONE, modal_p);
    bind(context, 'q', MDFR_NONE, modal_q);
    bind(context, 'r', MDFR_NONE, modal_r);
    bind(context, 's', MDFR_NONE, modal_s);
    bind(context, 't', MDFR_NONE, modal_t);
    bind(context, 'u', MDFR_NONE, modal_u);
    bind(context, 'v', MDFR_NONE, modal_v);
    bind(context, 'w', MDFR_NONE, modal_w);
    bind(context, 'x', MDFR_NONE, modal_x);
    bind(context, 'y', MDFR_NONE, modal_y);
    bind(context, 'z', MDFR_NONE, modal_z);

    bind(context, '1', MDFR_NONE, modal_1);
    bind(context, '2', MDFR_NONE, modal_2);
    bind(context, '3', MDFR_NONE, modal_3);
    bind(context, '4', MDFR_NONE, modal_4);
    bind(context, '5', MDFR_NONE, modal_5);
    bind(context, '6', MDFR_NONE, modal_6);
    bind(context, '7', MDFR_NONE, modal_7);
    bind(context, '8', MDFR_NONE, modal_8);
    bind(context, '9', MDFR_NONE, modal_9);
    bind(context, '0', MDFR_NONE, modal_0);
    bind(context, '-', MDFR_NONE, modal_minus);
    bind(context, '=', MDFR_NONE, modal_equals);

    bind(context, key_back, MDFR_NONE, modal_backspace);
    bind(context, key_back, MDFR_SHIFT, modal_backspace);

    bind(context, key_up, MDFR_NONE, modal_up);
    bind(context, key_up, MDFR_SHIFT, modal_up);

    bind(context, key_down, MDFR_NONE, modal_down);
    bind(context, key_down, MDFR_SHIFT, modal_down);

    bind(context, key_left, MDFR_NONE, modal_left);
    bind(context, key_left, MDFR_SHIFT, modal_left);

    bind(context, key_right, MDFR_NONE, modal_right);
    bind(context, key_right, MDFR_SHIFT, modal_right);

    bind(context, key_del, MDFR_NONE, modal_delete);
    bind(context, key_del, MDFR_SHIFT, modal_delete);

    bind(context, key_home, MDFR_NONE, modal_home);
    bind(context, key_home, MDFR_SHIFT, modal_home);

    bind(context, key_end, MDFR_NONE, modal_end);
    bind(context, key_end, MDFR_SHIFT, modal_end);

    bind(context, key_page_up, MDFR_NONE, modal_page_up);
    bind(context, key_page_up, MDFR_SHIFT, modal_page_up);

    bind(context, key_page_down, MDFR_NONE, modal_page_down);
    bind(context, key_page_down, MDFR_SHIFT, modal_page_down);

    bind(context, '\t', MDFR_NONE, modal_tab);
    bind(context, '\t', MDFR_SHIFT, modal_tab);

    end_map(context);

    end_bind_helper(context);
    return context->write_total;
}
