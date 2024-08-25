# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
* Show different color in Battery Icon when charging
### Changed
* Support pico-sdk 2.0.0
### Fixed
* Revise active battery check voltage divider to torelate up to 5.5V
* Fix ID3v2 tag (RIFF ID3v2) read failure for large file over 32bit signed size range

## [v0.9.5] - 2024-06-06
### Added
* Add support of Waveshare RP2040-LCD-0.96 board
* Support tag information by ID3v2 tag (RIFF ID3v2) as higher priority than that by LIST chunk
* Also support coverart image from the JPEG bitstream in ID3v2 tag as higher priority than separate JPEG file in local directory
* Add button layout configurations for natural button assignment in FileView and Config mode when buttons are horizontally placed
### Changed
* Replace transistor with MOS-FET in battery operation circuit (Q2) thanks to conditional pullup
* Self-configure by checking if active battery check circuit is populated, not define USE_ACTIVE_BATTERY_CHECK macro
* Use submodule for pico_flash_param library
### Fixed
* Add workaround for mount fail case of Samsung PRO Plus card

## [v0.9.4] - 2024-04-09
### Added
* Support Hi-Res WAV up to 24bit / 192KHz
* Display bit resolution and sampling frequency
* Support Mono WAV
* Add LCD Config in Config Menu
* Force to reset ConfigParam when format revision changed to avoid mulfunction
### Changed
* Use submodule for pico_audio_i2s_32b library
* Change pullup register for buttons into conditional pullup for power-on by any HP buttons
### Fixed
* Correct track number display when no TAG information in WAV file
* Don't display empty album art when no JPEG
* Fix resume play freeze when card contents have been changed

## [v0.9.3] - 2024-03-06
### Changed
* Support pico-sdk 1.5.1 (previously 1.4.0)
* Update FatFs R0.15 (previously R0.14b)
* Remove external pullup resistors for PLUS, MINUS buttons
* Remove external pullup resistors for MISO, MOSI of microsd SPI
* Clean up libraries. pico_fatfs and pico_st7735_80x160 are switched to submodules.
* Change I2C DMA scheme from single buffer to double buffers
### Fixed
* Fix build error of "cannot find -lmy_pico_stdio_usb_headers"
* Delete obsolete CENTER button completely not to disturb other button actions when pullup is not attached. [issue#4](https://github.com/elehobica/RPi_Pico_WAV_Player/issues/4)

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
