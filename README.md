# Galaksija-Emulator
An emulator of the Z80-based Yugoslav computer

## Quick info
The emulator is quite early in it's development stage and IS missing a lot of features.

It's code is mostly a **mostly rewritten** version of [this](https://emulator.galaksija.org/).

It also retains emulation state backwards-compatibility.

I tried to keep the emulator self-contained, so most of the code is handled
inside of a library rather than the main executable.

It uses no dynamic allocations, which is great for memory safety.

It's internal name is galaxy_\*, since my brain works in English when writing code.

Emulator accuracy is **NOT** guranteed, since I have no real machine to test on.

## Usage

CTRL + L to load state
CTRL + S to save state
CTRL + N to send BREAK
CTRL + R to reset the machine

# Building/Running
You'll need:
 * A modern OS
 * A C compiler
 * CMake
 * SDL2
 * Pkg-Config
 * Git
 * Original firmware - NOT INCLUDED (I need to sort out the legalise first)
 
```
git clone https://github.com/nikp123/Galaksija-Emulator --recursive
cd Galaksija-Emulator
mkdir build
cd build
cmake ..
make
./GalEmu
```

