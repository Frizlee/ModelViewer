#include "Prerequisites.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "image/Atlas.hpp"
#include "Batch.hpp"
#include "GeometryPart.hpp"
#include "gui/Font.hpp"
#include "gui/FontManager.hpp"
#include "GeometryBuffer.hpp"
#include "image/Image.hpp"
#include "image/PNGCodec.hpp"
#include "image/DDSCodec.hpp"
#include "Texture.hpp"
#include "Shaders.hpp"

class ObjectPart : public GeometryPart
{
public:
#pragma pack(push, 1)
	struct Vertex
	{
		float x, y, z;
		float u, v;

		Vertex() {}
		Vertex(float x, float y, float z
			, float u, float v) : x(x), y(y), z(z)
			, u(u), v(v) {}
	};
#pragma pack(pop)

	ObjectPart() {}
	uint8_t* getBytes();
	uint32_t getVertexCount();
	void clear();
	void update(std::vector<Vertex> &vertices);
	void update(std::vector<uint16_t> &&elements);
	void pushBack(float x, float y, float z, float u, float v);

	static std::shared_ptr<GeometryLayout> GetLayout();

private:
	std::vector<Vertex> mVertices;
};

typedef std::shared_ptr<ObjectPart> ObjectPartSharedPtr;

void ErrorCallback(int errorCode, const char* description);
GLuint LoadProgram(const char *vs, const char *fs);

