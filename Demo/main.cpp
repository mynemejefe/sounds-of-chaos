#include <Dragonfly/editor.h>		 //inlcludes most features
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h>	 //will be replaced
#include <SDL/SDL_mixer.h>

void BasicSound(glm::vec2 pos, int len, int FS, float buff[]) {
	float freq = 432;
	//freq = pos.length();


	for (int i = 0; i < len; ++i)
	{
		float fval = sinf(2 * (float)M_PI * freq / FS * i) / 4;
		buff[2 * i] = fval * i / len;
		buff[2 * i + 1] = fval * i / len;
	}
}

float Length2(glm::vec2 vec) {
	return  vec.x * vec.x + vec.y * vec.y;
}

glm::vec2 Mul(glm::vec2 vec) {
	return glm::vec2(vec.x * vec.x - vec.y * vec.y, vec.x * vec.y + vec.y * vec.x);
}

void FillFractal(glm::vec2 pos, int freq, int len, int FS, float buff[])
{
	glm::vec2 z = pos;
	glm::vec2 c = pos;
	int i = 0, max_iterations = 2000;

	while (Length2(z) <= 2 * 2 && i < max_iterations && i < len) {
		float length = Length2(z);

		float fval = sinf(2 * (float)M_PI * freq / FS * i) / 4;
		buff[2 * i] = fval * length;
		buff[2 * i + 1] = fval * length;

		z = Mul(z) + c;

		//std::cout << "iteration: " << i << ", length:  " << length << ", z={" << z.x << ";" << z.y << "}" << std::endl;

		i++;
	}

	if (i != len && i != 0) {
		int repeats = len / i - 1;
		int original_iterations = i;
		int j = 0;
		for (; i < len; i++) {
			buff[2 * i] = buff[2 * j];
			buff[2 * i + 1] = buff[2 * j + 1];
			j = j % original_iterations + 1;
		}
	}
}

