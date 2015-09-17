#ifndef SHADERS_HPP
#define SHADERS_HPP

const char *VERTEX_SHADER = ""
"#version 330 core\n"
"layout(location = 0) in vec3 pos;\n"
"layout(location = 1) in vec4 color;\n"
"out vec4 outColor;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
"	outColor = color;\n"
"}\n";

const char *FRAGMENT_SHADER = ""
"#version 330 core\n"
"in vec4 outColor;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"	fragColor = outColor;\n"
"}\n";

#endif // SHADERS_HPP

