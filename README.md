## Parallax Inc. Propeller2 emulator

The project is a try to write an emulator for the Parallax Inc. Propeller2
processor in C++, using the Qt5 framework to make it platform independent.

In it's current state, the code does next to nothing. Only (part of) the
constants and structures to be used are defined. There is an opcode decoder
with several hundred opcode handling functions, some of which are already
filled with live.

The goal is to be able to exercise Propeller2 spin2 binary files by
emulating instructions to the extent possible without being real hardware
with ports, timings etc.

Implementing also a disassembler, and perhaps an assembler, as well as
a debugger interface in the Qt5 UI are also on the list of things to do.

#### Screenshot of v0.0.1 disassembly of the ROM
![Screenshot v0.0.1](https://github.com/pullmoll/p2emu/blob/master/p2em_001.png)

#### Screenshot of v0.2.2 assemmbly of spin2_interpreter.spin2
![Screenshot v0.2.0](https://github.com/pullmoll/p2emu/blob/master/p2em_020.png)
