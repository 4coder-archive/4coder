/*
4coder_examples.cpp - Commands that are included mainly to serve as example code for
customization writers.
*/

// TOP

// tags: history; group
// example-of: History_Group; history_group_begin; history_group_end
CUSTOM_COMMAND_SIG(double_backspace)
CUSTOM_DOC("Example of history group helpers")
{
 /* History_Group is a wrapper around the history API that makes it easy to
group any series of edits into a single undo/redo record in the buffer's history.
Before any edits call history_group_begin and afterwards call history_group_end.
After history_group_end all of the edits to the buffer supplied in history_group_begin
will be merged, including all edits from function and command calls. */
 
 View_ID view = get_active_view(app, Access_ReadWriteVisible);
 Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
 History_Group group = history_group_begin(app, buffer);
 backspace_char(app);
 backspace_char(app);
 history_group_end(group);
}

// tags: query; bar
// example-of: Query_Bar
CUSTOM_COMMAND_SIG(play_with_a_counter)
CUSTOM_DOC("Example of query bar")
{
 /* Query bars make a quick lightweight display of a single line of text for interactive
commands, while still showing the buffer. Query bars are convenient because they don't
require any complex UI setup, or extra rendering work inside your command.

First, constructing a Query_Bar_Group is a convenient way to make sure the query bar
states are fully cleaned up when the command ends.

Second, we make our query bar and start showing it with start_query_bar. Until we
call end_query_bar on the same bar, or the group's destructor runs, the bar struct
needs to remain in memory. The easy way to accomplish this is to just let the bar be
on the commands stack frame. */
 
 i32 counter = 0;
 
 Query_Bar_Group group(app);
 Query_Bar dumb_bar = {};
 dumb_bar.prompt = SCu8("Goes away at >= 10");
 if (!start_query_bar(app, &dumb_bar, 0)){
  return;
 }
 
 Query_Bar bar = {};
 bar.prompt = SCu8("Counter = ");
 bar.string = SCu8("");
 if (!start_query_bar(app, &bar, 0)){
  return;
 }
 
 for (;;){
  /* Notice here, we set the string of the query bar BEFORE we call get_next_input.
  get_next_input blocks this command until the next input is sent from the core. Whatever
string we put in the bar now will be shown and remain in the bar until an event wakes
up this command and we get a chance to modify the bar again. */
  Scratch_Block scratch(app);
  bar.string = push_stringf(scratch, "%d", counter);
  if (counter >= 10){
   end_query_bar(app, &dumb_bar, 0);
  }
  
  User_Input in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
  if (in.abort){
   break;
  }
  
  if (match_key_code(&in.event, KeyCode_Up)){
   counter += 1;
  }
  else if (match_key_code(&in.event, KeyCode_Down)){
   counter -= 1;
  }
  else{
   leave_current_input_unhandled(app);
  }
 }
}

// tags: input; loop
// example-of: get_next_input; leave_current_input_unhandled
CUSTOM_COMMAND_SIG(display_key_codes)
CUSTOM_DOC("Example of input handling loop")
{
 /* In the 4coder custom layer, inputs are handled by a view context. A view context is a
thread that hands off control with the main thread of the 4coder core. When a command is
running in a view context thread, it can wait for inputs from the core by calling
get_next_input. If your command gets inputs from the core, then default input handling
isn't happening, so command bindings don't trigger unless you trigger them yourself. */
 
 Query_Bar_Group group(app);
 Query_Bar bar = {};
 bar.prompt = SCu8("KeyCode = ");
 if (!start_query_bar(app, &bar, 0)){
  return;
 }
 
 Key_Code code = 0;
 b32 is_dead_key = false;
 
 for (;;){
  Scratch_Block scratch(app);
  if (code == 0){
   bar.string = SCu8("...");
  }
  else{
   bar.string = push_stringf(scratch, "KeyCode_%s (%d)%s", key_code_name[code], code,
                             is_dead_key?" dead-key":"");
  }
  User_Input in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
  if (in.abort){
   break;
  }
  if (in.event.kind == InputEventKind_KeyStroke){
   code = in.event.key.code;
   is_dead_key = event_is_dead_key(&in.event);
  }
  else{
   /* Marking inputs as handled lets the core determine if certain inputs should
    be passed to additional handlers. This is especially important for text input,
    which is explained more in the example display_text_input. */
   leave_current_input_unhandled(app);
  }
 }
}

