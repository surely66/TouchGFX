/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#ifndef APPLICATIONFONTPROVIDER_HPP
#define APPLICATIONFONTPROVIDER_HPP

#include <touchgfx/FontManager.hpp>

struct Typography
{
  static const touchgfx::FontId BIGNUMBERS = 0;
  static const touchgfx::FontId TEMPERATURE_SMALL = 1;
  static const touchgfx::FontId TEMPERATURE_MEDIUM = 2;
  static const touchgfx::FontId TEMPERATURE_LARGE = 3;
  static const touchgfx::FontId JOGWHEEL_INDICATOR_TEXT = 4;
  static const touchgfx::FontId ROOM_TEXT = 5;
  static const touchgfx::FontId DATE_TEXT = 6;
  static const touchgfx::FontId TIME_TEXT = 7;
  static const touchgfx::FontId TOPTILE_TEXT = 8;
  static const touchgfx::FontId HOME_SCREEN_NORMAL = 9;
  static const touchgfx::FontId HOME_SCREEN_LARGE = 10;
  static const touchgfx::FontId ENTERTAINMENT_NORMAL = 11;
  static const touchgfx::FontId ENTERTAINMENT_SMALL = 12;
};

class ApplicationFontProvider : public touchgfx::FontProvider
{
public:
  virtual touchgfx::Font* getFont(touchgfx::FontId fontId);
};

#endif /* APPLICATIONFONTPROVIDER_HPP */
