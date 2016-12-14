/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#ifndef APPLICATIONFONTPROVIDER_HPP
#define APPLICATIONFONTPROVIDER_HPP

#include <touchgfx/FontManager.hpp>

struct Typography
{
  static const touchgfx::FontId MAIN_STYLE = 0;
  static const touchgfx::FontId BUTTON_STYLE = 1;
};

class ApplicationFontProvider : public touchgfx::FontProvider
{
public:
  virtual touchgfx::Font* getFont(touchgfx::FontId fontId);
};

#endif /* APPLICATIONFONTPROVIDER_HPP */
