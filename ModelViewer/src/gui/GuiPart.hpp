#ifndef GUI_PART_HPP
#define GUI_PART_HPP
#include "../Prerequisites.hpp"
#include "../GeometryPart.hpp"


class GuiPart : public GeometryPart
{
public:
#pragma pack(push, 1)
	struct Vertex
	{
		float x, y;
		float u, v;
		
		Vertex() {}
		Vertex(float x, float y, float u, float v) 
			: x(x), y(y), u(u), v(v) {}
	};
#pragma pack(pop)

	GuiPart() {}
	uint8_t* getBytes();
	uint32_t getVertexCount();
	void clear();
	void update(std::vector<Vertex> &vertices);
	void pushBack(float x, float y, float u, float v);
	void reserve(size_t size);

	static std::shared_ptr<GeometryLayout> GetLayout();

private:
	std::vector<Vertex> mVertices;
};

typedef std::shared_ptr<GuiPart> GuiPartSharedPtr;


#endif // GUI_PART_HPP

