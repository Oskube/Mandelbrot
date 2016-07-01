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
|Mouse 2   | Go back to the previous selection
|O         | Save current view in to a BMP file
|P         | Print chunk info
|R         | Recalculate current view
|Space     | Show chunks top-left corner
|Numpad +  | Increase the number of iterations **
|Numpad -  | Decrease the number of iterations **

_* Use ctrl to disable aspect ratio forcing_  
_** Use modifiers keys change iterations easier: ctrl by 10, ctrl+shift by 100, ctrl+alt by 1000._
