/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
  Copyright (c) 2019 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _VGACONTROLLER_H_INCLUDED
#define _VGACONTROLLER_H_INCLUDED


/**
 * @file
 *
 * @brief This file contains fabgl::VGAControllerClass definition and the VGAController instance.
 */


#include <stdint.h>
#include <stddef.h>
#include <atomic>

#include "rom/lldesc.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "fabglconf.h"
#include "fabutils.h"




namespace fabgl {





/** @brief Represents one of the four blocks of horizontal or vertical line */
enum ScreenBlock {
  FrontPorch,   /**< Horizontal line sequence is: FRONTPORCH -> SYNC -> BACKPORCH -> VISIBLEAREA */
  Sync,         /**< Horizontal line sequence is: SYNC -> BACKPORCH -> VISIBLEAREA -> FRONTPORCH */
  BackPorch,    /**< Horizontal line sequence is: BACKPORCH -> VISIBLEAREA -> FRONTPORCH -> SYNC */
  VisibleArea   /**< Horizontal line sequence is: VISIBLEAREA -> FRONTPORCH -> SYNC -> BACKPORCH */
};


/** @brief Specifies the VGA timings. This is a modeline decoded. */
struct Timings {
  char          label[22];       /**< Resolution text description */
  int           frequency;       /**< Pixel frequency (in Hz) */
  int16_t       HVisibleArea;    /**< Horizontal visible area length in pixels */
  int16_t       HFrontPorch;     /**< Horizontal Front Porch duration in pixels */
  int16_t       HSyncPulse;      /**< Horizontal Sync Pulse duration in pixels */
  int16_t       HBackPorch;      /**< Horizontal Back Porch duration in pixels */
  int16_t       VVisibleArea;    /**< Vertical number of visible lines */
  int16_t       VFrontPorch;     /**< Vertical Front Porch duration in lines */
  int16_t       VSyncPulse;      /**< Vertical Sync Pulse duration in lines */
  int16_t       VBackPorch;      /**< Vertical Back Porch duration in lines */
  char          HSyncLogic;      /**< Horizontal Sync polarity '+' or '-' */
  char          VSyncLogic;      /**< Vertical Sync polarity '+' or '-' */
  uint8_t       scanCount;       /**< Scan count. 1 = single scan, 2 = double scan (allowing low resolutions like 320x240...) */
  uint8_t       multiScanBlack;  /**< 0 = Additional rows are the repetition of the first. 1 = Additional rows are blank. */
  ScreenBlock   HStartingBlock;  /**< Horizontal starting block. DetermineshHorizontal order of signals */
};



/*
  Notes:
    - all positions can have negative and outofbound coordinates. Shapes are always clipped correctly.
*/
enum PrimitiveCmd {
  // Set current pen color
  // params: color
  SetPenColor,

  // Set current brush color
  // params: color
  SetBrushColor,

  // Paint a pixel at specified coordinates, using current pen color
  // params: color
  SetPixel,

  // Move current position to the specified one
  // params: point
  MoveTo,

  // Draw a line from current position to the specified one, using current pen color. Update current position.
  // params: point
  LineTo,

  // Fill a rectangle using current brush color
  // params: rect
  FillRect,

  // Fill an ellipse, current position is the center, using current brush color
  // params: size
  FillEllipse,

  // Draw an ellipse, current position is the center, using current pen color
  // params: size
  DrawEllipse,

  // Fill viewport with brush color
  // params: none
  Clear,

  // Scroll vertically without copying buffers
  // params: ivalue (scroll amount, can be negative)
  VScroll,

  // Scroll horizontally (time consuming operation!)
  // params: ivalue (scroll amount, can be negative)
  HScroll,

  // Draw a glyph (BW image)
  // params: glyph
  DrawGlyph,

  // Set paint options
  // params: glyphOptions
  SetGlyphOptions,

  // Set gluph options
  // params: paintOptions
  SetPaintOptions,

#if FABGLIB_HAS_INVERTRECT
  // Invert a rectangle
  // params: rect
  InvertRect,
#endif

  // Copy (overlapping) rectangle to current position
  // params: rect (source rectangle)
  CopyRect,

  // Set scrolling region
  // params: rect
  SetScrollingRegion,

  // Swap foreground (pen) and background (brush) colors of all pixels inside the specified rectangles. Other colors remain untaltered.
  // params: rect
  SwapFGBG,

#if FABGLIB_HAS_READWRITE_RAW_DATA
  // Read raw viewport data
  // params: rawData
  ReadRawData,

  // Write raw viewport data
  // params: rawData
  WriteRawData,
#endif

  // Render glyphs buffer
  // params: glyphsBufferRenderInfo
  RenderGlyphsBuffer,

