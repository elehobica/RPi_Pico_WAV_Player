/*
JPEGDecoder.cpp

JPEG Decoder for Arduino
https://github.com/MakotoKurauchi/JPEGDecoder
Public domain, Makoto Kurauchi <http://yushakobo.jp>

Latest version here:
https://github.com/Bodmer/JPEGDecoder

Bodmer (21/6/15): Adapted by Bodmer to display JPEGs on TFT (works with Mega and Due) but there
is a memory leak somewhere, crashes after decoding 1 file :-)

Bodmer (29/1/16): Now in a state with sufficient Mega and Due testing to release in the wild

Bodmer (various): Various updates and latent bugs fixed

Bodmer (14/1/17): Tried to merge ESP8266 and SPIFFS support from Frederic Plante's broken branch,
				worked on ESP8266, but broke the array handling :-(

Bodmer (14/1/17): Scrapped all FP's updates, extended the built-in approach to using different
				data sources (currently array, SD files and/or SPIFFS files)

Bodmer (14/1/17): Added ESP8266 support and SPIFFS as a source, added configuration option to
				swap bytes to support fast image transfer to TFT using ESP8266 SPI writePattern().

Bodmer (15/1/17): Now supports ad hoc use of SPIFFS, SD and arrays without manual configuration.

Bodmer (19/1/17): Add support for filename being String type

Bodmer (20/1/17): Correct last mcu block corruption (thanks stevstrong for tracking that bug down!)

Bodmer (20/1/17): Prevent deleting the pImage pointer twice (causes an exception on ESP8266),
				tidy up code.

Bodmer (24/1/17): Correct greyscale images, update examples
*/

#include <cstring>
#include "picojpeg.h"
#include "JPEGDecoder.h"

#define SWAP_BYTES

JPEGDecoder JpegDec;

JPEGDecoder::JPEGDecoder(){
	mcu_x = 0 ;
	mcu_y = 0 ;
	is_available = 0;
	thisPtr = this;
}


JPEGDecoder::~JPEGDecoder(){
	if (pImage) delete[] pImage;
}


uint8_t JPEGDecoder::pjpeg_callback(uint8_t* pBuf, uint8_t buf_size, uint8_t *pBytes_actually_read, void *pCallback_data) {
	JPEGDecoder *thisPtr = JpegDec.thisPtr ;
	thisPtr->pjpeg_need_bytes_callback(pBuf, buf_size, pBytes_actually_read, pCallback_data);
	return 0;
}


uint8_t JPEGDecoder::pjpeg_need_bytes_callback(uint8_t* pBuf, uint8_t buf_size, uint8_t *pBytes_actually_read, void *pCallback_data) {
	uint n;

	//pCallback_data;

	n = jpg_min(g_nInFileSize - g_nInFileOfs, buf_size);

	if (jpg_source == JPEG_ARRAY) { // We are handling an array
		for (int i = 0; i < (int) n; i++) {
			pBuf[i] = *((uint8_t *) jpg_data++);
			//printf("0x%x\n", pBuf[i]);
		}
	}

	UINT br;
	if (jpg_source == JPEG_SD_FILE) f_read(&g_fil, pBuf, n, &br);

	*pBytes_actually_read = (uint8_t) br;
	g_nInFileOfs += n;
	return 0;
}

int JPEGDecoder::decode_mcu(void) {

	status = pjpeg_decode_mcu();

	if (status) {
		is_available = 0 ;

		if (status != PJPG_NO_MORE_BLOCKS) {
			//#ifdef DEBUG
			printf("pjpeg_decode_mcu() failed with status %d\n", status);
			//#endif

			return -1;
		}
	}
	return 1;
}


