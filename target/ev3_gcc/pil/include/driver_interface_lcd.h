/**
 * Interface for LCD driver
 */

#pragma once

#include <kernel.h>
#include <t_syslog.h>

/**
 * Bitmap structure. The pixels are packed in the format of ST7586's DDRAM.
 */
typedef struct {
	int32_t width;
	int32_t height;
	void*   pixels;
} bitmap_t;

#define BITMAP_LINE_LENGTH(width) (((width)+2)/3)
#define BITMAP_SUBPX_SHIFT(x) ((2-(x))*3)
#define BITMAP_SUBPX_MASK(x)  (0x3 << (BITMAP_SUBPX_SHIFT(x)))
#define BITMAP_PIXELS_SIZE(w,h) (BITMAP_LINE_LENGTH(w)*h)

static inline bool_t bitmap_set_pixel(bitmap_t *bitmap, int32_t x, int32_t y, bool_t color) {
	if (x < 0 || x >= bitmap->width) return false;
	if (y < 0 || y >= bitmap->height) return false;

	uint8_t *pixel = bitmap->pixels + BITMAP_LINE_LENGTH(bitmap->width) * y + x / 3 /* x/3 => byte offset */;
	if (color)
		*pixel |= BITMAP_SUBPX_MASK(x % 3);
	else
		*pixel &= ~BITMAP_SUBPX_MASK(x % 3);
	return true;
}

static inline bool_t bitmap_get_pixel(const bitmap_t *bitmap, int32_t x, int32_t y) {
	if (x < 0 || x >= bitmap->width) return false;
	if (y < 0 || y >= bitmap->height) return false;
	uint8_t *pixel = bitmap->pixels + BITMAP_LINE_LENGTH(bitmap->width) * y + x / 3 /* x/3 => byte offset */;
	uint8_t mask = BITMAP_SUBPX_MASK(x % 3);
	return (*pixel & mask) == mask;
}

/**
 * bitBlt
 */

/**
 * Common raster operations for use with bitBlt (from LeJOS)
 */
#define ROP_CLEAR 0x00000000
#define ROP_AND 0xff000000
#define ROP_ANDREVERSE 0xff00ff00
#define ROP_COPY 0x0000ff00
#define ROP_ANDINVERTED 0xffff0000
#define ROP_NOOP 0x00ff0000
#define ROP_XOR 0x00ffff00
#define ROP_OR 0xffffff00
#define ROP_NOR 0xffffffff
#define ROP_EQUIV 0x00ffffff
#define ROP_INVERT 0x00ff00ff
#define ROP_ORREVERSE 0xffff00ff
#define ROP_COPYINVERTED 0x0000ffff
#define ROP_ORINVERTED 0xff00ffff
#define ROP_NAND 0xff0000ff
#define ROP_SET 0x000000ff

/**
 * Modified from LeJOS
 */