  // Draw a bitmap
  // params: bitmapDrawingInfo
  DrawBitmap,

  // Refresh sprites
  // no params
  RefreshSprites,

  // Swap buffers (m_doubleBuffered must be True)
  SwapBuffers,

  // Fill a path, using current brush color
  // params: path
  FillPath,

  // Draw a path, using current pen color
  // params: path
  DrawPath,

  // Set axis origin
  // params: point
  SetOrigin,

  // Set clipping rectangle
  // params: rect
  SetClippingRect,
};



/**
 * @brief This enum defines named colors.
 *
 * First eight full implement all available colors when 1 bit per channel mode is used (having 8 colors).
 */
enum Color {
  Black,          /**< Equivalent to R = 0, G = 0, B = 0 */
  Red,            /**< Equivalent to R = 1, G = 0, B = 0 */
  Green,          /**< Equivalent to R = 0, G = 1, B = 0 */
  Yellow,         /**< Equivalent to R = 1, G = 1, B = 0 */
  Blue,           /**< Equivalent to R = 0, G = 0, B = 1 */
  Magenta,        /**< Equivalent to R = 1, G = 0, B = 1 */
  Cyan,           /**< Equivalent to R = 0, G = 1, B = 1 */
  White,          /**< Equivalent to R = 1, G = 1, B = 1 */
  BrightBlack,    /**< Equivalent to R = 1, G = 1, B = 1 */
  BrightRed,      /**< Equivalent to R = 3, G = 0, B = 0 */
  BrightGreen,    /**< Equivalent to R = 0, G = 3, B = 0 */
  BrightYellow,   /**< Equivalent to R = 3, G = 3, B = 0 */
  BrightBlue,     /**< Equivalent to R = 0, G = 0, B = 3 */
  BrightMagenta,  /**< Equivalent to R = 3, G = 0, B = 3 */
  BrightCyan,     /**< Equivalent to R = 0, G = 3, B = 3 */
  BrightWhite,    /**< Equivalent to R = 3, G = 3, B = 3 */
};



/**
 * @brief Represents an RGB color.
 *
 * When 1 bit per channel (8 colors) is used then the maximum value (white) is 1 (R = 1, G = 1, B = 1).
 * When 2 bits per channel (64 colors) are used then the maximum value (white) is 3 (R = 3, G = 3, B = 3).
 */
struct RGB {
  uint8_t R : 2;  /**< The Red channel */
  uint8_t G : 2;  /**< The Green channel */
  uint8_t B : 2;  /**< The Blue channel */

  RGB() : R(0), G(0), B(0) { }
  RGB(Color color);
  RGB(uint8_t red, uint8_t green, uint8_t blue) : R(red), G(green), B(blue) { }
};


inline bool operator==(RGB const& lhs, RGB const& rhs)
{
  return lhs.R == rhs.R && lhs.G == rhs.G && lhs.B == rhs.B;
}


inline bool operator!=(RGB const& lhs, RGB const& rhs)
{
  return lhs.R != rhs.R || lhs.G != rhs.G || lhs.B == rhs.B;
}



/**
 * @brief Represents a glyph position, size and binary data.
 *
 * A glyph is a bitmap (1 bit per pixel). The fabgl::TerminalClass uses glyphs to render characters.
 */
struct Glyph {
  int16_t         X;      /**< Horizontal glyph coordinate */
  int16_t         Y;      /**< Vertical glyph coordinate */
  int16_t         width;  /**< Glyph horizontal size */
  int16_t         height; /**< Glyph vertical size */
  uint8_t const * data;   /**< Byte aligned binary data of the glyph. A 0 represents background or a transparent pixel. A 1 represents foreground. */

  Glyph() : X(0), Y(0), width(0), height(0), data(NULL) { }
  Glyph(int X_, int Y_, int width_, int height_, uint8_t const * data_) : X(X_), Y(Y_), width(width_), height(height_), data(data_) { }
};



/**
 * @brief Represents a region of raw screen buffer.
 */
struct RawData {
  int16_t   X;      /**< Horizontal region coordinate */
  int16_t   Y;      /**< Vertical region coordinate */
  int16_t   width;   /**< Horizontal region size */
  int16_t   height;  /**< Vertical region size */
  uint8_t * data;   /**< Raw region data */