int JPEGDecoder::read(void) {
	int y, x;
	uint16_t *pDst;

	if(is_available == 0 || mcu_y >= image_info.m_MCUSPerCol) {
		abort();
		return 0;
	}
	
	// Copy MCU's pixel blocks into the destination bitmap.
	for (y = 0; y < image_info.m_MCUHeight; y += 8) {

		int by_limit = jpg_min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));
		if (g_reduce) by_limit /= 8;

		for (x = 0; x < image_info.m_MCUWidth; x += 8) {

			// Compute source byte offset of the block in the decoder's MCU buffer.
			uint src_ofs = (x * 8U) + (y * 16U);
			const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
			const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
			const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

			int bx_limit = jpg_min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));
			if (g_reduce) bx_limit /= 8;

			if (!g_reduce) {
				pDst = pImage + row_pitch * y + x;
			} else {
				pDst = pImage + row_pitch * y/8 + x/8;
			}

			if (image_info.m_scanType == PJPG_GRAYSCALE) {
				int bx, by;
				for (by = 0; by < by_limit; by++) {

					for (bx = 0; bx < bx_limit; bx++) {
#ifdef SWAP_BYTES
						*pDst++ = (*pSrcR & 0xF8) | (*pSrcR & 0xE0) >> 5 | (*pSrcR & 0xF8) << 5 | (*pSrcR & 0x1C) << 11;
#else
						*pDst++ = (*pSrcR & 0xF8) << 8 | (*pSrcR & 0xFC) <<3 | *pSrcR >> 3;
#endif
						if (!g_reduce) {
							pSrcR++;
						} else {
							pSrcR += 8;
						}
					}

					if (!g_reduce) {
						pSrcR += (8 - bx_limit);
					} else {
						pSrcR += (1 - bx_limit)*8 + (8-1)*8;
					}
					pDst += (row_pitch - bx_limit);
				}
			}
			else {
				int bx, by;
				for (by = 0; by < by_limit; by++) {

					for (bx = 0; bx < bx_limit; bx++) {
#ifdef SWAP_BYTES
						*pDst++ = (*pSrcR & 0xF8) | (*pSrcG & 0xE0) >> 5 | (*pSrcB & 0xF8) << 5 | (*pSrcG & 0x1C) << 11;
#else
						*pDst++ = (*pSrcR & 0xF8) << 8 | (*pSrcG & 0xFC) <<3 | *pSrcB >> 3;
#endif
						if (!g_reduce) {
							pSrcR++; pSrcG++; pSrcB++;
						} else {
							pSrcR += 8; pSrcG += 8; pSrcB += 8;
						}
					}

					if (!g_reduce) {
						pSrcR += (8 - bx_limit);
						pSrcG += (8 - bx_limit);
						pSrcB += (8 - bx_limit);
					} else {
						pSrcR += (1 - bx_limit)*8 + (8-1)*8;
						pSrcG += (1 - bx_limit)*8 + (8-1)*8;
						pSrcB += (1 - bx_limit)*8 + (8-1)*8;
					}
					pDst += (row_pitch - bx_limit);
				}
			}
		}
	}

	MCUx = mcu_x;
	MCUy = mcu_y;

	mcu_x++;
	if (mcu_x == image_info.m_MCUSPerRow) {
		mcu_x = 0;
		mcu_y++;
	}

	if(decode_mcu()==-1) is_available = 0 ;

	return 1;
}

int JPEGDecoder::decodeSdFile(const char *jpgFile, uint64_t pos, size_t size, uint8_t reduce){
	FRESULT fr;

	fr = f_open(&g_fil, (TCHAR *) jpgFile, FA_READ);
	if (fr != FR_OK) {
		#ifdef DEBUG
		printf("ERROR: SD file not found!\n");
		#endif
		return -1;
	}

	jpg_source = JPEG_SD_FILE; // Flag to indicate a SD file

	g_nInFileOfs = 0;

	if (pos == 0) {
		g_nInFileSize = f_size(&g_fil);
	} else {
		fr = f_lseek(&g_fil, (FSIZE_t) pos);
		if (fr != FR_OK) {
			#ifdef DEBUG
			printf("ERROR: f_lseek failed\n");
			#endif
			return -1;
		}
		g_nInFileSize = size;
	}

	g_reduce = reduce;
	return decodeCommon();

}

int JPEGDecoder::decodeArray(const uint8_t array[], uint32_t  array_size, uint8_t reduce) {

	jpg_source = JPEG_ARRAY; // We are not processing a file, use arrays

	g_nInFileOfs = 0;

	jpg_data = (uint8_t *)array;

	g_nInFileSize = array_size;

	g_reduce = reduce;
	return decodeCommon();
}


int JPEGDecoder::decodeCommon() {

	width = 0;
	height = 0;
	comps = 0;
	MCUSPerRow = 0;
	MCUSPerCol = 0;
	scanType = (pjpeg_scan_type_t)0;
	MCUWidth = 0;
	MCUHeight = 0;

	status = pjpeg_decode_init(&image_info, pjpeg_callback, NULL, g_reduce);

	if (status) {
		//#ifdef DEBUG
		printf("pjpeg_decode_init() failed with status %d\n", status);
		/*
		if (status == PJPG_UNSUPPORTED_MODE) {
			printf("Progressive JPEG files are not supported.\n");
		}
		*/
		//#endif

		return 0;
	}

	decoded_width =  image_info.m_width;
	decoded_height =  image_info.m_height;
	
	row_pitch = (!g_reduce) ? image_info.m_MCUWidth : image_info.m_MCUWidth/8;
	pImage = new uint16_t[image_info.m_MCUWidth * image_info.m_MCUHeight];

	memset(pImage , 0 , image_info.m_MCUWidth * image_info.m_MCUHeight * sizeof(*pImage));

	row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
	col_blocks_per_mcu = image_info.m_MCUHeight >> 3;

	is_available = 1 ;

	width = decoded_width;
	height = decoded_height;
	comps = 1;
	MCUSPerRow = image_info.m_MCUSPerRow;
	MCUSPerCol = image_info.m_MCUSPerCol;
	scanType = image_info.m_scanType;
	MCUWidth = image_info.m_MCUWidth;
	MCUHeight = image_info.m_MCUHeight;

	return decode_mcu();
}

void JPEGDecoder::abort(void) {

	mcu_x = 0 ;
	mcu_y = 0 ;
	is_available = 0;
	if(pImage) delete[] pImage;
	pImage = NULL;
	
	if (jpg_source == JPEG_SD_FILE) f_close(&g_fil);
}
