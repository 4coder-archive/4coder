Distribution Date: 06.03.2016 (dd.mm.yyyy)

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

****Changes in 4.0.0****
alt + x: changed to arbitrary command (NOW WORKS ANYWHERE!)
Opens a command prompt from which you can execute:
   "open menu" to open the menu (old behavior of alt+x)
   "open all code" loads all cpp and h files in current directory
   "close all code" closes all cpp and h files currently open
   "open in quotes" opens the file who's name under the cursor is surrounded by quotes
   "dos lines" dosify the file end of iles
   "nix lines" nixify the file end of iles

alt + z: execute arbitrary command-line command
Specify an output buffer and a command to execute
and the results will be dropped into the specified buffer.

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

****Old Information****
Basic Navigation:
mouse click - move cursor
arrows - move cursor
home & end - move cursor to beginning/end of line
page up & page down - page up & page down respectively
control + left/right - move cursor left/right to first whitespace
control + up/down - move cursor up or down to first blank line

Fancy Navigation:
control + f : begin find mode, uses interaction bar
control + r : begin reverse-find mode, uses interaction bar

While in find mode or reverse-find mode, pressing enter ends the mode
leaving the cursor wherever the find highlighter is, and pressing escape
ends the mode leaving the cursor wherever it was before the find mode began.

control + g - goto line number
control + m - swap cursor and mark

Basic Editing:
characters keys, delete, and backspace
control + c : copy between cursor and mark
control + x : cut between cursor and mark
control + v : paste at cursor
control + V : use after normal paste to cycle through older copied text
control + d : delete between cursor and mark
control + SPACE : set mark to cursor

Undo and History:
control + z : undo
control + y : redo
control + Z: undo / history timelines
control + h: history back step
control + H: history forward step
alt + left: increase rewind speed (through undo)
alt + right: increase fastforward speed (through redo)
alt + down: stop redining / fastforwarding

Fancy Editing:
control + u : to uppercase between cursor and mark
control + j : to lowercase between cursor and mark
control + q: query replace
control + a: replace in range
control + =: write increment
control + -: decrement increment
control + [: write {} pair with cursor in line between
control + {: as <control + [> with a semicolon after "}"
control + }: as <control + [> with a "break;" after "}"
control + 9: wrap the range specified by mark and cursor in parens
control + i: wrap the range specified by mark and cursor in #if 0 #endif

Whitespace Boringness:
Typing characters: },],),; and inserting newlines cause the line to autotab
TAB: word complete
control + TAB : auto indent lines between cursor and mark
shift + TAB: auto indent cursor line
control + 1 : set the file to dos mode for writing to disk
control + ! : set the flie to nix mode for writing to disk

Viewing Options:
alt + c - open theme selection UI
control + p : vertically split the current panel (max 16)
control + '-' : horizontally split the current panel (max 16)
control + P : close the currently selected panel
control + , : switch to another panel
control + l : toggle line wrapping
control + L : toggle end of line mode
	mode 1: treat all \r\n and all \n as newline, show \r when not followed by \n
	mode 2: treat all \r and \n as newline
	mode 3: treat all \n as newline, show all \r
control + ? : toggle highlight whitespace mode

Tools:
alt + m : search in the current hot directory and up through all parent
    > directories for a build.bat, and execute that bat if it discovered, sending
    > output to the buffer *compilation*

File Managing:
control + n : create a new file, begins interactive input mode
control + o : open file, begins interactive input mode
control + O : reopen the current file
	(discarding any differences the live version has from the file system's version)
control + s : save
control + w : save as, begins interative input mode
control + i : switch active file in this panel, begins interactive input mode
control + k : kill (close) a file, begins interactive input mode
control + K : kill (close) the file being viewed in the currently active panel

While in interactive input mode, pressing enter confirms the input for the command, and
pressing escape (once) will end the input mode and abort the command.  If the file does
not exist either the nearest match will be opened, or no file will be opened if none is
considered a match.  Use backspace to go back through directories.

Menu UI
Keyboard options:
    > left control + left alt act as AltGr

Theme selection UI
esc - close UI view return to major view if one was open previously

Themes are saved in p4c files, I encourage either of the following pronciations:
"pack"
"pee-fork"


