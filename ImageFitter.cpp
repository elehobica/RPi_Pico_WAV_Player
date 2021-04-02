/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstring>
#include "picojpeg/JPEGDecoder.h"
#include "ImageFitter.h"

//=====================================
// Implementation of ImageFitter class
//=====================================
ImageFitter& ImageFitter::instance()
{
    static ImageFitter _instance; // Singleton
    return _instance;
}

ImageFitter::ImageFitter()
{
}

ImageFitter::~ImageFitter()
{

}

void ImageFitter::config(uint16_t *img_rgb565, uint16_t width, uint16_t height, bool resizeFit, bool keepAspectRatio, bool packHBlank)
{
    this->img_rgb565 = img_rgb565;
    this->width = width;
    this->height = height;
    this->resizeFit = resizeFit;
    this->keepAspectRatio = keepAspectRatio;
    this->packHBlank = packHBlank;
}

// MCU block 1/2 Accumulation (Shrink) x count times
void ImageFitter::jpegMcu2sAccum(int count, uint16_t mcu_w, uint16_t mcu_h, uint16_t *pImage)
{
    int i;
    mcu_w <<= count;
    mcu_h <<= count;
    for (i = 0; i < count; i++) {
        for (int16_t mcu_ofs_y = 0; mcu_ofs_y < mcu_h; mcu_ofs_y+=2) {
            for (int16_t mcu_ofs_x = 0; mcu_ofs_x < mcu_w; mcu_ofs_x+=2) {
                uint32_t r = 0;
                uint32_t g = 0;
                uint32_t b = 0;
                for (int y = 0; y < 2; y++) {
                    for (int x = 0; x < 2; x++) {
                        // RGB565 format
                        r += pImage[mcu_w*(mcu_ofs_y+y)+mcu_ofs_x+x] & 0xf800;
                        g += pImage[mcu_w*(mcu_ofs_y+y)+mcu_ofs_x+x] & 0x07e0;
                        b += pImage[mcu_w*(mcu_ofs_y+y)+mcu_ofs_x+x] & 0x001f;
                    }
                }
                r = (r / 4) & 0xf800;
                g = (g / 4) & 0x07e0;
                b = (b / 4) & 0x001f;
                pImage[mcu_w/2*(mcu_ofs_y/2)+mcu_ofs_x/2] = ((uint16_t) r | (uint16_t) g | (uint16_t) b);
            }
        }
        mcu_w /= 2;
        mcu_h /= 2;
    }
}

