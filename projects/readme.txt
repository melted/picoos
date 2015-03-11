Here are two projects for demonstration.
They show how you can use pico]OS and the make-system in your own projects.

demo1 uses the pico]OS make system, and the makefile is quite simple.
You can copy & paste this example to have a basis for your own projects.
To compile the example, change into the demo1 directory and type "make".

demo2 is just the same as demo1. But demo2 does not use the pico]OS make system.
The makefile is a standard gnu makefile, and pico]OS is only linked as external
library to the project. This demo can be interesting for you if you whish to
use your own makefiles and your own make environment to build your project.

Note: To be able to build demo2 you must have Windows XP and MinGW-GCC installed.
