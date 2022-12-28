#include <Dragonfly/editor.h>
#include <Dragonfly/detail/buffer.h>
#include <Dragonfly/detail/vao.h>
#include <SDL/SDL_mixer.h>
#include "fractalsound.h"

int main(int argc, char* args[])
{
	df::Sample sam("Sounds of chaos", 800, 800, df::Sample::FLAGS::DEFAULT);
	df::Camera cam;								// Implements a camera event class with handles
	cam.SetView(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	cam.SetSpeed(4.0f);
	sam.AddHandlerClass(cam, 5);				// class callbacks will be called to change its state
	sam.AddHandlerClass<df::ImGuiHandler>(10);	// static handle functions only

	eltecg::ogl::ArrayBuffer MyVBO;	MyVBO.constructMutable(std::vector<glm::vec2>{ {-1, -1}, { 1, -1 }, { -1, 1 }, { 1, 1 }}, GL_STATIC_DRAW);
	eltecg::ogl::VertexArray MyVAO;	MyVAO.addVBO<glm::vec2>(MyVBO);

	df::VaoArrays demoVao((GLuint)MyVAO, GL_TRIANGLE_STRIP, 4, 0u);

	df::ShaderProgramVF program = "MyShaderProgram";
	program << "Shaders/vert.vert"_vert << "Shaders/frag.frag"_frag << df::LinkProgram;
	df::ShaderProgramVF postprocess = "Postprocess shader program";
	postprocess << "Shaders/postprocess.vert"_vert << "Shaders/postprocess.frag"_frag << df::LinkProgram;

	int w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();
	auto frameBuff = df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1);

	//General variables
	glm::vec2 lastClickPos{ 0,0 };
	float zoomValue = 0.5f;
	bool isShiftHeldDown = false;
	struct pianoKey {
		glm::vec2 pos{ 0 };
		int fractalType = 0;
		int freq = 440;
	};
	pianoKey pianoKeys[10];

	//Sound variables
	const int FS = 44100;
	int freq = 440;
	bool allowCloseNeighbours = true;
	FractalSound* fractalSound = new FractalSound(FS);

	//Graphic variables
	int maxIterations = 2500;
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

					fractalSound->PlaySoundAtPos(fractalType, lastClickPos, freq, allowCloseNeighbours);

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
				//play sound / record new sound
				if (isShiftHeldDown)
				{
					pianoKeys[pianoKey].pos = glm::vec2(lastClickPos.x, lastClickPos.y);
					pianoKeys[pianoKey].fractalType = fractalType;
					pianoKeys[pianoKey].freq = freq;
				}
				else if (glm::length(pianoKeys[pianoKey].pos) != 0)
				{
					struct pianoKey key = pianoKeys[pianoKey];
					fractalSound->PlaySoundAtPos(key.fractalType, key.pos, key.freq, true);
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
				ImGui::PushItemWidth(-220);
				ImGui::Text(u8"Poz�ci�: (%f, %f)", cam.GetEye().x, cam.GetEye().y);
				ImGui::Text(u8"Kurzor poz�ci�: (%f, %f)", lastClickPos.x, lastClickPos.y);
				ImGui::Text(u8"Sebess�g: %f, Zoom m�rt�ke: %f", cam.GetSpeed(), zoomValue);
				ImGui::SliderInt(u8"Hang alapfrekvencia", &freq, 0, 6000);
				ImGui::Checkbox(u8"K�zeli szomsz�dok hangj�nak enged�lyez�se", &allowCloseNeighbours);
				ImGui::SliderInt(u8"Maximum iter�ci�s l�p�sek", &maxIterations, 1, 2500, "%d"/*, ImGuiSliderFlags_Logarithmic */);
				ImGui::Combo(u8"Frakt�l t�pusa", &fractalType, fractalTypes, IM_ARRAYSIZE(fractalTypes));
				ImGui::SliderFloat(u8"Frakt�l h�tter�nek f�nyereje", &backgroundBrightness, 0.1, 3);
				ImGui::ColorEdit3(u8"Frakt�l belsej�nek sz�ne", insideColor);
				ImGui::ColorEdit3(u8"Frakt�l h�tter�nek sz�ne", outsideColor);
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
