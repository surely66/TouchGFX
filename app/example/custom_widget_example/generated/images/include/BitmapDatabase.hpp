// Generated by imageconverter. Please, do not edit!

#ifndef BITMAPDATABASE_HPP
#define BITMAPDATABASE_HPP

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/Bitmap.hpp>

const uint16_t BITMAP_BG_ID = 0; // Size: 480x272 pixels
const uint16_t BITMAP_DOWN_BTN_ID = 1; // Size: 130x56 pixels
const uint16_t BITMAP_DOWN_BTN_PRESSED_ID = 2; // Size: 130x56 pixels
const uint16_t BITMAP_UP_BTN_ID = 3; // Size: 130x56 pixels
const uint16_t BITMAP_UP_BTN_PRESSED_ID = 4; // Size: 130x56 pixels

namespace BitmapDatabase
{
#ifndef NO_USING_NAMESPACE_TOUCHGFX
  using namespace touchgfx;
#endif

  class BitmapData;
  const touchgfx::Bitmap::BitmapData* getInstance();
  uint16_t getInstanceSize();
}

#endif
