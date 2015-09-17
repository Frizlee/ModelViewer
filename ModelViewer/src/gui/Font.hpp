#ifndef FONT_HPP
#define FONT_HPP
#include "../Prerequisites.hpp"
#include "CodeToGlyphIndexMap.hpp"
#include "GuiPart.hpp"
#include "../Rect.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

class Font
{
public:
	Font();
	Font(const std::string &facePath, uint8_t size, std::shared_ptr<Atlas> atlas);
	~Font();
	Font(const Font &fnt) = delete;
	Font& operator=(const Font &fnt) = delete;
	void setFace(const std::string &facePath, uint8_t size, std::shared_ptr<Atlas> atlas);
	GuiPartSharedPtr print(const std::string &text, float x, float y);
	void print(GuiPartSharedPtr part, const std::string &text, float x, float y);


private:
	static FT_Library gFTLib;
	FT_Face mFace;
	std::shared_ptr<Atlas> mAtlas;

	struct GlyphInfo
	{
		float ax, ay;
		float bw, bh;
		float bl, bt;
		uint32_t tx, ty;
	};

	uint8_t mSize;
	uint32_t mNewLineSpacing;
	uint32_t mTabWidth;
	uint32_t mMaxHeight;

	std::vector<GlyphInfo> mChars;
	CodeToGlyphIndexMap mMap;

	bool cacheChar(uint32_t charCode);
};

#endif // FONT_HPP