static void bitmap_bitblt(const bitmap_t *src, int sx, int sy, bitmap_t *dst, int dx, int dy, int w, int h, int rop) {
#if 1 // A (very?) slow version for testing

	for (int rel_x = 0; rel_x < w; rel_x++) {
	    for (int rel_y = 0; rel_y < h; rel_y++) {
	        switch(rop) {
	        case ROP_CLEAR:
	            bitmap_set_pixel(dst, dx + rel_x, dy + rel_y, false);
	            break;
	        case ROP_SET:
	            bitmap_set_pixel(dst, dx + rel_x, dy + rel_y, true);
	            break;
	        case ROP_COPY:
	            bitmap_set_pixel(dst, dx + rel_x, dy + rel_y,
	                    bitmap_get_pixel(src, sx + rel_x, sy + rel_y));
	            break;
	        case ROP_COPYINVERTED:
	            bitmap_set_pixel(dst, dx + rel_x, dy + rel_y,
	                    !bitmap_get_pixel(src, sx + rel_x, sy + rel_y));
	        	break;
	        default:
	            syslog(LOG_ERROR, "%s(): ROP code 0x%08x not supported.", __FUNCTION__, rop);
	            return;
	        }
	    }
	}
#if 0
	int sw = src ? src->width : 0;
    int sh = src ? src->height : 0;
    int dw = dst ? dst->width : 0;
    int dh = dst ? dst->height : 0;

	int iy = sy;
	int oy = dy;
	for (; h > 0; iy++, oy++, h--) {
		int ix = sx, ox = dx;
		for (int i = 0; i < w; ix++, ox++, i++) {
		switch(rop) {
		case ROP_CLEAR:
			bitmap_set_pixel(lcd_screen, ox, oy, false);
			break;
		case ROP_SET:
			bitmap_set_pixel(lcd_screen, ox, oy, true);
			break;
		default:
			syslog(LOG_ERROR, "%s(): ROP code 0x%08x not supported.", __FUNCTION__, rop);
			return;
		}
		}
	}
#endif
#else // LeJOS version
	int sw = src->width;
	int sh = src->height;
	int dw = dst->width;
	int dh = dst->height;
	/* This is a partial implementation of the BitBlt algorithm. It provides a
	 * complete set of raster operations and handles partial and fully aligned
	 * images correctly. Overlapping source and destination images is also
	 * supported. It does not performing mirroring. The code was converted
	 * from an initial Java implementation and has not been optimized for C.
	 * The general mechanism is to perform the block copy with Y as the inner
	 * loop (because on the display the bits are packed y-wise into a byte). We
	 * perform the various rop cases by reducing the operation to a series of
	 * AND and XOR operations. Each step is controlled by a byte in the rop code.
	 * This mechanism is based upon that used in the X Windows system server.
	 */
	// Clip to source and destination
	int trim;
	if (dx < 0) {
		trim = -dx;
		dx = 0;
		sx += trim;
		w -= trim;
	}
	if (dy < 0) {
		trim = -dy;
		dy = 0;
		sy += trim;
		h -= trim;
	}
	if (sx < 0 || sy < 0)
		return;
	if (dx + w > dw)
		w = dw - dx;
	if (sx + w > sw)
		w = sw - sx;
	if (w <= 0)
		return;
	if (dy + h > dh)
		h = dh - dy;
	if (sy + h > sh)
		h = sh - sy;
	if (h <= 0)
		return;
	// Setup initial parameters and check for overlapping copy
	int xinc = 1;
	int yinc = 1;
	byte firstBit = 1;
	if (src == dst) {
		// If copy overlaps we use reverse direction
		if (dy > sy) {
			sy = sy + h - 1;
			dy = dy + h - 1;
			yinc = -1;
		}
		if (dx > sx) {
			firstBit = (byte) 0x80;
			xinc = -1;
			sx = sx + w - 1;
			dx = dx + w - 1;
		}
	}
	if (src == null)
		src = dst;
	int swb = (sw + 7) / 8;
	int dwb = (dw + 7) / 8;
	//if (src == displayBuf)
	//swb = HW_MEM_WIDTH;
	//if (dst == displayBuf)
	//dwb = HW_MEM_WIDTH;
	int inStart = sy * swb;
	int outStart = dy * dwb;
	byte inStartBit = (byte)(1 << (sx & 0x7));
	byte outStartBit = (byte)(1 << (dx & 0x7));
	dwb *= yinc;
	swb *= yinc;
	// Extract rop sub-fields
	byte ca1 = (byte)(rop >> 24);
	byte cx1 = (byte)(rop >> 16);
	byte ca2 = (byte)(rop >> 8);
	byte cx2 = (byte) rop;
	boolean noDst = (ca1 == 0 && cx1 == 0);
	int ycnt;

	// Check for byte aligned case and optimise for it
	if (w >= 8 && inStartBit == firstBit && outStartBit == firstBit) {
		int ix = sx / 8;
		int ox = dx / 8;
		int byteCnt = w / 8;
		ycnt = h;
		while (ycnt-- > 0) {
			int inIndex = inStart + ix;
			int outIndex = outStart + ox;
			int cnt = byteCnt;
			while (cnt-- > 0) {
				if (noDst)
					dst[outIndex] = (byte)((src[inIndex] & ca2) ^ cx2);
				else {
					byte inVal = src[inIndex];
					dst[outIndex] = (byte)(
							(dst[outIndex] & ((inVal & ca1) ^ cx1))
									^ ((inVal & ca2) ^ cx2));
				}
				outIndex += xinc;
				inIndex += xinc;
			}
			ix += swb;
			ox += dwb;
		}
		// Do we have a final non byte multiple to do?
		w &= 0x7;
		if (w == 0) {
			//if (dst == displayBuf)
			//update(displayBuf);
			return;
		}
		//inStart = sy*swb;
		//outStart = dy*dwb;
		sx += byteCnt * 8;
		dx += byteCnt * 8;
	}

	// General non byte aligned case
	int ix = sx / 8;
	int ox = dx / 8;
	ycnt = h;
	while (ycnt-- > 0) {
		int inIndex = inStart + ix;
		byte inBit = inStartBit;
		byte inVal = src[inIndex];
		byte inAnd = (byte)((inVal & ca1) ^ cx1);
		byte inXor = (byte)((inVal & ca2) ^ cx2);
		int outIndex = outStart + ox;
		byte outBit = outStartBit;
		byte outPixels = dst[outIndex];
		int cnt = w;
		while (true) {
			if (noDst) {
				if ((inXor & inBit) != 0)
					outPixels |= outBit;
				else
					outPixels &= ~outBit;
			} else {
				byte resBit = (byte)(
						(outPixels & ((inAnd & inBit) != 0 ? outBit : 0))
								^ ((inXor & inBit) != 0 ? outBit : 0));
				outPixels = (byte)((outPixels & ~outBit) | resBit);
			}
			if (--cnt <= 0)
				break;
			if (xinc > 0) {
				inBit <<= 1;
				outBit <<= 1;
			} else {
				inBit >>= 1;
				outBit >>= 1;
			}
			if (inBit == 0) {
				inBit = firstBit;
				inIndex += xinc;
				inVal = src[inIndex];
				inAnd = (byte)((inVal & ca1) ^ cx1);
				inXor = (byte)((inVal & ca2) ^ cx2);
			}
			if (outBit == 0) {
				dst[outIndex] = outPixels;
				outBit = firstBit;
				outIndex += xinc;
				outPixels = dst[outIndex];
			}
		}
		dst[outIndex] = outPixels;
		inStart += swb;
		outStart += dwb;
	}
	//if (dst == displayBuf)
	//update(displayBuf);
#endif
}

