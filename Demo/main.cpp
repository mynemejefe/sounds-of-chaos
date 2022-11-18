#include <Dragonfly/editor.h>		 //inlcludes most features
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h>	 //will be replaced

int main(int argc, char* args[])
{
	df::Sample sam("Dragonfly Demo", 800, 800, df::Sample::FLAGS::DEFAULT);
	// df::Sample simplifies OpenGL, SDL, ImGui, RenderDoc in the render loop, and handles user input via callback member functions in priority queues
	df::Camera cam;								// Implements a camera event class with handles
	cam.SetView(glm::vec3(0, 0, 0), glm::vec3(0,1,0), glm::vec3(0,0,1));
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
	glm::vec2 lastClickPos{0,0};
	float zoomValue = 1.0f;

	//Graphic variables
	float inside_color[3] = { 0.9,0.5,0.3 };
	float outside_color[3] = { 0.9,0.5,0.3 };
	float background_dim = 1;
	int max_iterations = 5000;
	float fractal_complexity = 1;

	sam.AddResize([&](int w, int h) {frameBuff = frameBuff.MakeResized(w, h); });
	sam.AddMouseWheel([&](SDL_MouseWheelEvent wheel)
		{
			if (wheel.y == 1) {
				zoomValue *= 1.05;
				//ChangeSpeed(1.05f);
				//moveSpeed_ *= 1.05f;
				//moveSpeed_ = 1 / zoomValue_;
				return true;
			}
			else if (wheel.y == -1) {
				zoomValue /= 1.05;
				//ChangeSpeed(1/1.05f);
				//moveSpeed_ /= 1.05f;
				//moveSpeed_ = 1 / zoomValue_;
				return true;
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
				ImGui::SliderInt("Max iteration", &max_iterations, 1, 5000, "%d"/*, ImGuiSliderFlags_Logarithmic */ );
				ImGui::SliderFloat("Fractal complexity(?)", &fractal_complexity, 0.1, 1);
				ImGui::SliderFloat("Fractal background dim", &background_dim, 0, 1);
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
