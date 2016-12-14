// Generated by imageconverter. Please, do not edit!

#include <touchgfx/Bitmap.hpp>
#include <BitmapDatabase.hpp>

#ifndef NO_USING_NAMESPACE_TOUCHGFX
using namespace touchgfx;
#endif

extern const unsigned char _bg[]; 
extern const unsigned char _down_btn[]; 
extern const unsigned char _down_btn_alpha_channel[]; 
extern const unsigned char _down_btn_pressed[]; 
extern const unsigned char _down_btn_pressed_alpha_channel[]; 
extern const unsigned char _up_btn[]; 
extern const unsigned char _up_btn_alpha_channel[]; 
extern const unsigned char _up_btn_pressed[]; 
extern const unsigned char _up_btn_pressed_alpha_channel[]; 

const touchgfx::Bitmap::BitmapData bitmap_database[] = {
    { _bg, 0, 480, 272, 0, 0, 480, 272, touchgfx::Bitmap::RGB565 },
    { _down_btn, _down_btn_alpha_channel, 130, 56, 3, 0, 124, 55, touchgfx::Bitmap::RGB565 },
    { _down_btn_pressed, _down_btn_pressed_alpha_channel, 130, 56, 3, 0, 124, 55, touchgfx::Bitmap::RGB565 },
    { _up_btn, _up_btn_alpha_channel, 130, 56, 3, 0, 124, 55, touchgfx::Bitmap::RGB565 },
    { _up_btn_pressed, _up_btn_pressed_alpha_channel, 130, 56, 3, 0, 124, 55, touchgfx::Bitmap::RGB565 }
};

namespace BitmapDatabase
{
  const touchgfx::Bitmap::BitmapData* getInstance()
  {
    return bitmap_database;
  }
  uint16_t getInstanceSize()
  {
    return (uint16_t)(sizeof(bitmap_database) / sizeof(touchgfx::Bitmap::BitmapData));
  }
}

