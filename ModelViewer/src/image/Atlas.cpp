#include "Atlas.hpp"

Atlas::Atlas() : mRoot(new Node())
{
	mRoot->rc.left = 0;
	mRoot->rc.bottom = 0;
	mRoot->rc.width = 0;
	mRoot->rc.height = 0;
}

Atlas::Atlas(unsigned int width, unsigned int height, ColorFormat format, 
	std::vector<unsigned char> bytes) :  mRoot(new Node())
{
	mRoot->rc.left = 0;
	mRoot->rc.bottom = 0;
	mRoot->rc.width = width;
	mRoot->rc.height = height;
	create(width, height, format, bytes);
}

Atlas::Atlas(unsigned int width, unsigned int height, ColorFormat format, 
	unsigned char *bytes) : mRoot(new Node())
{
	mRoot->rc.left = 0;
	mRoot->rc.bottom = 0;
	mRoot->rc.width = width;
	mRoot->rc.height = height;
	create(width, height, format, bytes);
}

Atlas::~Atlas()
{
	delete mRoot;
}

TRect<unsigned int> Atlas::insert(std::shared_ptr<Image> img)
{
	if (img->getColorFormat() != mFormat)
	{
		// TODO: Error handling.
		return TRect<unsigned int>(0, 0, 0, 0);
	}

	Node *nd = mRoot->insert(img->getWidth(), img->getHeight());

	if (nd == nullptr)
	{
		// TODO: Error handling.
		return TRect<unsigned int>(0, 0, 0, 0);
	}

	nd->imgPtr = img; 

	drawNode(nd);
	
	//Return rectangle with texture coordinates. Reversed Y axis. Not anymore!
	/*TRect<unsigned int> ret;
	ret.left = nd->rc.left;
	ret.width = nd->rc.width;
	ret.bottom = nd->rc.top();
	ret.height = nd->rc.height;*/

	return nd->rc;
}

//void Atlas::setMargin(unsigned int margin)
//{
//	mMargin = margin;
//}

void Atlas::drawNode(Node *nd)
{
	if (nd->child[0] != nullptr)
		drawNode(nd->child[0]);
	if (nd->child[1] != nullptr)
		drawNode(nd->child[1]);
	if (nd->imgPtr != nullptr)
	{
		TRect<unsigned int> *rc = &nd->rc;

		for (unsigned int i = 0; i < nd->imgPtr->getHeight(); ++i)
		{
			memcpy(&mBytes.at((rc->bottom + i) * mWidth + rc->left),
				&nd->imgPtr->getBytes().at(i * nd->imgPtr->getWidth()),
				nd->imgPtr->getWidth());
		}
	}
}

Atlas::Node::~Node()
{
	if (child[0] != nullptr)
		delete child[0];
	if (child[1] != nullptr)
		delete child[1];
}

Atlas::Node * Atlas::Node::insert(unsigned int width, unsigned int height)
{
	if (child[0] != nullptr && child[1] != nullptr)
	{
		Node *newNode = child[0]->insert(width, height);

		if (newNode != nullptr)
			return newNode;
		else
			return child[1]->insert(width, height);
	}
	else
	{
		if (imgPtr != nullptr)
			return nullptr;

		if (rc.width < width || rc.height < height)
			return nullptr;

		if (rc.width == width && rc.height == height)
			return this;

		child[0] = new Node();
		child[1] = new Node();

		unsigned int dw = rc.width - width;
		unsigned int dh = rc.height - height;

		if (dw > dh)
		{
			child[0]->rc = TRect<unsigned int>(rc.left,
				rc.bottom, width, rc.height);
			child[1]->rc = TRect<unsigned int>(rc.left + width,
				rc.bottom, rc.width - width, rc.height);
		}
		else
		{
			child[0]->rc = TRect<unsigned int>(rc.left,
				rc.bottom, rc.width, height);
			child[1]->rc = TRect<unsigned int>(rc.left,
				rc.bottom + height, rc.width, rc.height - height);
		}

		return child[0]->insert(width, height);
	}
}