void ImageFitter::loadJpeg(bool reduce)
{
    if (img_rgb565 == NULL) { return; }
    src_w = JpegDec.width;
    src_h = JpegDec.height;
    uint16_t mcu_w = JpegDec.MCUWidth;
    uint16_t mcu_h = JpegDec.MCUHeight;
    #ifdef DEBUG_LCD_ELEMENT_BOX
    { // DEBUG
        char str[256];
        sprintf(str, "JPEG info: (w, h) = (%d, %d), (mcu_w, mcu_h) = (%d, %d)", src_w, src_h, mcu_w, mcu_h);
        Serial.println(str);
    }
    #endif // DEBUG_LCD_ELEMENT_BOX
   if (reduce) {
        src_w /= 8;
        src_h /= 8;
        mcu_w /= 8;
        mcu_h /= 8;
        #ifdef DEBUG_LCD_ELEMENT_BOX
        { // DEBUG
            char str[256];
            sprintf(str, "Reduce applied:  (w, h) = (%d, %d), (virtual) (mcu_w, mcu_h) = (%d, %d)", src_w, src_h, mcu_w, mcu_h);
            Serial.println(str);
        }
        #endif // DEBUG_LCD_ELEMENT_BOX
   }
    // Calculate MCU 2's Accumulation Count
    int mcu_2s_accum_cnt = 0;
    {
        while (1) {
            if (!resizeFit) break;
            if (keepAspectRatio) {
                if (src_w <= width * 2 && src_h <= height * 2) break;
            } else {
                if (src_w <= width * 2 || src_h <= height * 2) break;
            }
            if (mcu_w == 1 || mcu_h == 1) break;
            src_w /= 2;
            src_h /= 2;
            mcu_w /= 2;
            mcu_h /= 2;
            mcu_2s_accum_cnt++;
        }
        #ifdef DEBUG_LCD_ELEMENT_BOX
        { // DEBUG
            if (mcu_2s_accum_cnt > 0) {
                char str[256];
                sprintf(str, "Accumulated %d times: (w, h) = (%d, %d), (mcu_w, mcu_h) = (%d, %d)", mcu_2s_accum_cnt, src_w, src_h, mcu_w, mcu_h);
                Serial.println(str);
            }
        }
        #endif // DEBUG_LCD_ELEMENT_BOX
    }

    int16_t mcu_y_prev = 0;
    int16_t mod_y_start = 0;
    int16_t plot_y_start = 0;
    while (JpegDec.read()) {
        // MCU 2's Accumulation
        jpegMcu2sAccum(mcu_2s_accum_cnt, mcu_w, mcu_h, JpegDec.pImage);
        int idx = 0;
        int16_t x, y;
        int16_t mcu_x = JpegDec.MCUx;
        int16_t mcu_y = JpegDec.MCUy;
        int16_t mod_y_pls = (!keepAspectRatio || src_h*width >= src_w*height) ? src_h : src_w;
        int16_t mod_y = mod_y_pls;
        int16_t plot_y = 0;
        // prepare plot_y (, mod_y) condition
        if (!resizeFit) {
            plot_y = mcu_h * mcu_y;
            if (plot_y >= height) continue; // don't use break because MCU order is not always left-to-right and top-to-bottom
        } else {
            if (mcu_y != mcu_y_prev) {
                for (y = 0; y < mcu_h * mcu_y; y++) {
                    while (mod_y < 0) {
                        mod_y += mod_y_pls;
                        plot_y++;
                    }
                    mod_y -= height;
                }
                // memorize plot_y (, mod_y) start condition
                mcu_y_prev = mcu_y;
                mod_y_start = mod_y;
                plot_y_start = plot_y;
            } else {
                // reuse plot_y (, mod_y) start condition
                mod_y = mod_y_start;
                plot_y = plot_y_start;
            }
        }
        int16_t mod_x_start = 0;
        int16_t plot_x_start = 0;
        for (int16_t mcu_ofs_y = 0; mcu_ofs_y < mcu_h; mcu_ofs_y++) {
            y = mcu_h * mcu_y + mcu_ofs_y;
            if (y >= src_h) break;
            int16_t mod_x_pls = (!keepAspectRatio || src_w >= src_h) ? src_w : src_h;
            int16_t mod_x = mod_x_pls;
            int16_t plot_x = 0;
            // prepare plot_x (, mod_x) condition
            if (!resizeFit) {
                plot_x = mcu_w * mcu_x;
                if (plot_x >= width) break;
            } else {
                if (mcu_ofs_y == 0) {
                    for (x = 0; x < mcu_w * mcu_x; x++) {
                        while (mod_x < 0) {
                            mod_x += mod_x_pls;
                            plot_x++;
                        }
                        mod_x -= width;
                    }
                    // memorize plot_x (, mod_x) start condition
                    mod_x_start = mod_x;
                    plot_x_start = plot_x;
                } else {
                    // reuse plot_x (, mod_x) start condition
                    mod_x = mod_x_start;
                    plot_x = plot_x_start;
                }
            }
            // actual plot_x
            for (int16_t mcu_ofs_x = 0; mcu_ofs_x < mcu_w; mcu_ofs_x++) {
                x = mcu_w * mcu_x + mcu_ofs_x;
                if (x >= src_w) {
                    idx += mcu_w - src_w%mcu_w; // skip horizontal padding area
                    break;
                }
                if (!resizeFit) {
                    if (plot_x >= width) break;
                    img_rgb565[width*plot_y+plot_x] = JpegDec.pImage[idx];
                    if (plot_x+1 > img_w) { img_w = plot_x+1; }
                    plot_x++;
                } else {
                    if (mod_y < 0) {
                        while (mod_x < 0) {
                            if (plot_x >= width) break;
                            img_rgb565[width*plot_y+plot_x] = JpegDec.pImage[idx];
                            if (plot_x+1 > img_w) { img_w = plot_x+1; }
                            mod_x += mod_x_pls;
                            plot_x++;
                        }
                        mod_x -= width;
                    }
                }
                idx++;
            }
            if (!resizeFit) {
                if (plot_y+1 > img_h) { img_h = plot_y+1; }
                plot_y++;
                if (plot_y >= height) break;
            } else {
                while (mod_y < 0) { // repeat previous line in case of expanding
                    if (plot_y+1 > img_h) { img_h = plot_y+1; }
                    mod_y += mod_y_pls;
                    plot_y++;
                    if (plot_y >= height) break;
                    if (mod_y < 0) {
                        for (x = plot_x_start; x < plot_x; x++) {
                            img_rgb565[width*plot_y+x] = img_rgb565[width*(plot_y-1)+x];
                        }
                    }
                }
                mod_y -= height;
            }
        }
    }
    #ifdef DEBUG_LCD_ELEMENT_BOX
    { // DEBUG
        char str[256];
        sprintf(str, "Resized to (img_w, img_h) = (%d, %d)", img_w, img_h);
        Serial.println(str);
    }
    #endif // DEBUG_LCD_ELEMENT_BOX
    if (packHBlank && img_w < width) { // delete horizontal blank
        for (int16_t plot_y = 1; plot_y < img_h; plot_y++) {
            memmove(&img_rgb565[img_w*plot_y], &img_rgb565[width*plot_y], img_w*2);
        }
    }
}

// load from JPEG File
bool ImageFitter::loadJpegFile(const char *filename, uint64_t pos, size_t size)
{
    int decoded;
    bool reduce = false;
    decoded = JpegDec.decodeSdFile(filename, pos, size, 0); // reduce == 0
    if (decoded <= 0) { return false; }
    src_w = JpegDec.width;
    src_h = JpegDec.height;
    if (src_w == width && src_h == height) { resizeFit = false; }
    if (resizeFit && (
        (keepAspectRatio && (src_w >= width*8 && src_h >= height*8)) ||
        (!keepAspectRatio && (src_w >= width*8 || src_h >= height*8))
    )) { // Use reduce decode for x8 larger image
        reduce = true;
        JpegDec.abort();
        decoded = JpegDec.decodeSdFile(filename, pos, size, 1); // reduce == 1
        if (decoded <= 0) { return false; }
    }
    loadJpeg(reduce);
    JpegDec.abort();
    return true;
}

void ImageFitter::getSizeAfterFit(uint16_t *img_w, uint16_t *img_h)
{
    *img_w = this->img_w;
    *img_h = this->img_h;
}