  RawData(int X_, int Y_, int width_, int height_, uint8_t * data_) : X(X_), Y(Y_), width(width_), height(height_), data(data_) { }
};



/**
 * @brief Specifies various glyph painting options.
 */
union GlyphOptions {
  struct {
    uint16_t fillBackground   : 1;  /**< If enabled glyph background is filled with current background color. */
    uint16_t bold             : 1;  /**< If enabled produces a bold-like style. */
    uint16_t reduceLuminosity : 1;  /**< If enabled reduces luminosity. To implement characters faint. */
    uint16_t italic           : 1;  /**< If enabled skews the glyph on the right. To implement characters italic. */
    uint16_t invert           : 1;  /**< If enabled swaps foreground and background colors. To implement characters inverse (XORed with PaintState.paintOptions.swapFGBG) */
    uint16_t blank            : 1;  /**< If enabled the glyph is filled with the background color. To implement characters invisible or blink. */
    uint16_t underline        : 1;  /**< If enabled the glyph is underlined. To implement characters underline. */
    uint16_t doubleWidth      : 2;  /**< If enabled the glyph is doubled. To implement characters double width. 0 = normal, 1 = double width, 2 = double width - double height top, 3 = double width - double height bottom. */
    uint16_t userOpt1         : 1;  /**< User defined option */
    uint16_t userOpt2         : 1;  /**< User defined option */
  };
  uint16_t value;

  /** @brief Helper method to set or reset fillBackground */
  GlyphOptions & FillBackground(bool value) { fillBackground = value; return *this; }

  /** @brief Helper method to set or reset bold */
  GlyphOptions & Bold(bool value) { bold = value; return *this; }

  /** @brief Helper method to set or reset italic */
  GlyphOptions & Italic(bool value) { italic = value; return *this; }

  /** @brief Helper method to set or reset underlined */
  GlyphOptions & Underline(bool value) { underline = value; return *this; }

  /** @brief Helper method to set or reset doubleWidth */
  GlyphOptions & DoubleWidth(uint8_t value) { doubleWidth = value; return *this; }

  /** @brief Helper method to set or reset foreground and background swapping */
  GlyphOptions & Invert(uint8_t value) { invert = value; return *this; }
};



// GlyphsBuffer.map support functions
//  0 ..  7 : index
//  8 .. 11 : BG color (Color)
// 12 .. 15 : FG color (Color)
// 16 .. 31 : options (GlyphOptions)
// note: volatile pointer to avoid optimizer to get less than 32 bit from 32 bit access only memory
#define GLYPHMAP_INDEX_BIT    0
#define GLYPHMAP_BGCOLOR_BIT  8
#define GLYPHMAP_FGCOLOR_BIT 12
#define GLYPHMAP_OPTIONS_BIT 16
#define GLYPHMAP_ITEM_MAKE(index, bgColor, fgColor, options) (((uint32_t)(index) << GLYPHMAP_INDEX_BIT) | ((uint32_t)(bgColor) << GLYPHMAP_BGCOLOR_BIT) | ((uint32_t)(fgColor) << GLYPHMAP_FGCOLOR_BIT) | ((uint32_t)((options).value) << GLYPHMAP_OPTIONS_BIT))
inline uint8_t glyphMapItem_getIndex(uint32_t const volatile * mapItem) { return *mapItem >> GLYPHMAP_INDEX_BIT & 0xFF; }
inline Color glyphMapItem_getBGColor(uint32_t const volatile * mapItem) { return (Color)(*mapItem >> GLYPHMAP_BGCOLOR_BIT & 0x0F); }
inline Color glyphMapItem_getFGColor(uint32_t const volatile * mapItem) { return (Color)(*mapItem >> GLYPHMAP_FGCOLOR_BIT & 0x0F); }
inline GlyphOptions glyphMapItem_getOptions(uint32_t const volatile * mapItem) { return (GlyphOptions){.value = (uint16_t)(*mapItem >> GLYPHMAP_OPTIONS_BIT & 0xFFFF)}; }
inline void glyphMapItem_setOptions(uint32_t volatile * mapItem, GlyphOptions const & options) { *mapItem = (*mapItem & ~((uint32_t)0xFFFF << GLYPHMAP_OPTIONS_BIT)) | ((uint32_t)(options.value) << GLYPHMAP_OPTIONS_BIT); }

struct GlyphsBuffer {
  int16_t         glyphsWidth;
  int16_t         glyphsHeight;
  uint8_t const * glyphsData;
  int16_t         columns;
  int16_t         rows;
  uint32_t *      map;  // look at glyphMapItem_... inlined functions
};


struct GlyphsBufferRenderInfo {
  int16_t              itemX;  // starts from 0
  int16_t              itemY;  // starts from 0
  GlyphsBuffer const * glyphsBuffer;

