/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _IMAGEFITTER_H_DEFINED_
#define _IMAGEFITTER_H_DEFINED_

#include <cstdint>
#include <cstdlib>

#define IMAGE_FITTER_SWAP_BYTES

// reference to Singleton instance
#define imgFit ImageFitter::instance()

//=================================
// Interface of ImageFitter class
//=================================
class ImageFitter {
private:
	uint16_t width, height; // ImageBox dimension
    uint16_t *img_rgb565;
    uint16_t img_w, img_h; // dimention of image stored
	uint16_t src_w, src_h; // dimention of source image (JPEG/PNG)
    bool resizeFit; // true: resize to fit ImageBox size, false: original size (1:1)
    bool keepAspectRatio; // keep Aspect Ratio when resizeFit == true
    bool packHBlank; // pack (delete) Horizontal Blank if width > img_w
    ImageFitter();
    virtual ~ImageFitter();
    ImageFitter(const ImageFitter&) = delete;
	ImageFitter& operator=(const ImageFitter&) = delete;
    void jpegMcu2sAccum(int count, uint16_t mcu_w, uint16_t mcu_h, uint16_t *pImage);
    void loadJpeg(bool reduce);
public:
    static ImageFitter& instance(); // Singleton
    void config(uint16_t *img_rgb565, uint16_t width, uint16_t height, bool resizeFit = true, bool keepAspectRatio = true, bool packHBlank = false);
    bool loadJpegFile(const char *filename, const uint64_t pos = 0, const size_t size = 0);
    void getSizeAfterFit(uint16_t *img_w, uint16_t *img_h);
};

#endif // _IMAGEFITTER_H_DEFINED_

