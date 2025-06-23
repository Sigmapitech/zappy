#include <stdexcept>

#include <ft2build.h>

#include FT_FREETYPE_H

class Font {
private:
  FT_Library ft;
  FT_Face face;

public:
  Font(const std::string &fontPath, int fontSize)
  {
    if (FT_Init_FreeType(&ft) != 0)
      throw std::
        runtime_error("ERROR::FREETYPE: Could not init FreeType Library");

    if (FT_New_Face(ft, fontPath.c_str(), 0, &face) != 0)
      throw std::runtime_error("ERROR::FREETYPE: Failed to load font");

    // 0 for dynamic width
    if (FT_Set_Pixel_Sizes(face, 0, fontSize) != 0)
      throw std::runtime_error("ERROR::FREETYPE: Failed to set font size");
  }

  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;

  ~Font()
  {
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
  }

  void ActivateGlyph(char glyph)
  {
    if (FT_Load_Char(face, glyph, FT_LOAD_RENDER) != 0)
      throw std::runtime_error("ERROR::FREETYPE: Failed to load Glyph");
  }
};
