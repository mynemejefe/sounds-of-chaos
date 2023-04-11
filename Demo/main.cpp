#include <Dragonfly/editor.h>
#include <Dragonfly/detail/buffer.h>
#include <Dragonfly/detail/vao.h>
#include <SDL/SDL_mixer.h>
#include "fractalsound.h"
#include "variables.h"
#include "fractalsoundtester.h"

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

	const char* fractalTypes[]{ "Mandelbrot set", "Burning ship fraktál" };
	const char* SoundGenerationModes[]{ "Egyszerű", "Additív" };

	Variables variables;
	FractalSound* fractalSound = new FractalSound(variables.FS);

#ifdef TESTING
	FractalSoundTester* tester = new FractalSoundTester();
	tester->RunAllTests();
#else

	sam.AddMouseMotion([&](SDL_MouseMotionEvent e) { return true; }, 6);
	sam.AddMouseWheel([&](SDL_MouseWheelEvent wheel)
		{
			if (wheel.y == 1) {
				variables.zoomValue *= 1.1;
				cam.SetSpeed(cam.GetSpeed() / 1.1);
				return true;
			}
			else if (wheel.y == -1) {
				variables.zoomValue /= 1.1;
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
					x = (pos.x + (mouse.x / resolution.x - 0.5) * 2 / variables.zoomValue);
					y = -(pos.y - (mouse.y / resolution.y - 0.5) * 2 / variables.zoomValue);
					variables.lastClickPos = glm::vec2(x, y);

					fractalSound->PlaySoundAtPos(variables);

					return true;
				}
			}
			else return false;
		}, 6);
	sam.AddKeyDown([&](SDL_KeyboardEvent kb)
		{
			int key = kb.keysym.sym;

			if (key == SDLK_LSHIFT) {
				variables.isShiftHeldDown = true;
			}
			
			if (key >= SDLK_0 && key <= SDLK_9) {
				int pianoKey = key - SDLK_0;
				fractalSound->ModifyPianoKey(variables, pianoKey);
			}

			return true;
		}, 5);
	sam.AddKeyUp([&](SDL_KeyboardEvent kb) 
		{
			if (kb.keysym.sym == SDLK_LSHIFT)	
			{
				variables.isShiftHeldDown = false;
			}
			return true; 
		}, 5);
	
	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();

			ImGui::SetNextWindowSize({ 500,320 }, ImGuiCond_Once);
			ImGui::SetNextWindowSizeConstraints(ImVec2{ 350,100 }, ImVec2{ 1000,2000 });
			if (ImGui::Begin("Beállítások"))
			{
				ImGui::PushItemWidth(-220);
				if (ImGui::CollapsingHeader("Kamera információ")) {
					ImGui::Text("Pozíció: (%f, %f)", cam.GetEye().x, cam.GetEye().y);
					ImGui::Text("Kurzor pozíció: (%f, %f)", variables.lastClickPos.x, variables.lastClickPos.y);
					ImGui::Text("Sebesség: %f, Zoom mértéke: %f", cam.GetSpeed(), variables.zoomValue);
				}
				if (ImGui::CollapsingHeader("Megjelenítés")) {
					ImGui::Combo("Fraktál típusa", &variables.fractalType, fractalTypes, IM_ARRAYSIZE(fractalTypes));
					ImGui::SliderInt("Fraktál hatványkitevöje", &variables.power, 2, 10, "%d");
					ImGui::SliderInt("Maximum iterácós lépések", &variables.maxIterations, 1, 2500, "%d");
					ImGui::SliderFloat("Fraktál hátterének fényereje", &variables.backgroundBrightness, 0.1, 5);
					ImGui::ColorEdit3("Fraktál belsejének színe", variables.insideColor);
					ImGui::ColorEdit3("Fraktál hátterének színe", variables.outsideColor);
					ImGui::ColorEdit3("Kattintás színe", variables.lastClickColor);
				}
				if (ImGui::CollapsingHeader("Hanggenerálás")) {
					ImGui::Combo("Hanggenerálás módja", &variables.soundGenerationMode, SoundGenerationModes, IM_ARRAYSIZE(SoundGenerationModes));
					ImGui::SliderInt("Hang alapfrekvencia", &variables.freq, 0, 6000);
					ImGui::Checkbox("Közeli szomszédok hangjának engedélyezése", &variables.allowCloseNeighbours);
					ImGui::Checkbox("Hangerő normalizálása", &variables.normalizeSound);
				}
			}
			ImGui::End();

			frameBuff << df::Clear() << program
				<< "offset" << glm::vec2(cam.GetEye().x, cam.GetEye().y)
				<< "last_click" << variables.lastClickPos
				<< "last_click_col" << glm::vec3(variables.lastClickColor[0], variables.lastClickColor[1], variables.lastClickColor[2])
				<< "zoom_value" << variables.zoomValue
				<< "fractal_inside_col" << glm::vec3(variables.insideColor[0], variables.insideColor[1], variables.insideColor[2])
				<< "fractal_outside_col" << glm::vec3(variables.outsideColor[0], variables.outsideColor[1], variables.outsideColor[2])
				<< "background_brightness" << variables.backgroundBrightness
				<< "max_iter" << variables.maxIterations
				<< "fractal_type" << variables.fractalType
				<< "power" << variables.power;
			program << demoVao;	//Rendering: Ensures that both the vao and program is attached

			df::Backbuffer << df::Clear() << postprocess << "texFrame" << frameBuff.get<glm::u8vec3>();
			postprocess << df::NoVao(GL_TRIANGLES, 3); // Rendering a pixel shader

			GL_CHECK;
			program.Render(); postprocess.Render(); //only the UI!!
		}
	);

#endif
	return 0;
}
