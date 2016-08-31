Distribution Date: 30.8.2016 (dd.mm.yyyy)

Thank you for contributing to the 4coder project!

To submit bug reports or to request particular features email editor@4coder.net.

Watch 4coder.net blog and @AllenWebster4th twitter for news about 4coder progress.

---------------------------------
FAIR WARNING
---------------------------------

THINGS WILL GET CRASHY FAST IF ANY .ttf FILES ARE MISSING.
THIS EFFECT WILL ALSO OCCUR IF YOU LAUNCH FROM A DIRECTORY
THAT DOESN'T CONTAIN THE .ttf FILES. (This problem will be
fixed eventually).

This build is extremely "janky" for lack of a better term. From what limitted testing
I have been able to do I think it should run on a Windows 7 machine. It has not been
launched at all on any other version of Windows. I have done what I can to get rid of
the bugs and crashes that would make it unusable, but there are certainly more in there
if you start digging and pressing hard enough.

**Please USE SOURCE CONTROL WITH 4CODER for now**

-----------------------------------------------------
INSTRUCTIONS FOR USE
-----------------------------------------------------

****Command line options****
4ed [<files-to-open>] [options]

-d/-D <filename> -- use a dll other than 4coder_custom.dll for your customizations
 -d -- if the file isn't found look for 4coder_custom.dll
 -D -- only look for the specified

-i <line-number> -- line number to jump to in first file to open specified on command line

-w <w, h> -- width and height of the 4coder window
-p <x, y> -- position of the 4coder window

-W -- open in full screen, overrides -w and -p, although the size will still be the default size of the window

-T -- invoke special tool isntead of launching 4coder normally
   -T version : prints the 4coder version string

****Command Bindings****
Basic Navigation:
mouse left click - move cursor
mouse right click - set mark
arrows - move cursor
home & end - move cursor to beginning/end of line
page up & page down - move up/down by close to the height of the entire screen
control + left/right - move cursor left/right to first whitespace
control + up/down - move cursor up or down to first blank line

Fancy Navigation:
control + f : begin find mode, uses interaction bar
control + r : begin reverse-find mode, uses interaction bar

control + F : list all locations of a word in all open buffers, uses interaction bar
    > This command creates a *search* buffer that displays the locations and the line of each
    > occurence of the requested word.  By positioning the cursor and pressing return the user
    > jump to the word's occurence.

While in find mode or reverse-find mode, pressing enter ends the mode
leaving the cursor wherever the find highlighter is, and pressing escape
ends the mode leaving the cursor wherever it was before the find mode began.

control + g - goto line number, uses interaction bar
control + m - swap cursor and mark

control + e - center the view vertically on the cursor
control + E - in a view with unwrapped lines move the view to a position just left of the cursor

Basic Editing:
characters keys, delete, and backspace
control + c : copy between cursor and mark
control + x : cut between cursor and mark
control + v : paste at cursor
control + V : use after normal paste to cycle through older copied text
control + d : delete between cursor and mark
control + SPACE : set mark to cursor
control + backspace : backspace one word
control + delete : delete one word
alt + backspace : snipe one word

Undo and History:
control + z : undo
control + y : redo
control + h: history back step
control + H: history forward step

Fancy Editing:
control + u : to uppercase between cursor and mark
control + j : to lowercase between cursor and mark
control + q : query replace
control + a : replace in range
control + ~ : clean the trailing whitespace off of all lines

Fancy Editing in Code Files:
control + [ : write "{}" pair with cursor in line between
control + { : as control + [ with a semicolon after "}"
control + } : as control + [ with a "break;" after "}"
control + 0 : write "= {0};" at the cursor
control + i : wrap the range specified by mark and cursor in #if 0 #endif

alt + 1 : if cursor is inside a string, treat the string as a filename and
    > try to open the file with that name in the other panel

Whitespace Boringness:
Typing characters: },],),; and inserting newlines cause the line to autotab
TAB: word complete
control + TAB : auto indent lines between cursor and mark
shift + TAB: auto indent cursor line
control + 1 : set the file to dos mode for writing to disk
control + ! : set the flie to nix mode for writing to disk

Viewing Options:
alt + c - open theme selection UI
alt + d - open debug view

control + p : vertically split the current panel (max 16)
control + _ : horizontally split the current panel (max 16)
control + P : close the currently selected panel
control + , : switch to another panel

control + l : toggle line wrapping
control + ? : toggle highlight whitespace mode

f2 : toggle mouse suppresion mode

alt + s : show the scrollbar in this view
alt + w : hide the scrollbar in this view

Build Tools:
alt + m :
[On Windows] search in the current hot directory and up through all parent
    > directories for a build.bat, and execute that bat if it discovered, sending
    > output to the buffer *compilation*
[On Linux] The behavior is similar but the search looks for build.sh and if that
    > fails it looks for a Makefile

alt + . : change to the build panel
alt + , : close the build panel
alt + n : goto the next error listed in the build panel
alt + N : goto the previous error listed in the build panel
alt + M : goto the first error listed in the build panel

alt + z : execute any command line command you specify and send the output to the buffer you specify
alt + Z : repeat the command previously executed by the alt + z command

File Managing:
control + n : create a new file, begins interactive input mode
control + o : open file, begins interactive input mode
alt + o : open file in other panel, same as control + o but runs first changes the active view
control + O : reopen the current file
	(discarding any differences the live version has from the file system's version)
control + s : save
control + w : save as, begins interative input mode
control + i : switch active file in this panel, begins interactive input mode
control + k : kill (close) a file, begins interactive input mode
control + K : kill (close) the file being viewed in the currently active panel

While in interactive input mode, there are several ways to select an option.
The options can be clicked.  One option is always highlighted and pressing
return or tab will select the highlighted option.  Arrow keys navigate the
highlighted option.  Typing in characters narrows down the list of options.

Menu UI
Keyboard options:
    > left control + left alt act as AltGr

Theme selection UI
esc - close UI view return to major view if one was open previously
