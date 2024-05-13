# Config Menu Items

## General
### Time to Power Off
* Seconds to power off since last action
### Time to Leave Config
* Seconds to leave Congig Menu to go back previous mode
### GPIO Button Layout
* If Horizontal, GPIO Plus button to go down at FileView and Config menu, GPIO Minus button to go up
* If Vertial, GPIO Plus button to go up at FileView and Config menu, GPIO Minus button to go down
### HP Button Layout
* If Horizontal, Headphone Plus button to go down at FileView and Config menu, Headphone Minus button to go up
* If Vertial, Headphone Plus button to go up at FileView and Config menu, Headphone Minus button to go down

## Display
### LCD Config
* ST7735S 0.96" 160x80 LCD could require different configuration for its position offset, rotation, mirror and color order. It is hard to distinguish which type of configuration is needed from the module outlook.
* Suitable configuration out of 3 types(1, 2, 3) can be selected.
### Rotation
* Select 0 degree or 180 degree
### Backlight Low Level
* Low level of backlight which is applied since Time to backlight Low seconds since last action
### Backlight High Level
* High level of backlight which is applied within Time to backlight Low seconds since last action
### Time to Backlight Low
* Seconds to change backlight Low

## Play
### Time to Next Play
* Seconds to play next folder WAV files after current folder play finished
### Next Play Album
* Choose next play action
  * Stop to play none
  * Sequential to play next order's folder and stop when no next folders
  * SequentialRepeat to play next order's folder and repeat from the first folder when no next folders
  * Repeat to repeat current folder
  * Random to choose random folder where the depth of folder is defined by Ramdom Dir Depth
### Random Dir Depth
* Folder depth to go up and go down when Random is choosed at Next Play Album
* If folder hierarchy is artist -> album -> WAV files: 
  * Depth 1 to random play among current artist folders
  * Depth 2 to random play among whole artist folders
