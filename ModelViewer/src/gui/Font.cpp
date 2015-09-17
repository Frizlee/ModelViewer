#include "Font.hpp"
#include "../image/Atlas.hpp"

FT_Library Font::gFTLib = nullptr;
const uint8_t TAB_SIZE = 8; // Tab size in spaces;

Font::Font() : mFace(nullptr), mChars(), mAtlas(nullptr)
{
	if (gFTLib == nullptr)
	{
		if (FT_Init_FreeType(&gFTLib))
			throw 1; // TODO: Error handling.
	}
}

Font::Font(const std::string &facePath, uint8_t size, 
	std::shared_ptr<Atlas> atlas) : 
	mFace(nullptr), mChars(), mAtlas(nullptr)
{
	if (gFTLib == nullptr)
	{
		if (FT_Init_FreeType(&gFTLib))
			throw 1; // TODO: Error handling.
	}
	setFace(facePath, size, atlas);
}

Font::~Font()
{

}

void Font::setFace(const std::string &facePath, uint8_t size, 
	std::shared_ptr<Atlas> atlas)
{
	if (FT_New_Face(gFTLib, facePath.c_str(), 0, &mFace))
		throw 2; // TODO: Error handling.

	FT_Error err = FT_Select_Charmap(mFace, ft_encoding_unicode);
	FT_GlyphSlot g = mFace->glyph;
	mAtlas = atlas;
	mSize = size;

	FT_Set_Pixel_Sizes(mFace, 0, mSize);
	float maxAscend = 0.0f, maxDescend = 0.0f;

	// Default characters;
	for (uint8_t i = 32; i < 128; ++i)
	{
		cacheChar(i);
		// Glyph is still loaded untill next cacheChar call.

		// Calculate tab width
		if (i == 32)
			mTabWidth = (g->advance.x * TAB_SIZE) >> 6;

		// Calculate other dimensions.
		maxAscend = std::max(maxAscend, static_cast<float>(g->metrics.horiBearingY >> 6));
		maxDescend = std::min(maxDescend, 
			static_cast<float>((g->metrics.horiBearingY - g->metrics.height) >> 6));
	}

	mMaxHeight = static_cast<uint32_t>(maxAscend);
	mNewLineSpacing = static_cast<uint32_t>(maxAscend - maxDescend);
}

GuiPartSharedPtr Font::print(const std::string &text, float x, float y)
{
	GuiPartSharedPtr part = std::make_shared<GuiPart>();
	print(part, text, x, y);
	return part;
}

void Font::print(GuiPartSharedPtr part, const std::string &text, float x, float y)
{
	const unsigned int *p;
	float nx, ny;

	std::basic_string<unsigned int> text32 = std::wstring_convert<
		std::codecvt_utf8<unsigned int>, unsigned int>{}.from_bytes(text);

	nx = x;
	ny = y - mMaxHeight;
	if (part->getVertexCount() > 0)
		part->clear();

	part->reserve(text32.length() * 16);

	for (p = text32.data(); *p; ++p)
	{
		// Control characters.
		if (*p == '\n')
		{
			nx = x;
			ny -= mNewLineSpacing;
			continue;
		}

		if (*p == '\t')
		{
			float dx = nx - x;
			unsigned int tabs = static_cast<unsigned int>(dx / mTabWidth + 1.0f);
			nx = x + tabs * mTabWidth;
			continue;
		}

		// Normal characters.
		int j = mMap.find(*p);
		const GlyphInfo *gi;

		if (j == -1)
		{
			if (cacheChar(*p) == false)
				continue;
			gi = &mChars.at(mChars.size() - 1);
		}
		else
			gi = &mChars.at(j);

		float x2 = nx + gi->bl;
		float y2 = ny + gi->bt;
		float tu = static_cast<float>(gi->tx) / mAtlas->getWidth();
		float tv = static_cast<float>(gi->ty) / mAtlas->getHeight();

		nx += gi->ax;
		ny += gi->ay;

		/*if (y2 < pos.bottom || x2 > pos.right())
		continue;*/

		float w = gi->bw;
		float h = gi->bh;

		/*if (x2 + w > pos.right())
		{
		wScale = (pos.right() - x2) / w;
		w = pos.right() - x2;
		}

		if (y2 - h < pos.bottom)
		{
		hScale = (y2 - pos.bottom) / h;
		h = y2 - pos.bottom;
		}*/

		if (!w || !h)
			continue;


		part->pushBack(x2, y2, // Top-left
			tu, tv + h / mAtlas->getHeight());
		part->pushBack(x2 + w, y2, // Top-right
			tu + w / mAtlas->getWidth(), tv + h / mAtlas->getHeight());
		part->pushBack(x2, y2 - h, // Bottom-left
			tu, tv);
		part->pushBack(x2 + w, y2, // Top-right
			tu + w / mAtlas->getWidth(), tv + h / mAtlas->getHeight());
		part->pushBack(x2, y2 - h, // Bottom-left
			tu, tv);
		part->pushBack(x2 + w, y2 - h, // Bottom-right
			tu + w / mAtlas->getWidth(), tv);
	}
}

bool Font::cacheChar(uint32_t charCode)
{
	FT_GlyphSlot g = mFace->glyph;

	if (FT_Load_Char(mFace, charCode, FT_LOAD_RENDER))
		return false;

	GlyphInfo gi;
	gi.tx = 0;
	gi.ty = 0;

	if (g->bitmap.buffer != nullptr)
	{
		std::shared_ptr<Image> img = std::make_shared<Image>(g->bitmap.width,
			g->bitmap.rows, ColorFormat::R, g->bitmap.buffer);
		img->flipVerticaly();
		TRect<unsigned int> rc = mAtlas->insert(img);

		if (rc == TRect<uint32_t>(0, 0, 0, 0))
			return false;

		gi.tx = rc.left;
		gi.ty = rc.bottom;
	}

	gi.ax = static_cast<float>(g->advance.x >> 6);
	gi.ay = static_cast<float>(g->advance.y >> 6);
	gi.bw = static_cast<float>(g->bitmap.width);
	gi.bh = static_cast<float>(g->bitmap.rows);
	gi.bl = static_cast<float>(g->bitmap_left);
	gi.bt = static_cast<float>(g->bitmap_top);

	mMap.insert(charCode, mChars.size());
	mChars.push_back(gi);

	return true;
}

