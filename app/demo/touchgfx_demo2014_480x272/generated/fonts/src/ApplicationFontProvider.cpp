/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#include <fonts/ApplicationFontProvider.hpp>
#include <touchgfx/InternalFlashFont.hpp>

#ifndef NO_USING_NAMESPACE_TOUCHGFX
using namespace touchgfx;
#endif

extern touchgfx::InternalFlashFont& getFont_Asap_Regular_36_4bpp();
extern touchgfx::InternalFlashFont& getFont_Asap_Regular_15_4bpp();
extern touchgfx::InternalFlashFont& getFont_Asap_Regular_20_4bpp();
extern touchgfx::InternalFlashFont& getFont_Asap_Regular_22_4bpp();
extern touchgfx::InternalFlashFont& getFont_RobotoCondensed_Regular_14_4bpp();
extern touchgfx::InternalFlashFont& getFont_SourceSansPro_Regular_26_4bpp();
extern touchgfx::InternalFlashFont& getFont_SourceSansPro_Regular_16_4bpp();
extern touchgfx::InternalFlashFont& getFont_RobotoCondensed_Regular_12_4bpp();
extern touchgfx::InternalFlashFont& getFont_SourceSansPro_Regular_100_4bpp();
extern touchgfx::InternalFlashFont& getFont_SourceSansPro_Regular_18_4bpp();
extern touchgfx::InternalFlashFont& getFont_SourceSansPro_Regular_14_4bpp();
extern touchgfx::InternalFlashFont& getFont_RobotoCondensed_Regular_14_4bpp();
extern touchgfx::InternalFlashFont& getFont_RobotoCondensed_Regular_26_4bpp();

touchgfx::Font* ApplicationFontProvider::getFont(touchgfx::FontId fontId)
{
  switch(fontId)
  {
    
    case Typography::TEMPERATURE_DEGREE:
      return &(getFont_Asap_Regular_36_4bpp());
    
    case Typography::TEMPERATURE_MENU:
      return &(getFont_Asap_Regular_15_4bpp());
    
    case Typography::TEMPERATURE_MENU_DEGREES:
      return &(getFont_Asap_Regular_20_4bpp());
    
    case Typography::TEMPERATURE_HEADLINE:
      return &(getFont_Asap_Regular_22_4bpp());
    
    case Typography::GRAPH:
      return &(getFont_RobotoCondensed_Regular_14_4bpp());
    
    case Typography::IMAGEMENU_HEADLINE:
      return &(getFont_SourceSansPro_Regular_26_4bpp());
    
    case Typography::IMAGEMENU_MAINTEXT:
      return &(getFont_SourceSansPro_Regular_16_4bpp());
    
    case Typography::DEMOVIEW_MCULOAD_TEXT:
      return &(getFont_RobotoCondensed_Regular_12_4bpp());
    
    case Typography::WEATHER_LARGE_TEXT:
      return &(getFont_SourceSansPro_Regular_100_4bpp());
    
    case Typography::WEATHER_NORMAL_TEXT:
      return &(getFont_SourceSansPro_Regular_18_4bpp());
    
    case Typography::WEATHER_SMALL_TEXT:
      return &(getFont_SourceSansPro_Regular_14_4bpp());
    
    case Typography::ALBUM_SMALL:
      return &(getFont_RobotoCondensed_Regular_14_4bpp());
    
    case Typography::ALBUM_LARGE:
      return &(getFont_RobotoCondensed_Regular_26_4bpp());
    
    default:
      return 0;
  }
}