  GlyphsBufferRenderInfo(int itemX_, int itemY_, GlyphsBuffer const * glyphsBuffer_) : itemX(itemX_), itemY(itemY_), glyphsBuffer(glyphsBuffer_) { }
};


/**
 * @brief Represents an image with 64 colors image and transparency.
 *
 * Each pixel uses 8 bits (one byte), 2 bits per channel - RGBA, with following disposition:
 *
 * 7 6 5 4 3 2 1 0
 * A A B B G G R R
 *
 * AA = 0 fully transparent, AA = 3 fully opaque.
 * Each color channel can have values from 0 to 3 (maxmum intensity).
 */
struct Bitmap {
  int16_t         width;          /**< Bitmap horizontal size */
  int16_t         height;         /**< Bitmap vertical size */
  uint8_t const * data;           /**< Bitmap binary data */
  bool            dataAllocated;  /**< If true data is released when bitmap is destroyed */

  Bitmap() : width(0), height(0), data(NULL), dataAllocated(false) { }
  Bitmap(int width_, int height_, void const * data_, bool copy = false);
  Bitmap(int width_, int height_, void const * data_, int bitsPerPixel, RGB foregroundColor, bool copy = false);
  ~Bitmap();
};


struct BitmapDrawingInfo {
  int16_t        X;
  int16_t        Y;
  Bitmap const * bitmap;

  BitmapDrawingInfo(int X_, int Y_, Bitmap const * bitmap_) : X(X_), Y(Y_), bitmap(bitmap_) { }
};


/**
 * @brief This enum defines a set of predefined mouse cursors.
 */
enum CursorName {
  CursorPointerAmigaLike,     /**< 11x11 Amiga like colored mouse pointer */
  CursorPointerSimpleReduced, /**< 10x15 mouse pointer */
  CursorPointerSimple,        /**< 11x19 mouse pointer */
  CursorPointerShadowed,      /**< 11x19 shadowed mouse pointer */
  CursorPointer,              /**< 12x17 mouse pointer */
  CursorPen,                  /**< 16x16 pen */
  CursorCross1,               /**< 9x9 cross */
  CursorCross2,               /**< 11x11 cross */
  CursorPoint,                /**< 5x5 point */
  CursorLeftArrow,            /**< 11x11 left arrow */
  CursorRightArrow,           /**< 11x11 right arrow */
  CursorDownArrow,            /**< 11x11 down arrow */
  CursorUpArrow,              /**< 11x11 up arrow */
  CursorMove,                 /**< 19x19 move */
  CursorResize1,              /**< 12x12 resize orientation 1 */
  CursorResize2,              /**< 12x12 resize orientation 2 */
  CursorResize3,              /**< 11x17 resize orientation 3 */
  CursorResize4,              /**< 17x11 resize orientation 4 */
  CursorTextInput,            /**< 7x15 text input */
};


/**
 * @brief Defines a cursor.
 */
struct Cursor {
  int16_t hotspotX;           /**< Cursor horizontal hotspot (0 = left bitmap side) */
  int16_t hotspotY;           /**< Cursor vertical hotspot (0 = upper bitmap side) */
  Bitmap  bitmap;             /**< Cursor bitmap */
};


struct QuadTreeObject;


/**
 * @brief Represents a sprite.
 *
 * A sprite contains one o more bitmaps (fabgl::Bitmap object) and has
 * a position in a scene (fabgl::Scane class). Only one bitmap is displayed at the time.<br>
 * It can be included in a collision detection group (fabgl::CollisionDetector class).
 * Bitmaps can have different sizes.
 */
struct Sprite {
  volatile int16_t   x;
  volatile int16_t   y;
  Bitmap const * *   frames;  // array of pointer to Bitmap
  int16_t            framesCount;
  int16_t            currentFrame;
  int16_t            savedX;
  int16_t            savedY;
  int16_t            savedBackgroundWidth;
  int16_t            savedBackgroundHeight;
  uint8_t *          savedBackground;
  QuadTreeObject *   collisionDetectorObject;
  struct {
    uint8_t visible:  1;
    // A static sprite should be positioned before dynamic sprites.
    // It is never re-rendered unless allowDraw is 1. Static sprites always sets allowDraw=0 after drawings.
    uint8_t isStatic:  1;
    // This is always '1' for dynamic sprites and always '0' for static sprites.
    uint8_t allowDraw: 1;
  };

