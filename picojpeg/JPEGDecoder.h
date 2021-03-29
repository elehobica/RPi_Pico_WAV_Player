/*
JPEGDecoder.h

JPEG Decoder for Arduino
Public domain, Makoto Kurauchi <http://yushakobo.jp>

Adapted by Bodmer for use with a TFT screen

Latest version here:
https://github.com/Bodmer/JPEGDecoder

*/

#ifndef JPEGDECODER_H
#define JPEGDECODER_H

#include <cstdio>
#include "fatfs/ff.h"
#include "picojpeg.h"

//#define DEBUG

//------------------------------------------------------------------------------
#ifndef jpg_min
  #define jpg_min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//------------------------------------------------------------------------------
typedef unsigned char uint8;
typedef unsigned int uint;
//------------------------------------------------------------------------------

class JPEGDecoder {

typedef enum {
  JPEG_ARRAY = 0,
  JPEG_SD_FILE
} jpg_source_t;

private:
  FIL g_fil;
  pjpeg_scan_type_t scan_type;
  pjpeg_image_info_t image_info;
  
  int is_available;
  int mcu_x;
  int mcu_y;
  uint64_t g_nInFileSize;
  uint64_t g_nInFileOfs;
  uint8_t g_reduce = 0;
  uint row_pitch;
  uint decoded_width, decoded_height;
  uint row_blocks_per_mcu, col_blocks_per_mcu;
  uint8 status;
  jpg_source_t jpg_source = JPEG_ARRAY;
  uint8_t* jpg_data;
  
  static uint8 pjpeg_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data);
  uint8 pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data);
  int decode_mcu(void);
  int decodeCommon();
public:

  uint16_t *pImage;
  JPEGDecoder *thisPtr;

  int width;
  int height;
  int comps;
  int MCUSPerRow;
  int MCUSPerCol;
  pjpeg_scan_type_t scanType;
  int MCUWidth;
  int MCUHeight;
  int MCUx;
  int MCUy;
  
  JPEGDecoder();
  ~JPEGDecoder();

  int available(void);
  int read(void);
  
// reduce:
//  0: normal MCU size
//  1: 1/8 MCU size for x, y
  int decodeSdFile(const char *jpgFile, uint64_t pos = 0, size_t size = 0, uint8_t reduce = 0);
  int decodeArray(const uint8_t array[], uint32_t  array_size, uint8_t reduce = 0);
  void abort(void);

};

extern JPEGDecoder JpegDec;

#endif // JPEGDECODER_H
