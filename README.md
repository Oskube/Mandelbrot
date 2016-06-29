SDL2-Mandelbrot
===============
A program for exploring the Mandelbrot fractal. You can select the area where to zoom in with a mouse and move around with WASD keys.

Implementation
--------------
The program splits the current view into small chunks, calculates Mandelbrot and if iteration counts in neighbouring chunks differ it will split those chunks and recalculate them.

Compile and run
----------------
With Ubuntu or other similar distros install libsdl2-dev and compile:
```bash
sudo apt-get install libsdl2-dev
make
bin/mandelbrot
```

Usage
-----
You can check launch options by executing the program with `-h`.

### Controls

|Key       | Action
|:--------:|---------
|WASD      | Move around
|Mouse 1   | Select an area to zoom in *
|Mouse 2   | Go back to previous selection 
|R         | Recalculate current view
|P         | Print chunk info
|Space     | Show chunks top-left corner
|Numpad +  | Increase iterations **
|Numpad -  | Decrease iterations **

_* Use ctrl to disable aspect ratio forcing_  
_** Use modifiers keys change iterations easier: ctrl by 10, ctrl+shift by 100, ctrl+alt by 1000._