int main(int argc, char* args[])
{
	df::Sample sam("Dragonfly Demo", 800, 800, df::Sample::FLAGS::DEFAULT);
	// df::Sample simplifies OpenGL, SDL, ImGui, RenderDoc in the render loop, and handles user input via callback member functions in priority queues
	df::Camera cam;								// Implements a camera event class with handles
	cam.SetView(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	cam.SetSpeed(2.0f);
	sam.AddHandlerClass(cam, 5);				// class callbacks will be called to change its state
	sam.AddHandlerClass<df::ImGuiHandler>(10);	// static handle functions only

	eltecg::ogl::ArrayBuffer MyVBO;	MyVBO.constructMutable(std::vector<glm::vec2>{ {-1, -1}, { 1, -1 }, { -1, 1 }, { 1, 1 }}, GL_STATIC_DRAW);
	eltecg::ogl::VertexArray MyVAO;	MyVAO.addVBO<glm::vec2>(MyVBO);		//these two classes will be removed from Dragonfly as soon as we have the replacement ready

	df::VaoArrays demoVao((GLuint)MyVAO, GL_TRIANGLE_STRIP, 4, 0u); // temporary solution that wraps an ID

	df::TextureCube<> testCubemap("Assets/xpos.png", "Assets/xneg.png", "Assets/ypos.png", "Assets/yneg.png", "Assets/zpos.png", "Assets/zneg.png");
	df::Texture2D<> testTex = testCubemap[df::X_POS]; // 2D view of a cubemap face

	df::ShaderProgramEditorVF program = "MyShaderProgram";
	program << "Shaders/vert.vert"_vert << "Shaders/frag.frag"_frag << df::LinkProgram;
	df::ShaderProgramEditorVF postprocess = "Postprocess shader program";
	postprocess << "Shaders/postprocess.vert"_vert << "Shaders/postprocess.frag"_frag << df::LinkProgram;

	int w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();
	auto frameBuff = df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1);

	//General variables
	glm::vec2 lastClickPos{ 0,0 };
	float zoomValue = 1.0f;

	//Sound variables
	int FS = 44100;
	int freq = 440;

	//Graphic variables
	float inside_color[3] = { 0.9f,0.5f,0.3f };
	float outside_color[3] = { 0.9f,0.5f,0.3f };
	float background_dim = 1;
	int max_iterations = 5000;
	float fractal_complexity = 1;

	sam.AddResize([&](int w, int h) {frameBuff = frameBuff.MakeResized(w, h); });
	sam.AddMouseMotion([&](SDL_MouseMotionEvent e) { return true; }, 6);
	sam.AddMouseWheel([&](SDL_MouseWheelEvent wheel)
		{
			if (wheel.y == 1) {
				zoomValue *= 1.05;
				cam.SetSpeed(cam.GetSpeed() / 1.1);
				//ChangeSpeed(1.05f);
				//moveSpeed_ *= 1.05f;
				//moveSpeed_ = 1 / zoomValue_;
				return true;
			}
			else if (wheel.y == -1) {
				zoomValue /= 1.05;
				cam.SetSpeed(cam.GetSpeed() * 1.1);
				//ChangeSpeed(1/1.05f);
				//moveSpeed_ /= 1.05f;
				//moveSpeed_ = 1 / zoomValue_;
				return true;
			}
			else return false;
		}, 6);
	sam.AddMouseDown([&](SDL_MouseButtonEvent mouse) 
		{ 
			float x = 0, y = 0;
			glm::vec3 pos = cam.GetEye();
			glm::vec2 resolution = cam.GetSize();

			if (mouse.type == SDL_MOUSEBUTTONDOWN) {
				switch (mouse.button) {
				case SDL_BUTTON_LEFT:
					x = (pos.x + (mouse.x / resolution.x - 0.5) * 2 / zoomValue / zoomValue);
					y = (pos.y - (mouse.y / resolution.y - 0.5) * 2 / zoomValue / zoomValue);
					lastClickPos = glm::vec2(x, y);

					if (-1 == Mix_OpenAudio(FS, AUDIO_F32, 2, 512))
					{
						return true;
					}

					Mix_Chunk* chunk = new Mix_Chunk;
					chunk->alen = 4 * 2 * FS;
					chunk->abuf = new Uint8[chunk->alen];
					chunk->allocated = 0;
					chunk->volume = 127;

					//BasicSound(glm::vec2(2, 3), FS, FS, (float*)chunk->abuf);
					FillFractal(glm::vec2(x, y), freq, FS, FS, (float*)chunk->abuf);

					Mix_PlayChannel(-1, chunk, 0);

					//Mix_FreeChunk(chunk);

					return true;
				}
			}
			else return false;
		}, 6);
	
	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();
			cam.RenderUI();
			
			ImGui::SetNextWindowSize({ 600,400 }, ImGuiCond_Always);
			if (ImGui::Begin("Fractal editor"))
			{
				ImGui::Text("Position: (%f, %f)", cam.GetEye().x, cam.GetEye().y);
				ImGui::Text("Cursor position: (%f, %f)", lastClickPos.x, lastClickPos.y);
				ImGui::Text("Speed: %f, Zoom level: %f", cam.GetSpeed(), zoomValue);
				ImGui::SliderInt("Sampling signal frequency", &FS, 8000, 80000);
				ImGui::SliderInt("Sound frequency base", &freq, 0, 1236);
				ImGui::SliderInt("Max iteration", &max_iterations, 1, 5000, "%d"/*, ImGuiSliderFlags_Logarithmic */ );
				ImGui::SliderFloat("Fractal complexity(?)", &fractal_complexity, 0.1, 1);
				ImGui::SliderFloat("Fractal background dim", &background_dim, 0.1, 2);
				ImGui::ColorEdit3("Fractal inside color", inside_color);
				ImGui::ColorEdit3("Fractal outside color", outside_color);
			}
			ImGui::End();

			frameBuff << df::Clear() << program
				<< "x_offset" << cam.GetEye().x << "y_offset" << cam.GetEye().y
				<< "zoom_value" << zoomValue
				<< "fractal_inside_col" << glm::vec3(inside_color[0], inside_color[1], inside_color[2])
				<< "fractal_outside_col" << glm::vec3(outside_color[0], outside_color[1], outside_color[2])
				<< "background_dim" << background_dim
				<< "max_iter" << max_iterations
				<< "fractal_complexity" << fractal_complexity;
			program << demoVao;	//Rendering: Ensures that both the vao and program is attached

			df::Backbuffer << df::Clear() << postprocess << "texFrame" << frameBuff.get<glm::u8vec3>();
			postprocess << df::NoVao(GL_TRIANGLES, 3); // Rendering a pixel shader

			GL_CHECK;
			program.Render(); postprocess.Render(); //only the UI!!
		}
	);
	return 0;
}
