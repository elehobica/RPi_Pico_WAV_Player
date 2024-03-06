# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]


## [v0.9.3] - 2024-03-06
### Fixed
* Fix build error of "cannot find -lmy_pico_stdio_usb_headers"
* Delete obsolete CENTER button completely not to disturb other button actions when pullup is not attached. [issue#4](https://github.com/elehobica/RPi_Pico_WAV_Player/issues/4)

### Changed
* Support pico-sdk 1.5.1 (previously 1.4.0)
* Update FatFs R0.15 (previously R0.14b)
* Remove external pullup resistors for PLUS, MINUS buttons
* Remove external pullup resistors for MISO, MOSI of microsd SPI
* Clean up libraries. pico_fatfs and pico_st7735_80x160 are switched to submodules.
* Change I2C DMA scheme from single buffer to double buffers


## [v0.9.2] - 2023-11-25
### Fixed
* Small fix for fatfs CS handling


## [v0.9.1] - 2021-09-21
### Added
* Add gpio_disable_pulls() to SD-Card SPI pins

### Changed
* Adjust ADC conversion coefficient for static battery check


## [v0.9.0] - 2021-06-20
### Added
* Support ChargeMode which allows to remove USB charge selection switch

### Changed
* No hardware modification needed in Raspberry Pi Pico board
  * External circuit around battery charger revised to be compliant to pico-datasheet.pdf
  * Use built-in battery voltage check circuit (static battery check) as default
* Increase display charactor length of title, artist and album. add measure to prevent buffer overflow
* Remove GPIO center button and integrated into GP26 ADC level detection to detect wake up trigger


## [v0.8.1] - 2021-06-03
* Initial release