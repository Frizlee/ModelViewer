#ifndef SHADERS_HPP
#define SHADERS_HPP

static const char *GEOMETRY_VERTEX_SHADER = ""
"#version 330 core\n"
"layout(location = 0) in vec3 pos;\n"
"layout(location = 1) in vec2 texCoord;\n"
"out vec2 texcoord;\n"
"uniform mat4 modelViewProjection;\n"
"void main()\n"
"{\n"
"	gl_Position = modelViewProjection * vec4(pos, 1.0);\n"
"	texcoord = texCoord;\n"
"	//texcoord = vec2(1.0, 1.0);\n"
"}\n";

static const char *GEOMETRY_FRAGMENT_SHADER = ""
"#version 330 core\n"
"in vec2 texcoord;\n"
"layout(location = 0) out vec4 fragColor;\n"
"uniform sampler2D sampler;\n"
"void main()\n"
"{\n"
"	fragColor = texture(sampler, texcoord);\n"
"	//fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n";

static const char *FONT_VERTEX_SHADER = ""
"#version 330 core\n"
"layout(location = 0) in vec2 pos;\n"
"layout(location = 1) in vec2 texCoord;"
"uniform mat4 ortho;\n"
"out vec2 coord;\n"
"void main()\n"
"{\n"
"	gl_Position = ortho * vec4(pos.x, pos.y, 0.0, 1.0);\n"
"	coord = texCoord;\n"
"}";

static const char *FONT_FRAGMENT_SHADER = ""
"#version 330 core\n"
"in vec2 coord;\n"
"layout(location = 0) out vec4 frag_color;\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"	frag_color = vec4(1.0f, 1.0f, 1.0f, texture(tex, coord).r) * color;\n"
"	//frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
"	//frag_color = texture(tex, coord);\n"
"}";

static const char *BLUR_FRAGMENT_SHADER = ""
"#version 330 core\n"
"in vec2 coord;\n"
"layout(location = 0) out vec4 frag_color;\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"const float blurSizeH = 1.0 / 300.0;\n"
"const float blurSizeV = 1.0 / 200.0;\n"
"void main()\n"
"{\n"
"	vec4 sum = vec4(0.0);\n"
"	for (int x = -4; x <= 4; ++x)\n"
"		for(int y = -4; y <= 4; ++y)\n"
"			sum += texture(tex, \n"
"				vec2(coord.x + x * blurSizeV, coord.y + y * blurSizeH)\n"
"			) / 81.0;"
"	frag_color = sum;\n"
"	frag_color = texture(tex, coord);\n"
"}\n";

#endif // SHADERS_HPP

