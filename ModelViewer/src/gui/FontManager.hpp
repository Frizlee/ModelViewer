#ifndef FONT_MANAGER_HPP
#define FONT_MANAGER_HPP
#include "../Prerequisites.hpp"

class FontManager
{
public:
	FontManager();
	~FontManager();
	FontManager(const FontManager&) = delete;
	FontManager& operator=(const FontManager&) = delete;
	Font* getFont(const std::string &name, unsigned char size);
	void setFontDir(const std::string &path);
	void setAtlas(std::shared_ptr<Atlas> atlas);
	std::shared_ptr<Atlas> getAtlas();

private:
	std::map<std::string, Font*> mFonts;
	std::string mFontDir;
	std::shared_ptr<Atlas> mAtlas;
};

#endif // FONT_MANAGER_HPP

