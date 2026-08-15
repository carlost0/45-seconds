# 45 seconds

[itch.io page](https://devbycarlos.itch.io/gmtk-45-seconds)

---

## description
you're a lost cave miner and have 45 seconds to find gold before it catches you
use W/S to move and A/D to look around, you can destroy blocks drawn with 'r' by pressing SPACE. 

it's a 3d game written in C with my own [ascii graphics library](https://github.com/carlost0/aschii) and sadly, is used SDL2 for sound and runs entirely in the terminal.
you may have to adjust your terminal's text size, in the windows console, you can do that by pressing 'ctrl' + '+' and  'ctrl' + '-' to increase and decrease the text size respectively.

there's currently a bug, where when you change scenes (eg. going into the options menu) the game doesn't continue until you press a key. since i'm to lazy to fix that issue you will have to press any key when changing scenes.
because the game was made on linux and only later ported to windows, it may run a bit differently on windows.

---

## building

currently, theres no option for building on windows.

on linux you can build for both linux with `make linux` and for windows `make windows`.
both of these targets put the executable in build/linux and build/windows respectively.

the game depends on SDL2, SDL2_mixer and aschii, but they are all included in the bin/ direcory

# screenshots

### title screen

[title screen](https://img.itch.zone/aW1hZ2UvNDgyMTIzOC8yODc0ODMzNS5wbmc=/original/fqSEoW.png "title screen")

### gameplay

[gameplay](https://img.itch.zone/aW1nLzI4NzQ3ODgxLnBuZw==/original/urN5F1.png "gameplay")