/**
 * Font structure.
 */
typedef struct {
    // Font information
    uint32_t height;
    uint32_t width; // 0 if not monospace
    // Glyph array for fast seeking (e.g. ASCII printable characters)
    uint32_t  first_code_point; // UTF-8 code point of the first char in the array
    bitmap_t *array;
    uint32_t  array_sz;
    // Glyph dictionary for more characters (e.g. CJK), not supported yet
    void     *dict;
    uint32_t  dict_sz;
} font_t;

static inline bitmap_t *utf8_char_bitmap(uint32_t codepoint, font_t *font) {
    if (codepoint >= font->first_code_point && codepoint < font->first_code_point + font->array_sz)
        return &font->array[codepoint - font->first_code_point];
    else { // Search the dictionary
//        assert(false/*not support yet*/);
        return NULL;
    }
}

static inline
void bitmap_draw_string(const char *str, bitmap_t *dest, int x, int y, font_t *font, int rop) {
    bitmap_t *question = utf8_char_bitmap('?', font); // For default
    assert(question != NULL);

    while(*str != '\0') {
        uint32_t codepoint;
        if (!(*str & 0x80)) {
            codepoint = *str;
            str++;
        } else { // High UTF-8 code point, not support yet
            assert(false);
            codepoint = *str;
            str++;
        }

        bitmap_t *bitmap = utf8_char_bitmap(codepoint, font);
        if (bitmap == NULL) bitmap = question; // Fall-back

        bitmap_bitblt(bitmap, 0, 0, dest, x, y, bitmap->width, bitmap->height, rop);
        x += bitmap->width;
    }
}

/**
 * Support for BMP file
 */

#define CHECK_COND(exp, _ercd) do {                         \
    if (!(exp)) {                                           \
        ercd = _ercd;                                       \
        goto error_exit;                                    \
    }                                                       \
} while (false)

typedef struct tagBITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
  uint32_t bfInfoHeaderSize; // Added for convenience
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	uint32_t  biSize;
	int32_t   biWidth;
	int32_t   biHeight;
	uint16_t  biPlanes;
	uint16_t  biBitCount;
    uint32_t  biCompression;
    uint32_t  biSizeImage;
    int32_t   biXPixPerMeter;
    int32_t   biYPixPerMeter;
    uint32_t  biClrUsed;
    uint32_t  biClrImporant;
} __attribute__((packed)) BITMAPINFOHEADER;


