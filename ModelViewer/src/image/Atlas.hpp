#ifndef ATLAS_PACKER_HPP
#define ATLAS_PACKER_HPP
#include "../Prerequisites.hpp"
#include "../Rect.hpp"
#include "Image.hpp"

class Atlas : public Image
{
public:
	Atlas();
	Atlas(unsigned int width, unsigned int height, ColorFormat format,
		std::vector<unsigned char> bytes);
	Atlas(unsigned int width, unsigned int height, ColorFormat format,
		unsigned char *bytes = nullptr);
	~Atlas();
	Atlas(const Atlas&) = delete;
	Atlas& operator=(const Atlas&) = delete;

	TRect<unsigned int> insert(std::shared_ptr<Image> img);
	//void setMargin(unsigned int margin);


private:
	//unsigned int mMargin;
	struct Node
	{
		Node *child[2];
		TRect<unsigned int> rc;
		std::shared_ptr<Image> imgPtr;

		Node() : child{ nullptr, nullptr }, imgPtr(nullptr) {}
		~Node();
		Node(const Node &nd) = delete;
		Node& operator=(const Node &nd) = delete;
		Node* insert(unsigned int width, unsigned int height);
	} *mRoot;
	

	void drawNode(Node *nd);
};

#endif // ATLAS_PACKER_HPP

