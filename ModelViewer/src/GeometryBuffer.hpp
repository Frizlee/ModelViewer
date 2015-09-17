#ifndef GEOMETRY_BUFFER_HPP
#define GEOMETRY_BUFFER_HPP
#include "Prerequisites.hpp"

class GeometryBuffer
{
public:
	GeometryBuffer();
	GeometryBuffer(size_t size);

	void setSize(size_t size);
	void bind();
	void update(uint8_t *bytes, size_t size, size_t position);
	void update(std::vector<uint8_t> &bytes, size_t position);
	size_t alloc(size_t size);
	void free(size_t offset);

	static void Unbind();

private:
	GLuint mVbo;

	struct Node
	{
		Node *child[2];
		size_t offset;
		size_t size;
		bool free;
		Node() : child{ nullptr, nullptr }, free(true) {};
		~Node();
		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
		Node* insert(size_t size);
		void remove(size_t offset);
	} *mRoot;
};

#endif // GEOMETRY_BUFFER_HPP

