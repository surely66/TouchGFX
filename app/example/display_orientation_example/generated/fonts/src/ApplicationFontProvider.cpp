/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#include <fonts/ApplicationFontProvider.hpp>
#include <touchgfx/InternalFlashFont.hpp>

#ifndef NO_USING_NAMESPACE_TOUCHGFX
using namespace touchgfx;
#endif

extern touchgfx::InternalFlashFont& getFont_RobotoCondensed_Regular_14_4bpp();

touchgfx::Font* ApplicationFontProvider::getFont(touchgfx::FontId fontId)
{
  switch(fontId)
  {
    
    case Typography::HEADLINE:
      return &(getFont_RobotoCondensed_Regular_14_4bpp());
    
    default:
      return 0;
  }
}
