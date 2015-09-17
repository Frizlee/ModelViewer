#include "FontManager.hpp"
#include "../image/Atlas.hpp"
#include "Font.hpp"


FontManager::FontManager() : mFonts(), mFontDir("."), mAtlas(nullptr)
{
}

FontManager::~FontManager()
{
	std::map<std::string, Font*>::iterator it;

	for (it = mFonts.begin(); it != mFonts.end(); ++it)
	{
		delete it->second;
	}
}

Font * FontManager::getFont(const std::string &name, unsigned char size)
{
	std::string key = name;
	key += size;
	std::map<std::string, Font*>::iterator it;

	it = mFonts.find(key);

	if (it == mFonts.end())
	{
		std::string fontPath = mFontDir + "\\" + name;
		Font *fnt = new Font(fontPath, size, mAtlas);
		mFonts.insert(std::pair<std::string, Font*>(key, fnt));
		
		return fnt;
	}

	return it->second;
}

void FontManager::setFontDir(const std::string &path)
{
	mFontDir = path;
}

void FontManager::setAtlas(std::shared_ptr<Atlas> atlas)
{
	mAtlas = atlas;
}

std::shared_ptr<Atlas> FontManager::getAtlas()
{
	return mAtlas;
}