  Sprite();
  ~Sprite();
  Bitmap const * getFrame() { return frames ? frames[currentFrame] : NULL; }
  int getFrameIndex() { return currentFrame; }
  void nextFrame() { ++currentFrame; if (currentFrame >= framesCount) currentFrame = 0; }
  Sprite * setFrame(int frame) { currentFrame = frame; return this; }
  Sprite * addBitmap(Bitmap const * bitmap);
  Sprite * addBitmap(Bitmap const * bitmap[], int count);
  void clearBitmaps();
  int getWidth()  { return frames[currentFrame]->width; }
  int getHeight() { return frames[currentFrame]->height; }
  void allocRequiredBackgroundBuffer();
  Sprite * move(int offsetX, int offsetY, bool wrapAround = true);
  Sprite * moveTo(int x, int y);
};


struct Path {
  Point const * points;
  int           pointsCount;
};


/**
 * @brief Specifies general paint options.
 */
struct PaintOptions {
  uint8_t swapFGBG : 1;  /**< If enabled swaps foreground and background colors */
};


struct Primitive {
  PrimitiveCmd cmd;
  union {
    int16_t                ivalue;
    RGB                    color;
    Point                  position;
    Size                   size;
    Glyph                  glyph;
    Rect                   rect;
    GlyphOptions           glyphOptions;
    RawData                rawData;
    PaintOptions           paintOptions;
    GlyphsBufferRenderInfo glyphsBufferRenderInfo;
    BitmapDrawingInfo      bitmapDrawingInfo;
    Path                   path;
  };

  Primitive() { }
};


struct PaintState {
  RGB          penColor;
  RGB          brushColor;
  Point        position;        // value already traslated to "origin"
  GlyphOptions glyphOptions;
  PaintOptions paintOptions;
  Rect         scrollingRegion;
  Point        origin;
  Rect         clippingRect;    // relative clipping rectangle
  Rect         absClippingRect; // actual absolute clipping rectangle (calculated when setting "origin" or "clippingRect")
};


/**
* @brief Represents the VGA controller
*
* Use this class to set screen resolution and to associate VGA signals to ESP32 GPIO outputs.
*
* This example initializes VGA Controller with 8 colors (5 GPIOs used) and 640x350 resolution:
*
*     // Assign GPIO22 to Red, GPIO21 to Green, GPIO19 to Blue, GPIO18 to HSync and GPIO5 to VSync
*     VGAController.begin(GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18, GPIO_NUM_5);
*
*     // Set 640x350@70Hz resolution
*     VGAController.setResolution(VGA_640x350_70Hz);
*
* This example initializes VGA Controller with 64 colors (8 GPIOs used) and 640x350 resolution:
*
*     // Assign GPIO22 and GPIO_NUM_21 to Red, GPIO_NUM_19 and GPIO_NUM_18 to Green, GPIO_NUM_5 and GPIO_NUM_4 to Blue, GPIO_NUM_23 to HSync and GPIO_NUM_15 to VSync
*     VGAController.begin(GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_23, GPIO_NUM_15);
*
*     // Set 640x350@70Hz resolution
*     VGAController.setResolution(VGA_640x350_70Hz);
*/
class VGAControllerClass {

public:

  /**
   * @brief This is the 8 colors (5 GPIOs) initializer.
   *
   * One GPIO per channel, plus horizontal and vertical sync signals.
   *
   * @param redGPIO GPIO to use for red channel.
   * @param greenGPIO GPIO to use for green channel.
   * @param blueGPIO GPIO to use for blue channel.
   * @param HSyncGPIO GPIO to use for horizontal sync signal.
   * @param VSyncGPIO GPIO to use for vertical sync signal.
   *
   * Example:
   *
   *     // Use GPIO 22 for red, GPIO 21 for green, GPIO 19 for blue, GPIO 18 for HSync and GPIO 5 for VSync
   *     VGAController.begin(GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18, GPIO_NUM_5);
   */
  void begin(gpio_num_t redGPIO, gpio_num_t greenGPIO, gpio_num_t blueGPIO, gpio_num_t HSyncGPIO, gpio_num_t VSyncGPIO);

  /**
   * @brief This is the 64 colors (8 GPIOs) initializer.
   *
   * Two GPIOs per channel, plus horizontal and vertical sync signals.
   *
   * @param red1GPIO GPIO to use for red channel, MSB bit.
   * @param red0GPIO GPIO to use for red channel, LSB bit.
   * @param green1GPIO GPIO to use for green channel, MSB bit.
   * @param green0GPIO GPIO to use for green channel, LSB bit.
   * @param blue1GPIO GPIO to use for blue channel, MSB bit.
   * @param blue0GPIO GPIO to use for blue channel, LSB bit.
   * @param HSyncGPIO GPIO to use for horizontal sync signal.
   * @param VSyncGPIO GPIO to use for vertical sync signal.
   *
   * Example:
   *
   *     // Use GPIO 22-21 for red, GPIO 19-18 for green, GPIO 5-4 for blue, GPIO 23 for HSync and GPIO 15 for VSync
   *     VGAController.begin(GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_23, GPIO_NUM_15);
   */
  void begin(gpio_num_t red1GPIO, gpio_num_t red0GPIO, gpio_num_t green1GPIO, gpio_num_t green0GPIO, gpio_num_t blue1GPIO, gpio_num_t blue0GPIO, gpio_num_t HSyncGPIO, gpio_num_t VSyncGPIO);

