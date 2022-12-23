#include <Dragonfly/editor.h>		 //inlcludes most features
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h>	 //will be replaced
#include <SDL/SDL_mixer.h>
#include "fractalsound.h"

float Length2(glm::vec2 vec) {
	return  vec.x * vec.x + vec.y * vec.y;
}

glm::vec2 Mul(glm::vec2 u, glm::vec2 v) {
	return glm::vec2(u.x * v.x - u.y * v.y, u.x * v.y + u.y * v.x);
}

void FillFractal(int fractalType, glm::vec2 pos, int freq, int len, int FS, float buff[])
{
	glm::vec2 z = pos;
	glm::vec2 c = pos;
	int i = 0, max_iterations = 2000;

	while (Length2(z) <= 2 * 2 && i < max_iterations && i < len) {
		float length = Length2(z);

		float fval = sinf(2 * (float)M_PI * freq / FS * i) / 4;
		buff[2 * i] = fval * length;
		buff[2 * i + 1] = fval * length;

		switch (fractalType) {
		case 0:
			z = Mul(z, z) + c;
			break;
		case 1:
			z = Mul(Mul(z, z), z) + c;
			break;
		case 2:
			z = powf(sqrtf(Length2(z)), 2) + c;
			break;
		}

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

void PlaySoundAtPos(int fractalType, glm::vec2 pos, int FS, int freq) 
{
	if (-1 == Mix_OpenAudio(FS, AUDIO_F32, 2, 512))
	{
		return;
	}

	Mix_Chunk* chunk = new Mix_Chunk;
	chunk->alen = 4 * 2 * FS;
	//chunk->abuf = new Uint8[chunk->alen];
	chunk->abuf = (Uint8*)malloc(chunk->alen * sizeof(Uint8));
	chunk->allocated = 0;
	chunk->volume = 127;

	FillFractal(fractalType, pos, freq, FS, FS, (float*)chunk->abuf);
	
	Mix_PlayChannel(-1, chunk, 0);

	Sleep(1000);

	delete(chunk);
	//SDL_free(chunk);
	//Mix_FreeChunk(chunk);
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

	df::ShaderProgramEditorVF program = "MyShaderProgram";
	program << "Shaders/vert.vert"_vert << "Shaders/frag.frag"_frag << df::LinkProgram;
	df::ShaderProgramEditorVF postprocess = "Postprocess shader program";
	postprocess << "Shaders/postprocess.vert"_vert << "Shaders/postprocess.frag"_frag << df::LinkProgram;

	int w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();
	auto frameBuff = df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1);

	//General variables
	glm::vec2 lastClickPos{ 0,0 };
	float zoomValue = 1.0f;
	bool isShiftHeldDown = false;
	struct pianoKey {
		glm::vec2 pos{ 0 };
		int fractalType = 0;
		int freq = 440;
	};
	pianoKey pianoKeys[10];
	int language = 0;

	//Sound variables
	const int FS = 44100;
	int freq = 440;
	FractalSound* fractalSound = new FractalSound(FS);

	//Graphic variables
	int maxIterations = 5000;
	int fractalType = 0;
	const char* fractalTypes[]{ "Mandelbrot set", "Multibrot set (z = z^3+c)", "Burning ship fractal" };
	float insideColor[3] = { 0.9f,0.5f,0.3f };
	float outsideColor[3] = { 0.9f,0.5f,0.3f };
	float backgroundBrightness = 1;

	sam.AddMouseMotion([&](SDL_MouseMotionEvent e) { return true; }, 6);
	sam.AddMouseWheel([&](SDL_MouseWheelEvent wheel)
		{
			if (wheel.y == 1) {
				zoomValue *= 1.1;
				cam.SetSpeed(cam.GetSpeed() / 1.1);
				return true;
			}
			else if (wheel.y == -1) {
				zoomValue /= 1.1;
				cam.SetSpeed(cam.GetSpeed() * 1.1);
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
					x = (pos.x + (mouse.x / resolution.x - 0.5) * 2 / zoomValue);
					y = (pos.y - (mouse.y / resolution.y - 0.5) * 2 / zoomValue);
					lastClickPos = glm::vec2(x, y);

					fractalSound->PlaySoundAtPos(fractalType, lastClickPos, freq);

					return true;
				}
			}
			else return false;
		}, 6);
	sam.AddKeyDown([&](SDL_KeyboardEvent kb)
		{
			int pianoKey = -1;
			switch (kb.keysym.sym)
			{
			case SDLK_LSHIFT:	isShiftHeldDown = true; break;
			case SDLK_0:		pianoKey = 0; break;
			case SDLK_1:		pianoKey = 1; break;
			case SDLK_2:		pianoKey = 2; break;
			case SDLK_3:		pianoKey = 3; break;
			case SDLK_4:		pianoKey = 4; break;
			case SDLK_5:		pianoKey = 5; break;
			case SDLK_6:		pianoKey = 6; break;
			case SDLK_7:		pianoKey = 7; break;
			case SDLK_8:		pianoKey = 8; break;
			case SDLK_9:		pianoKey = 9; break;
			}

			if (pianoKey != -1)
			{
				/* play sound / record new sound */
				if (isShiftHeldDown)
				{
					pianoKeys[pianoKey].pos = glm::vec2(lastClickPos.x, lastClickPos.y);
					pianoKeys[pianoKey].fractalType = fractalType;
					pianoKeys[pianoKey].freq = freq;
				}
				else if (glm::length(pianoKeys[pianoKey].pos) != 0)
				{
					struct pianoKey key = pianoKeys[pianoKey];
					fractalSound->PlaySoundAtPos(key.fractalType, key.pos, key.freq);
				}
			}

			return true;
		}, 5);
	sam.AddKeyUp([&](SDL_KeyboardEvent kb) 
		{
			if (kb.keysym.sym == SDLK_LSHIFT)	
			{
				isShiftHeldDown = false;
			}
			return true; 
		}, 5);
	
	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();
			cam.RenderUI();

			ImGui::SetNextWindowSize({ 675,260 }, ImGuiCond_Always);
			if (ImGui::Begin("Fractal editor"))
			{
				if (language == 0) {
					ImGui::Text("Position: (%f, %f)", cam.GetEye().x, cam.GetEye().y);
					ImGui::Text("Cursor position: (%f, %f)", lastClickPos.x, lastClickPos.y);
					ImGui::Text("Speed: %f, Zoom level: %f", cam.GetSpeed(), zoomValue);
					ImGui::SliderInt("Sound frequency base", &freq, 0, 1236);
					ImGui::Checkbox("Play sounds for close neighbours", &allowCloseNeighbours);
					ImGui::SliderInt("Max iteration", &maxIterations, 1, 5000, "%d"/*, ImGuiSliderFlags_Logarithmic */ );
					ImGui::Combo("Fractal type", &fractalType, fractalTypes, IM_ARRAYSIZE(fractalTypes));
					ImGui::SliderFloat("Fractal background brightness", &backgroundBrightness, 0.1, 3);
					ImGui::ColorEdit3("Fractal inside color", insideColor);
					ImGui::ColorEdit3("Fractal outside color", outsideColor);
				}
				else if (language == 1) {
					ImGui::Text("Pozicio: (%f, %f)", cam.GetEye().x, cam.GetEye().y);
					ImGui::Text("Kurzor pozicio: (%f, %f)", lastClickPos.x, lastClickPos.y);
					ImGui::Text("Sebesseg: %f, Zoom merteke: %f", cam.GetSpeed(), zoomValue);
					ImGui::SliderInt("Hang alapfrekvencia", &freq, 0, 1236);
					ImGui::Checkbox("Play sounds for close neighbours", &allowCloseNeighbours);
					ImGui::SliderInt("Maximum iteracios lepesek", &maxIterations, 1, 5000, "%d"/*, ImGuiSliderFlags_Logarithmic */);
					ImGui::Combo("Fraktal tipusa", &fractalType, fractalTypes, IM_ARRAYSIZE(fractalTypes));
					ImGui::SliderFloat("Fraktal hatterenek fenyereje", &backgroundBrightness, 0.1, 3);
					ImGui::ColorEdit3("Fraktal belsejenek szine", insideColor);
					ImGui::ColorEdit3("Fraktal hatterenek szine", outsideColor);
				}
				
				if (ImGui::SmallButton("English")) {
					language = 0;
				} 
				ImGui::SameLine();
				if (ImGui::SmallButton("Magyar")) {
					language = 1;
				}
			}
			ImGui::End();

			frameBuff << df::Clear() << program
				<< "offset" << glm::vec2(cam.GetEye().x, cam.GetEye().y)
				<< "zoom_value" << zoomValue
				<< "fractal_inside_col" << glm::vec3(insideColor[0], insideColor[1], insideColor[2])
				<< "fractal_outside_col" << glm::vec3(outsideColor[0], outsideColor[1], outsideColor[2])
				<< "background_brightness" << backgroundBrightness
				<< "max_iter" << maxIterations
				<< "fractal_type" << fractalType;
			program << demoVao;	//Rendering: Ensures that both the vao and program is attached

			df::Backbuffer << df::Clear() << postprocess << "texFrame" << frameBuff.get<glm::u8vec3>();
			postprocess << df::NoVao(GL_TRIANGLES, 3); // Rendering a pixel shader

			GL_CHECK;
			program.Render(); postprocess.Render(); //only the UI!!
		}
	);
	return 0;
}