/**
 * E_NOSPT: format not supported
 * E_OBJ: corrupt / wrong BMP file data.
 */
static inline
ER bmpfile_read_header(const void *bmpfile, SIZE bmpfilesz, int32_t *p_width, int32_t *p_height) {
	ER ercd;

	// Check header
	CHECK_COND(bmpfilesz >= sizeof(BITMAPFILEHEADER), E_OBJ);
//	syslog(LOG_NOTICE, "HERE0 sizeof(BITMAPFILEHEADER) %d", sizeof(BITMAPFILEHEADER));
	const BITMAPFILEHEADER *filehdr = bmpfile;
#if 0
	for (int i = 0; i < bmpfilesz; i+=4) { // Dump bmpfile
		syslog(LOG_NOTICE, "%02x %02x %02x %02x", ((uint8_t*)bmpfile)[i], ((uint8_t*)bmpfile)[i+1], ((uint8_t*)bmpfile)[i+2], ((uint8_t*)bmpfile)[i+3]);
	}
#endif
//	syslog(LOG_NOTICE, "filehdr->bfType 0x%08x", filehdr->bfType);
//	syslog(LOG_NOTICE, "filehdr->bfSize 0x%08x", filehdr->bfSize);
//	syslog(LOG_NOTICE, "filehdr->bfInfoHeaderSize %d", filehdr->bfInfoHeaderSize);
	CHECK_COND(filehdr->bfInfoHeaderSize == sizeof(BITMAPINFOHEADER), E_NOSPT);
//	syslog(LOG_NOTICE, "HERE1");
	CHECK_COND(bmpfilesz >= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) - 4/*sizeof(bfInfoHeaderSize)*/, E_OBJ);
//	syslog(LOG_NOTICE, "HERE2");
	BITMAPINFOHEADER *infohdr = (BITMAPINFOHEADER*)&(filehdr->bfInfoHeaderSize);
	CHECK_COND(infohdr->biWidth >= 0 && infohdr->biHeight >= 0, E_NOSPT);
//	syslog(LOG_NOTICE, "HERE3 infohdr->biBitCount %d", infohdr->biBitCount);
	CHECK_COND(infohdr->biBitCount == 1, E_NOSPT); // Monochrome
//	syslog(LOG_NOTICE, "HERE4");
	CHECK_COND(infohdr->biCompression == 0, E_NOSPT); // No compression
//	syslog(LOG_NOTICE, "HERE5");
	*p_width = infohdr->biWidth;
	*p_height = infohdr->biHeight;
	int data_size = ((*p_width + 31) / 32) * 4 * *p_height; // Hard coded
	CHECK_COND(filehdr->bfOffBits + data_size <= bmpfilesz, E_OBJ);
//	syslog(LOG_NOTICE, "HERE6");

	ercd = E_OK;

error_exit:
	assert(ercd == E_OK);
	return ercd;
}


/**
 * E_PAR: bitmap & bmpfile have different size
 * E_NOSPT: format not supported
 * E_OBJ: corrupt / wrong BMP file data.
 */
static inline
ER bmpfile_to_bitmap(const void *bmpfile, SIZE bmpfilesz, bitmap_t *bitmap) {
	ER ercd;

	int32_t width, height;

	ercd = bmpfile_read_header(bmpfile, bmpfilesz, &width, &height);
	if (ercd != E_OK) goto error_exit;

	CHECK_COND(bitmap->width == width && bitmap->height >= height, E_PAR);

	const BITMAPFILEHEADER *filehdr = bmpfile;
	const uint8_t *data = bmpfile + filehdr->bfOffBits;
	int linewidth = ((width + 31) / 32) * 4;
	for (int y = 0; y < bitmap->height; ++y) {
		const uint8_t *line = data + (bitmap->height - y - 1) * linewidth;
		uint8_t databyte = 0;
		for (int x = 0; x < bitmap->width; ++x) {
			if (x % 8 == 0) databyte = line[x / 8]; // Update databyte
			bitmap_set_pixel(bitmap, x, y, (databyte & 0x80) == 0 /* todo: check 0x80 highest bit => leftmost pixel */);
			databyte <<= 1;
		}
	}

	ercd = E_OK;

error_exit:
	assert(ercd == E_OK);
	return ercd;
}