  /**
   * @brief Get number of bits allocated for each channel.
   *
   * Number of bits depends by which begin() initializer has been called.
   *
   * @return 1 (8 colors) or 2 (64 colors).
   */
  uint8_t getBitsPerChannel() { return m_bitsPerChannel; }

  /**
   * @brief Set current resolution using linux-like modeline.
   *
   * Modeline must have following syntax (non case sensitive):
   *
   *     "label" clock_mhz hdisp hsyncstart hsyncend htotal vdisp vsyncstart vsyncend vtotal (+HSync | -HSync) (+VSync | -VSync) [DoubleScan] [FrontPorchBegins | SyncBegins | BackPorchBegins | VisibleBegins] [MultiScanBlank]
   *
   * In fabglconf.h there are macros with some predefined modelines for common resolutions.
   * When MultiScanBlank and DoubleScan is specified then additional rows are not repeated, but just filled with blank lines.
   *
   * @param modeline Linux-like modeline as specified above.
   * @param viewPortWidth Horizontal viewport size in pixels. If less than zero (-1) it is sized to modeline visible area width.
   * @param viewPortHeight Vertical viewport size in pixels. If less then zero (-1) it is sized to maximum allocable.
   * @param doubleBuffered if True allocates another viewport of the same size to use as back buffer. Make sure there is enough free memory.
   *
   * Example:
   *
   *     // Use predefined modeline for 640x480@60Hz
   *     VGAController.setResolution(VGA_640x480_60Hz);
   *
   *     // The same of above using modeline string
   *     VGAController.setResolution("\"640x480@60Hz\" 25.175 640 656 752 800 480 490 492 525 -HSync -VSync");
   *
   *     // Set 640x382@60Hz but limit the viewport to 640x350
   *     VGAController.setResolution(VGA_640x382_60Hz, 640, 350);
   *
   */
  void setResolution(char const * modeline, int viewPortWidth = -1, int viewPortHeight = -1, bool doubleBuffered = false);

  void setResolution(Timings const& timings, int viewPortWidth = -1, int viewPortHeight = -1, bool doubleBuffered = false);

  Timings * getResolutionTimings() { return &m_timings; }

  /**
   * @brief Return the screen width in pixels.
   *
   * @return Screen width in pixels.
   */
  int getScreenWidth()    { return m_timings.HVisibleArea; }

  /**
   * @brief Return the screen height in pixels.
   *
   * @return Screen height in pixels.
   */
  int getScreenHeight()   { return m_timings.VVisibleArea; }

  /**
   * @brief Return horizontal position of the viewport.
   *
   * @return Horizontal position of the viewport (in case viewport width is less than screen width).
   */
  int getViewPortCol()    { return m_viewPortCol; }

  /**
   * @brief Return vertical position of the viewport.
   *
   * @return Vertical position of the viewport (in case viewport height is less than screen height).
   */
  int getViewPortRow()    { return m_viewPortRow; }

  /**
   * @brief Return horizontal size of the viewport.
   *
   * @return Horizontal size of the viewport.
   */
  int getViewPortWidth()  { return m_viewPortWidth; }

  /**
   * @brief Return vertical size of the viewport.
   *
   * @return Vertical size of the viewport.
   */
  int getViewPortHeight() { return m_viewPortHeight; }

  void addPrimitive(Primitive const & primitive);

  void primitivesExecutionWait();

  /**
   * @brief Enable or disable drawings inside vertical retracing time.
   *
   * When vertical retracing occurs (on Vertical Sync) an interrupt is trigged. Inside this interrupt primitives
   * like line, circles, glyphs, etc.. are painted.<br>
   * This method can disable (or reenable) this behavior, making drawing instantaneous. Flickering may occur when
   * drawings are executed out of retracing time.<br>
   * When background executing is disabled the queue is emptied executing all pending primitives.
   *
   * @param value When true drawings are done during vertical retracing, when false drawings are executed instantly.
   */
  void enableBackgroundPrimitiveExecution(bool value);

  /**
   * @brief Suspend drawings.
   *
   * Suspends drawings disabling vertical sync interrupt.<br>
   * After call to suspendBackgroundPrimitiveExecution() adding new primitives may cause a deadlock.<br>
   * To avoid it a call to "processPrimitives()" should be performed very often.<br>
   * This method maintains a counter so can be nested.
   */
  void suspendBackgroundPrimitiveExecution();