int main(int argc, char** argv)
{
	glfwInit();
	glfwSetErrorCallback(ErrorCallback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window;
	window = glfwCreateWindow(512, 512, "Hello World!", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gl::sys::LoadFunctions();
	glfwSwapInterval(0);
	gl::Enable(gl::SCISSOR_TEST);
	gl::Enable(gl::DEPTH_TEST);
	gl::Enable(gl::BLEND);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
	
	gl::Viewport(0, 0, 512, 512);
	gl::Scissor(0, 0, 512, 512);
	gl::ClearColor(0.9f, 0.9f, 0.9f, 1.0f);

	GLuint program1, program2, program3;
	program1 = LoadProgram(FONT_VERTEX_SHADER, FONT_FRAGMENT_SHADER);
	program2 = LoadProgram(FONT_VERTEX_SHADER, BLUR_FRAGMENT_SHADER);
	program3 = LoadProgram(GEOMETRY_VERTEX_SHADER, GEOMETRY_FRAGMENT_SHADER);

	glm::vec4 color(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 ortho = glm::ortho(0.0f, 512.0f, 0.0f, 512.0f);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec4 distance = glm::vec4(0.0f, 0.0f, -150.0f, 0.0f);
	glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::quat quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 eye = glm::mat4_cast(quat) * distance;
	glm::mat4 view = glm::lookAt(glm::vec3(eye.x, eye.y, eye.z), center, glm::vec3(up.x, up.y, up.z));
	glm::mat4 proj = glm::perspective(90.0f, 1.0f, 10.0f, 500.0f);
	glm::mat4 model(1.0f);

	GLint orthoLoc1 = gl::GetUniformLocation(program1, "ortho");
	GLint colorLoc1 = gl::GetUniformLocation(program1, "color");
	GLint textureLoc1 = gl::GetUniformLocation(program1, "tex");
	GLint orthoLoc2 = gl::GetUniformLocation(program2, "ortho");
	GLint colorLoc2 = gl::GetUniformLocation(program2, "color");
	GLint textureLoc2 = gl::GetUniformLocation(program2, "tex");
	GLint mvpLoc3 = gl::GetUniformLocation(program3, "modelViewProjection");
	GLint textureLoc3 = gl::GetUniformLocation(program3, "sampler");

	double lastTime = glfwGetTime();
	double currTime;
	double deltaTime;
	double fpsUpdateTimer = 2.0;
	bool toggleD = false;

	glm::tvec2<double> lastMousePos;
	glm::tvec2<double> currMousePos;
	glm::tvec2<double> deltaMousePos;

	glfwGetCursorPos(window, &lastMousePos.x, &lastMousePos.y);
	
	FontManager fntMgr;
	fntMgr.setAtlas(std::make_shared<Atlas>(512, 512, ColorFormat::R));
	Font *fnt = fntMgr.getFont("FreeSans.ttf", 72);
	Font *fnt2 = fntMgr.getFont("segoeuisl.ttf", 16);
	Font *fnt3 = fntMgr.getFont("msjh.ttc", 16);

	// Font fnt("FreeSans.ttf", 48, &atlas);
	// Font fnt2("FreeSans.ttf", 12, &atlas);
	//fnt2->print("LoręąółńŁÓm ipsum dolor sit amet, \n"
	//	"consectetur			adipiscing elit, \n"
	//	"sed do eiusmod \t\ttempor incididunt \n"
	//	"ut labore et dolore magna aliqua. \n"
	//	"Ut enim ad minim veniam, quis nostrud exercitation ullamco \n"
	//	"laboris nisi ut aliquip ex ea commodo consequat. \n"
	//	"Duis aute irure dolor in reprehenderit in voluptate velit esse \n"
	//	"cillum dolore eu fugiat nulla pariatur. Excepteur sint \n"
	//	"occaecat cupidatat non proident, sunt in culpa qui \n"
	//	"officia deserunt mollit anim id est laborum",
	//	0, 400);
	Image img;
	img.loadFromFile("test.dds", &DDSCodec());
	Texture tex, tex2;
	tex2.createFromImage(img);

	GeometryBuffer buff(1024 * 1024);
	GeometryBuffer buff2(1024);
	Batch batch1(&buff, 1024 * 512, BatchDesc{ BatchDesc::DrawType::TRIANGLES, GuiPart::GetLayout() });
	Batch batch2(&buff, 128, BatchDesc{ BatchDesc::DrawType::TRIANGLES, GuiPart::GetLayout() });
	Batch batch3(&buff2, 1024, BatchDesc{ BatchDesc::DrawType::TRIANGLE_ELEMENTS, 
		ObjectPart::GetLayout() });
	
	GuiPartSharedPtr test(std::make_shared<GuiPart>());
	GuiPartSharedPtr fps(std::make_shared<GuiPart>());
	GuiPartSharedPtr text1(std::make_shared<GuiPart>());
	GuiPartSharedPtr text2(std::make_shared<GuiPart>());
	GuiPartSharedPtr text3(std::make_shared<GuiPart>());
	ObjectPartSharedPtr cube(std::make_shared<ObjectPart>());

	cube->pushBack(-50.0f, 50.0f,  50.0f, 0.0f, 1.0f); // TOP
	cube->pushBack( 50.0f, 50.0f,  50.0f, 1.0f, 1.0f);
	cube->pushBack(-50.0f, 50.0f, -50.0f, 0.0f, 0.0f);
	cube->pushBack( 50.0f, 50.0f, -50.0f, 1.0f, 0.0f);

	cube->pushBack(-50.0f, -50.0f,  50.0f, 0.0f, 1.0f); // BOTTOM
	cube->pushBack( 50.0f, -50.0f,  50.0f, 1.0f, 1.0f);
	cube->pushBack(-50.0f, -50.0f, -50.0f, 0.0f, 0.0f);
	cube->pushBack( 50.0f, -50.0f, -50.0f, 1.0f, 0.0f);

	cube->update(std::vector<uint16_t>{ 
		0, 1, 2, 1, 3, 2, // TOP
		4, 5, 6, 5, 7, 6, // BOTTOM
		0, 2, 4, 2, 6, 4, // LEFT
		3, 1, 7, 1, 5, 7, // RIGHT
		2, 3, 6, 3, 7, 6, // FRONT
		1, 0, 5, 0, 4, 5, // BACK
	});

	/*cube->pushBack(0.0f, 100.0f, 0.0f, 0.5f, 1.0f);
	cube->pushBack(100.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	cube->pushBack(-100.0f, 0.0f, 0.0f, 0.0f, 0.0f);*/


	test->pushBack(0.0f,	 512.0f, 0.0f, 1.0f);
	test->pushBack(512.0f, 512.0f, 1.0f, 1.0f);
	test->pushBack(0.0f,	 0.0f,	 0.0f, 0.0f);
	test->pushBack(512.0f, 512.0f, 1.0f, 1.0f);
	test->pushBack(0.0f,	 0.0f,	 0.0f, 0.0f);
	test->pushBack(512.0f, 0.0f,	 1.0f, 0.0f);

	fnt2->print(text1, "Hello World1", 0, 200);

	fnt2->print(text2, "OpenGL (Open Graphics Library — открытая графическая \n"
		"библиотека) — спецификация, \nопределяющая независимый от языка \n"
		"программирования кросс-платформенный \nпрограммный интерфейс \n"
		"для написания приложений, \nиспользующих двумерную и трехмерную \n"
		"компьютерную графику.",
		0, 400);

	fnt3->print(text3, "ふ䋤リャ 槊槚 媯饯Ủねめ 稧磪でぎゃ府, \n"
		"樦楌 猪フへユほ 囨榜け滯ニ 饦䧣ら びウゥ氯ひょ諧 \n"
		"媯饯Ủねめ 갣にゃにょマプ 䨦果 鏥愦榯 びウゥ氯ひょ諧\n"
		"骥夯詃䧺襃 ぴゃ榦 查ぴゅ揦 䧎觃ぷ ひゃ姌 埩ヴャ堩うぴゃ \n"
		"㦵稪姎ぎょゝ 儥秵ツァぢゐ, 儥秵ツァ 榦䧥ひ䦞鏥\n"
		"椯廦妤褎礊 㠣	て, が榟た奥む フョざぴ椢䶧 滧䏩ヂョ \n"
		"䧪橣 鏥愦榯 觃ぷ 媯饯Ủねめ 駤みゅえ褤フュ, やこ ゆと䨦\n"
		"駤みゅえ褤フュ 觚代秦饊韩 鰦さじゅ嫧じょ, ぴょ禌 さじゅ嫧\n"
		"媯饯Ủねめ ぽ仯じゃ禤儦 が榟た奥む ピェ骥 駧も勩 \n"
		"ぽ仯じゃ禤儦 が榟た奥む, 䶥だ夦りゅびゅ \n"
		"ちゅ難みじぐ フョざぴ椢䶧 饯Ủ ち稃ヱ"	,
		400, 400);
	
	//batch.newPart(test);
	batch1.newPart(fps);
	batch1.newPart(text1);
	batch1.newPart(text2);
	batch1.newPart(text3);
	batch1.update();

	batch2.newPart(test);
	batch2.update();

	//batch3.newPart(cube);
	//batch3.update();

	GLuint testVao;
	GLuint testVbo;
	GLuint testEbo;
	gl::GenVertexArrays(1, &testVao);
	gl::GenBuffers(1, &testVbo);
	gl::GenBuffers(1, &testEbo);

	gl::BindVertexArray(testVao);
	gl::BindBuffer(gl::ARRAY_BUFFER, testVbo);
	gl::BufferData(gl::ARRAY_BUFFER, cube->getVertexCount() * sizeof(ObjectPart::Vertex),
		cube->getBytes(), gl::STATIC_DRAW);
	gl::EnableVertexAttribArray(0);
	gl::VertexAttribPointer(0, 3, gl::FLOAT, gl::FALSE_, sizeof(ObjectPart::Vertex), 0);
	gl::EnableVertexAttribArray(1);
	gl::VertexAttribPointer(1, 2, gl::FLOAT, gl::FALSE_, sizeof(ObjectPart::Vertex), (GLvoid*)12);
	gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, testEbo);
	gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, cube->getElementCount() * sizeof(uint16_t), cube->getElements(), gl::STATIC_DRAW);

	//fntMgr.getAtlas()->flipVerticaly();
	tex.createFromImage(*fntMgr.getAtlas());

	GLuint sampler;
	gl::GenSamplers(1, &sampler);
	gl::SamplerParameteri(sampler, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
	gl::SamplerParameteri(sampler, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
	gl::SamplerParameteri(sampler, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
	gl::SamplerParameteri(sampler, gl::TEXTURE_MAG_FILTER, gl::LINEAR);

	GLuint depthBuffer;
	// gl::GenRenderbuffers(1, &depthBuffer);
	// gl::BindRenderbuffer(gl::RENDERBUFFER, depthBuffer);
	// gl::RenderbufferStorage(gl::RENDERBUFFER, gl::DEPTH_COMPONENT, 512, 512);
	gl::GenTextures(1, &depthBuffer);
	gl::BindTexture(gl::TEXTURE_2D, depthBuffer);
	gl::TexImage2D(gl::TEXTURE_2D, 0, gl::DEPTH_COMPONENT, 512, 512, 0, gl::DEPTH_COMPONENT, gl::FLOAT, nullptr);

	GLuint renderTexture;
	gl::GenTextures(1, &renderTexture);
	gl::BindTexture(gl::TEXTURE_2D, renderTexture);
	gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, 512, 512, 0, gl::RGBA, gl::UNSIGNED_BYTE, 0);

	GLuint frameBuffer;
	gl::GenFramebuffers(1, &frameBuffer);
	gl::BindFramebuffer(gl::FRAMEBUFFER, frameBuffer);
	// gl::FramebufferRenderbuffer(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::RENDERBUFFER, depthBuffer);
	gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::TEXTURE_2D, depthBuffer, 0);
	gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0, gl::TEXTURE_2D, renderTexture, 0);

	float angle = 0.0;

	while (!glfwWindowShouldClose(window))
	{
		currTime = glfwGetTime();
		deltaTime = currTime - lastTime;
		lastTime = currTime;

		glfwGetCursorPos(window, &currMousePos.x, &currMousePos.y);
		deltaMousePos = currMousePos - lastMousePos;
		lastMousePos = currMousePos;

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			toggleD = false;

		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
			toggleD = true;

		glfwPollEvents();
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
		fpsUpdateTimer -= deltaTime;

		if (fpsUpdateTimer <= 0.0)
		{ 
			fpsUpdateTimer = 2.0;
			glfwSetWindowTitle(window, std::to_string(deltaTime).c_str());
			text2->toggleHidden();
			fnt->print(fps,	std::to_string(1 / deltaTime).c_str(), 0, 500);

			batch1.update();
		}
		
		// Buffer drawing;
		gl::BindFramebuffer(gl::FRAMEBUFFER, frameBuffer);
		gl::DepthMask(gl::TRUE_);
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		
		angle += static_cast<float>(deltaTime);
		quat = glm::rotate(quat, static_cast<float>(deltaMousePos.x) * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		quat = glm::rotate(quat, static_cast<float>(deltaMousePos.y) * 0.01f, glm::vec3(1.0f, 0.0f, 1.0f));
		eye = distance * quat;
		up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * quat;

		model = glm::scale(glm::vec3(1.5f, 1.5f, 1.5f)) * glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::lookAt(glm::vec3(eye.x, eye.y, eye.z), center, 
			glm::vec3(up.x, up.y, up.z));
			
		gl::UseProgram(program3);
		gl::Uniform1i(textureLoc3, 0);
		tex.bind(0);
		gl::BindSampler(0, sampler);
		gl::UniformMatrix4fv(mvpLoc3, 1, gl::FALSE_, &(proj * view * model)[0][0]);
		gl::BindVertexArray(testVao);
		//gl::DrawElements(gl::TRIANGLES, cube->getElementCount(), gl::UNSIGNED_SHORT, 0);
		GLsizei counts[] = { cube->getElementCount() };
		GLvoid *indices[] = { 0 };
		gl::MultiDrawElements(gl::TRIANGLES, counts, gl::UNSIGNED_SHORT, indices, 1);
		//batch3.render();


		gl::DepthMask(gl::FALSE_);
		gl::UseProgram(program1);
		gl::Uniform1i(textureLoc1, 0);
		tex.bind(0);
		gl::BindSampler(0, sampler);
		gl::Uniform4fv(colorLoc1, 1, &color[0]);
		gl::UniformMatrix4fv(orthoLoc1, 1, gl::FALSE_, &ortho[0][0]);
		batch1.render();

		gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
		gl::Clear(gl::COLOR_BUFFER_BIT);
		gl::UseProgram(program2);
		gl::Uniform1i(textureLoc2, 0);
		gl::ActiveTexture(gl::TEXTURE0);
		if (toggleD)
			gl::BindTexture(gl::TEXTURE_2D, renderTexture);
		else
			gl::BindTexture(gl::TEXTURE_2D, depthBuffer);
		//tex2.bind(0);
		gl::BindSampler(0, sampler);
		gl::Uniform4fv(colorLoc2, 1, &color[0]);
		gl::UniformMatrix4fv(orthoLoc2, 1, gl::FALSE_, &ortho[0][0]);
		batch2.render();

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void ErrorCallback(int errorCode, const char* description)
{
	std::cerr << errorCode << " " << description << std::endl;
}

GLuint LoadProgram(const char *vs, const char *fs)
{
	GLuint vertexShaderID = gl::CreateShader(gl::VERTEX_SHADER);
	GLuint fragmentShaderID = gl::CreateShader(gl::FRAGMENT_SHADER);
	GLint result;
	GLint logLength;
	gl::ShaderSource(vertexShaderID, 1, (const GLchar**)&vs, nullptr);
	gl::CompileShader(vertexShaderID);
	gl::GetShaderiv(vertexShaderID, gl::COMPILE_STATUS, &result);
	gl::GetShaderiv(vertexShaderID, gl::INFO_LOG_LENGTH, &logLength);
	char *vslog = new char[logLength + 1];
	vslog[logLength] = '\0';
	gl::GetShaderInfoLog(vertexShaderID, logLength, nullptr, vslog);
	std::cout << vslog << std::endl;
	delete[] vslog;

	gl::ShaderSource(fragmentShaderID, 1, (const GLchar**)&fs, nullptr);
	gl::CompileShader(fragmentShaderID);
	gl::GetShaderiv(fragmentShaderID, gl::COMPILE_STATUS, &result);
	gl::GetShaderiv(fragmentShaderID, gl::INFO_LOG_LENGTH, &logLength);
	char *fslog = new char[logLength + 1];
	fslog[logLength] = '\0';
	gl::GetShaderInfoLog(fragmentShaderID, logLength, nullptr, fslog);
	std::cout << fslog << std::endl;
	delete[] fslog;

	GLuint program = gl::CreateProgram();
	gl::AttachShader(program, vertexShaderID);
	gl::AttachShader(program, fragmentShaderID);
	gl::LinkProgram(program);
	gl::GetProgramiv(program, gl::LINK_STATUS, &result);
	gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &logLength);
	char *proglog = new char[logLength + 1];
	proglog[logLength] = '\0';
	gl::GetProgramInfoLog(program, logLength, nullptr, proglog);
	std::cout << proglog << std::endl;
	delete[] proglog;

	gl::DeleteShader(vertexShaderID);
	gl::DeleteShader(fragmentShaderID);

	return program;
}

uint8_t* ObjectPart::getBytes()
{
	return reinterpret_cast<uint8_t*>(mVertices.data());
}

uint32_t ObjectPart::getVertexCount()
{
	return mVertices.size();
}

void ObjectPart::clear()
{
	if (mVertices.size() > 0)
	{
		mVertices.clear();
		markBatchDirty();
	}
}

void ObjectPart::update(std::vector<ObjectPart::Vertex> &vertices)
{
	mVertices = vertices;
	markBatchDirty();
}

void ObjectPart::update(std::vector<uint16_t> &&elements)
{
	mElements = std::move(elements);
}

void ObjectPart::pushBack(float x, float y, float z, float u, float v)
{
	mVertices.push_back(std::move(Vertex(x, y, z, u, v)));
	markBatchDirty();
}

std::shared_ptr<GeometryLayout> ObjectPart::GetLayout()
{
	static std::shared_ptr<GeometryLayout> layout = std::make_shared<GeometryLayout>(
		std::vector<uint8_t>{ 3, 2 },
		std::vector<GeometryLayout::Component>{
			GeometryLayout::Component::FLOAT,
			GeometryLayout::Component::FLOAT },
		std::vector<uint8_t>{ 0, 0 }, 20);

	return layout;
}


