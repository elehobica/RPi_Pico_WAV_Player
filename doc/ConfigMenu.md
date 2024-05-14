# Config Menu Items

## General
### Time to Power Off
* Time to power off after last action
### Time to Leave Config
* Time to leave Config Menu to go back to previous mode
### Push Button Layout
* "Horizontal" to assign Push Plus button to ascend (down) at FileView and Config mode, Push Minus button to descend (up)
* "Vertial" to assign Push Plus button to descend (up) at FileView and Config mode, Push Minus button to ascend (down)
* Regardless of the selection, volume operation in Play mode is always assigned as Plus button to increace, Minus button to decrease
### HP Button Layout
* "Horizontal" to assign Headphone Plus button to ascend (down) at FileView and Config mode, Headphone Minus button to descend (up)
* "Vertial" to assign Headphone Plus button to descend (up) at FileView and Config mode, Headphone Minus button to ascend (down)
* Regardless of the selection, volume operation in Play mode is always assigned as Plus button to increace, Minus button to decrease

## Display
### LCD Config
* ST7735S 0.96" 160x80 LCD could require different configuration for its position offset, rotation, mirror and color order. It is hard to distinguish which type of configuration is needed from the module outlook.
* Select suitable configuration out of 3 types (1, 2, 3)
### Rotation
* Select 0 degree or 180 degree
### Backlight Low Level
* Low level of backlight applied in "Time to Backlight Low" time after last action
### Backlight High Level
* High level of backlight applied within "Time to Backlight Low" time after last action
### Time to Backlight Low
* Time to change backlight to Low Level

## Play
### Time to Next Play
* Time to play next folder WAV files after current folder play finished
### Next Play Album
* Choose next play action
  * "Stop" to play none
  * "Sequential" to play next order's folder and stop when no next folders
  * "SequentialRepeat" to play next order's folder and repeat from the first folder when no next folders
  * "Repeat" to repeat current folder
  * "Random" to choose random folder where the depth of folder is defined by Ramdom Dir Depth
### Random Dir Depth
* Folder depth to go up and go down when Random is choosed at Next Play Album
* If folder hierarchy is artist -> album -> WAV files: 
  * Depth 1 to random play among current artist folders
  * Depth 2 to random play among whole artist folders