  /**
   * @brief Resume drawings after suspendBackgroundPrimitiveExecution().
   *
   * Resumes drawings enabling vertical sync interrupt.
   */
  void resumeBackgroundPrimitiveExecution();

  /**
   * @brief Draw immediately all primitives in the queue.
   *
   * Draws all primitives before they are processed in the vertical sync interrupt.<br>
   * May generate flickering because don't care of vertical sync.
   */
  void processPrimitives();

  /**
   * @brief Move screen by specified horizontal and vertical offset.
   *
   * Screen moving is performed moving horizontal and vertical Front and Back porchs.
   *
   * @param offsetX Horizontal offset in pixels. < 0 goes left, > 0 goes right.
   * @param offsetY Vertical offset in pixels. < 0 goes up, > 0 goes down.
   *
   * Example:
   *
   *     // Move screen 4 pixels right, 1 pixel left
   *     VGAController.moveScreen(4, -1);
   */
  void moveScreen(int offsetX, int offsetY);

  /**
   * @brief Reduce or expands screen size by the specified horizontal and vertical offset.
   *
   * Screen shrinking is performed changing horizontal and vertical Front and Back porchs.
   *
   * @param shrinkX Horizontal offset in pixels. > 0 shrinks, < 0 expands.
   * @param shrinkY Vertical offset in pixels. > 0 shrinks, < 0 expands.
   *
   * Example:
   *
   *     // Shrink screen by 8 pixels and move by 8 pixels to the left
   *     VGAController.shrinkScreen(8, 0);
   *     VGAController.moveScreen(8, 0);
   */
  void shrinkScreen(int shrinkX, int shrinkY);

  /**
   * @brief Set the list of active sprites.
   *
   * A sprite is an image that keeps background unchanged.<br>
   * There is no limit to the number of active sprites, but flickering and slow
   * refresh happens when a lot of sprites (or large sprites) are visible.<br>
   * To empty the list of active sprites call VGAControllerClass.removeSprites().
   *
   * @param sprites The list of sprites to make currently active.
   * @param count Number of sprites in the list.
   *
   * Example:
   *
   *     // define a sprite with user data (velX and velY)
   *     struct MySprite : Sprite {
   *       int  velX;
   *       int  velY;
   *     };
   *
   *     static MySprite sprites[10];
   *
   *     VGAController.setSprites(sprites, 10);
   */
  template <typename T>
  void setSprites(T * sprites, int count) {
    setSprites(sprites, count, sizeof(T));
  }

  /**
   * @brief Empty the list of active sprites.
   *
   * Call this method when you don't need active sprites anymore.
   */
  void removeSprites() { setSprites(NULL, 0, 0); }

  /**
   * @brief Force the sprites to be updated.
   *
   * Screen is automatically updated whenever a primitive is painted (look at CanvasClass).<br>
   * When a sprite updates its image or its position (or any other property) it is required
   * to force a refresh using this method.<br>
   * VGAControllerClass.refreshSprites() is required also when using the double buffered mode, to paint sprites.
   */
  void refreshSprites();

  /**
   * @brief Return true if VGAControllerClass is on double buffered mode.
   *
   * @return True if VGAControllerClass is on double buffered mode.
   */
  bool isDoubleBuffered() { return m_doubleBuffered; }

  /**
   * @brief Set mouse cursor and make it visible.
   *
   * @param cursor Cursor to use when mouse pointer need to be painted. NULL = disable mouse pointer.
   */
  void setMouseCursor(Cursor const * cursor);

  /**
   * @brief Set mouse cursor from a set of predefined cursors.
   *
   * @param cursorName Name (enum) of predefined cursor.
   *
   * Example:
   *
   *     VGAController.setMouseCursor(CursorName::CursorPointerShadowed);
   */
  void setMouseCursor(CursorName cursorName);

  /**
   * @brief Set mouse cursor position.
   *
   * @param X Mouse cursor horizontal position.
   * @param Y Mouse cursor vertical position.
   */
  void setMouseCursorPos(int X, int Y);

private:

  void init(gpio_num_t VSyncGPIO);

  uint8_t preparePixel(RGB rgb, bool HSync = false, bool VSync = false);

  void freeBuffers();
  void fillHorizBuffers(int offsetX);
  void fillVertBuffers(int offsetY);
  int fill(uint8_t volatile * buffer, int startPos, int length, uint8_t red, uint8_t green, uint8_t blue, bool hsync, bool vsync);
  void allocateViewPort();
  void freeViewPort();
  int calcRequiredDMABuffersCount(int viewPortHeight);

  void execPrimitive(Primitive const & prim);

