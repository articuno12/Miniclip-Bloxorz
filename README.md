# About Game
##### This is a Replica of miniclip's Bloxorz game made using opengl.
In this world, there is a movable cuboidal block. The goal of the player is to
make it fall into a square hole. The world consists of square tiles which can
be arranged in any manner.

# Rules
* The Player wins the game when the block falls through the square hole
  (the block has to be vertical for it to fall through the hole).
* If the block falls down the edges then the game ends.
* The tiles are of 4 types - normal tiles, fragile tiles, bridges and switches.
* In order for a bridge to be constructed, a switch (in a different location)
  needs to be pressed by the movable block.
* Bridges can have 2 states - open and closed,which can be toggled as per the requirements
  of the player.
* If the block stands up vertically on a fragile  tile , then the tile breaks.
  To move on fragile tiles, the block should lie horizontally (on the longer side).
* The block can only skip one row/column (one cell, in any direction) in case of jump.

# Run Game
* To Compile : `make`
* To run game : `./game`

# Libraries Used & Installation

* Glut ,Glew,Glfw : OpenGL Library
  <br>
  Glm : Maths Library to support matrix computations
  <br>
  `sudo apt install freeglut3-dev libglew-dev libglm-dev libglfw3-dev`

* Soil : To render images
   <br>
  `sudo apt-get install libsoil-dev`
* Ao : For sound
  <br>
  `sudo apt-get install libao-dev`

# Views

* ### Block View
  This is a view from the block’s position where only a part of the world in
  front is visible. In other words, in this view, we see what the block sees, as
   if we were the block.

* ### Top View
  This is a top-down view, as if we were looking vertically downwards
  from a position in the sky. This gives a clear pic-ture of the path

* ### Tower view
  Here, the camera is sitting on a tower, to the side of the plane of playing,
  observing it at an angle.

* ### Follow Cam view
  This is a view of the block and the region in front of it from a location
  behind and above it, as if the camera is following the back

* ### Helicopter Cam view
  Here, the camera is movable with the mouse in an intutive manner.
  Clicking and dragging should change the look angle, the up vector should
  remain up always, and the scroll wheel will move the camera closer or farther
  away from the scene.
 
# Controls
* Move left : LEFT ARROW KEY
* Move Right : RIGHT ARROW KEY
* Up Left    : UP ARROW KEY
* Up Down    : DOWN ARROW KEY
* Pause game : p
* quit game  : q
* top view   : t
* tower view : o
* Follow cam view : b
* Helicopter Cam View : s
* zoom in    : n
* zoom out   : f
* Rotate camera vertically : v
* Rotate camera horizontally : m

# Preview
  ![Alt Text](https://github.com/articuno12/Miniclip-Bloxorz/raw/master/optimised.gif)
