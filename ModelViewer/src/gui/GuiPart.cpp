#include "GuiPart.hpp"

uint8_t* GuiPart::getBytes()
{
	return reinterpret_cast<uint8_t*>(mVertices.data());
}

uint32_t GuiPart::getVertexCount()
{
	return mVertices.size();
}

void GuiPart::clear()
{
	if (mVertices.size() > 0)
	{
		mVertices.clear();
		markBatchDirty();
	}
}

void GuiPart::update(std::vector<GuiPart::Vertex> &vertices)
{
	mVertices = vertices;
	markBatchDirty();
}

void GuiPart::pushBack(float x, float y, float u, float v)
{
	mVertices.push_back(std::move(GuiPart::Vertex(x, y, u, v)));
	markBatchDirty();
}

void GuiPart::reserve(size_t size)
{
	mVertices.reserve(size);
}

std::shared_ptr<GeometryLayout> GuiPart::GetLayout()
{
	static std::shared_ptr<GeometryLayout> layout = std::make_shared<GeometryLayout>(
		std::vector<uint8_t>{ 2, 2 },
		std::vector<GeometryLayout::Component>{ 
			GeometryLayout::Component::FLOAT, 
			GeometryLayout::Component::FLOAT 
		},
		std::vector<uint8_t>{ 0, 0 }, 
		16);

	return layout;
}