  void execSetPixel(Point const & position);
  void execLineTo(Point const & position);
  void execFillRect(Rect const & rect);
  void execFillEllipse(Size const & size);
  void execDrawEllipse(Size const & size);
  void execClear();
  void execVScroll(int scroll);
  void execHScroll(int scroll);
  void execDrawGlyph(Glyph const & glyph, GlyphOptions glyphOptions, RGB penColor, RGB brushColor);
  void execDrawGlyph_full(Glyph const & glyph, GlyphOptions glyphOptions, RGB penColor, RGB brushColor);
  void execDrawGlyph_light(Glyph const & glyph, GlyphOptions glyphOptions, RGB penColor, RGB brushColor);
  void execInvertRect(Rect const & rect);
  void execCopyRect(Rect const & source);
  void execSwapFGBG(Rect const & rect);
  void execReadRawData(RawData const & rawData);
  void execWriteRawData(RawData const & rawData);
  void execRenderGlyphsBuffer(GlyphsBufferRenderInfo const & glyphsBufferRenderInfo);
  void execDrawBitmap(BitmapDrawingInfo const & bitmapDrawingInfo);
  void execSwapBuffers();
  void execDrawPath(Path const & path);
  void execFillPath(Path const & path);

  void updateAbsoluteClippingRect();

  void drawBitmap(int destX, int destY, Bitmap const * bitmap, uint8_t * saveBackground, bool ignoreClippingRect);

  void fillRow(int y, int x1, int x2, uint8_t pattern);
  void swapRows(int yA, int yB, int x1, int x2);

  void drawLine(int X1, int Y1, int X2, int Y2, uint8_t pattern);

  void hideSprites();
  void showSprites();

  void setSprites(Sprite * sprites, int count, int spriteSize);

  static void VSyncInterrupt();

  static void setupGPIO(gpio_num_t gpio, int bit, gpio_mode_t mode);

  // DMA related methods
  bool setDMABuffersCount(int buffersCount);
  void setDMABufferBlank(int index, void volatile * address, int length);
  void setDMABufferView(int index, int row, int scan, volatile uint8_t * * viewPort, bool onVisibleDMA);
  void setDMABufferView(int index, int row, int scan);
  void volatile * getDMABuffer(int index, int * length);

  int                    m_bitsPerChannel;  // 1 = 8 colors, 2 = 64 colors, set by begin()
  Timings                m_timings;
  int16_t                m_linesCount;
  volatile int16_t       m_maxVSyncISRTime; // Maximum us VSync interrupt routine can run

  // These buffers contains a full line, with FrontPorch, Sync, BackPorch and blank visible area, in the
  // order specified by timings.HStartingBlock
  volatile uint8_t *     m_HBlankLine_withVSync;
  volatile uint8_t *     m_HBlankLine;
  int16_t                m_HLineSize;

  bool                   m_doubleBuffered;

  volatile int16_t       m_viewPortCol;
  volatile int16_t       m_viewPortRow;
  volatile int16_t       m_viewPortWidth;
  volatile int16_t       m_viewPortHeight;

  // when double buffer is enabled the "drawing" view port is always m_viewPort, while the "visible" view port is always m_viewPortVisible
  // when double buffer is not enabled then m_viewPort = m_viewPortVisible
  volatile uint8_t * *   m_viewPort;
  volatile uint8_t * *   m_viewPortVisible;

  uint8_t *              m_viewPortMemoryPool[FABGLIB_VIEWPORT_MEMORY_POOL_COUNT + 1];  // last allocated pool is NULL

  volatile QueueHandle_t m_execQueue;
  PaintState             m_paintState;

  // when double buffer is enabled the running DMA buffer is always m_DMABuffersRunning
  // when double buffer is not enabled then m_DMABuffers = m_DMABuffersRunning
  lldesc_t volatile *    m_DMABuffersHead;
  lldesc_t volatile *    m_DMABuffers;
  lldesc_t volatile *    m_DMABuffersVisible;

  int                    m_DMABuffersCount;

  gpio_num_t             m_VSyncGPIO;
  int                    m_VSyncInterruptSuspended;             // 0 = enabled, >0 suspended
  bool                   m_backgroundPrimitiveExecutionEnabled; // when False primitives are execute immediately

  void *                 m_sprites;       // pointer to array of sprite structures
  int                    m_spriteSize;    // size of sprite structure
  int                    m_spritesCount;  // number of sprites in m_sprites array

  bool                   m_spritesHidden; // true between hideSprites() and showSprites()

  // mouse cursor (mouse pointer) support
  Sprite                 m_mouseCursor;
  int16_t                m_mouseHotspotX;
  int16_t                m_mouseHotspotY;

};



} // end of namespace


extern fabgl::VGAControllerClass VGAController;



#endif
