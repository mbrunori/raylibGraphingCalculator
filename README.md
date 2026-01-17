# Graphing Calculator in C using Raylib

## Description
This is a work-in-progress **graphing calculator** written in C and built with [raylib](https://www.raylib.com/).  
It can draw up to **six functions** and supports **sin, cos, tan, square roots, absolute values, addition, subtraction, multiplication and division**.  
It also features **variable scaling** and can compute **intersections** between any two functions.

## Features
- Plot up to **6 functions** simultaneously  
- Support for basic trigonometric functions, arithmetic operations, square roots and absolute values  
- Adjustable viewing window and scale  
- Intersection detection between two functions  

## Requirements
- **C compiler** (gcc or clang)
- **raylib** (installed on your system)
- **pkg-config**
- **make**

## Build Instructions
after cloning the repository, edit the makefile with your raylib installation path after that you can use the following command:  
`mingw32-make` to build the file (be sure to have a obj folder in your main folder)

## How to Use
When you open the application, navigate the menus using the **arrow keys**, **Enter**, and **Esc**.  
Select a field and press **Enter** to edit it; press **Del** to delete a function.

You can write expressions such as:
- `x^2 + 4`
- `sin(x)`
- `|x|`
- `r(x)` for square roots

Press **Esc** to return to the input menu and **Spacebar** to draw the functions.

### Additional Controls
- Press **G** to open the **Gsolve** menu and compute intersections  
- Press **V** to open the **vWindow** menu and edit the scale values  
  (use **Backspace** to delete digits)