// tags: text; input
// example-of: get_next_input; leave_current_input_unhandled; to_writable
CUSTOM_COMMAND_SIG(display_text_input)
CUSTOM_DOC("Example of to_writable and leave_current_input_unhandled")
{
 /* In the 4coder custom layer, inputs are handled by a view context. A view context is a
thread that hands off control with the main thread of the 4coder core. When a command is
running in a view context thread, it can wait for inputs from the core by calling
get_next_input. If your command gets inputs from the core, then default input handling
isn't happening, so command bindings don't trigger unless you trigger them yourself. */
 
 Query_Bar_Group group(app);
 Query_Bar bar = {};
 bar.prompt = SCu8("Weird String: ");
 if (!start_query_bar(app, &bar, 0)){
  return;
 }
 
 u8 buffer[256];
 u64 size = 0;
 
 for (;;){
  User_Input in = get_next_input(app, EventPropertyGroup_Any, EventProperty_Escape);
  if (in.abort){
   break;
  }
  
  String_Const_u8 in_string = to_writable(&in);
  if (in_string.size > 0){
   size = clamp_top(in_string.size, sizeof(buffer));
   block_copy(buffer, in_string.str, size);
   bar.string = SCu8(buffer, size);
  }
  else if (in.event.kind == InputEventKind_KeyStroke){
   /* If we handle a key stroke then the core marks any text input generated from that
key stroke as handled too, and the text input is never passed. By marking key strokes
as unhandled, we ensure we get text input events. */
   leave_current_input_unhandled(app);
  }
 }
}

// tags: string; number; query; user
// example-of: query_user_string; query_user_number
CUSTOM_COMMAND_SIG(string_repeat)
CUSTOM_DOC("Example of query_user_string and query_user_number")
{
 Query_Bar_Group group(app);
 Query_Bar string_bar = {};
 string_bar.prompt = SCu8("String: ");
 u8 string_buffer[KB(1)];
 string_bar.string.str = string_buffer;
 string_bar.string_capacity = sizeof(string_buffer);
 Query_Bar number_bar = {};
 number_bar.prompt = SCu8("Repeat Count: ");
 u8 number_buffer[KB(1)];
 number_bar.string.str = number_buffer;
 number_bar.string_capacity = sizeof(number_buffer);
 
 if (query_user_string(app, &string_bar)){
  if (string_bar.string.size > 0){
   if (query_user_number(app, &number_bar)){
    if (number_bar.string.size > 0){
     i32 repeats = (i32)string_to_integer(number_bar.string, 10);
     repeats = clamp_top(repeats, 1000);
     Scratch_Block scratch(app);
     String_Const_u8 msg = push_stringf(scratch, "%.*s\n", string_expand(string_bar.string));
     for (i32 i = 0; i < repeats; i += 1){
      print_message(app, msg);
     }
    }
   }
  }
 }
}

global Audio_Control the_music_control = {};

CUSTOM_COMMAND_SIG(music_start)
CUSTOM_DOC("Starts the music.")
{
 local_persist Audio_Clip the_music_clip = {};
 if (the_music_clip.sample_count == 0){
  Scratch_Block scratch(app);
  FILE *file = def_search_normal_fopen(scratch, "audio_test/chtulthu.wav", "rb");
  if (file != 0){
   the_music_clip = audio_clip_from_wav_FILE(&global_permanent_arena, file);
   fclose(file);
  }
 }
 
 if (!def_audio_is_playing(&the_music_control)){
  the_music_control.loop = true;
  the_music_control.channel_volume[0] = 1.f;
  the_music_control.channel_volume[1] = 1.f;
  def_audio_play_clip(the_music_clip, &the_music_control);
 }
}

CUSTOM_COMMAND_SIG(music_stop)
CUSTOM_DOC("Stops the music.")
{
 def_audio_stop(&the_music_control);
}

CUSTOM_COMMAND_SIG(hit_sfx)
CUSTOM_DOC("Play the hit sound effect")
{
 local_persist Audio_Clip the_hit_clip = {};
 if (the_hit_clip.sample_count == 0){
  Scratch_Block scratch(app);
  FILE *file = def_search_normal_fopen(scratch, "audio_test/hit.wav", "rb");
  if (file != 0){
   the_hit_clip = audio_clip_from_wav_FILE(&global_permanent_arena, file);
   fclose(file);
  }
 }
 
 local_persist u32 index = 0;
 local_persist Audio_Control controls[8] = {};
 
 Audio_Control *control = &controls[index%8];
 if (!def_audio_is_playing(control)){
  control->loop = false;
  control->channel_volume[0] = 1.f;
  control->channel_volume[1] = 1.f;
  def_audio_play_clip(the_hit_clip, control);
  index += 1;
 }
}


// BOTTOM

