#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include "Demeter.hpp"

Dem::Demeter::Time::Time(const SDL2 &sdl2Instance)
  : last(sdl2Instance.GetTicks64()), current(last)
{
}

void Dem::Demeter::Time::Update(const SDL2 &sdl2Instance)
{
  last = current;
  current = sdl2Instance.GetTicks64();
  delta = (current - last) * 1000.0 / sdl2Instance.GetTicks64();
}

Dem::Demeter::Demeter(std::unique_ptr<SDL2> renderer, bool activateDebug)
  : sdl2(std::move(renderer)), time(*sdl2), glDebug(activateDebug)
{
  std::unique_ptr<VertexShader> vertexShader = std::
    make_unique<VertexShader>(ASSET_DIR "/vertexShader.glsl");
  std::unique_ptr<FragmentShader> fragmentShader = std::
    make_unique<FragmentShader>(ASSET_DIR "/fragmentShader.glsl");
  shader = std::make_unique<ShaderProgram>(
    std::move(vertexShader), std::move(fragmentShader));

  glm::mat4 view = glm::
    translate(glm::mat4(1.0), glm::vec3(0.0, -75.0, -500.0));
  camera = Camera(view, glm::radians(90.0), 800.0 / 600.0);

  if (glDebug) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugCallback, nullptr);
  }
}

// OpenGL debug callback function
void Dem::Demeter::DebugCallback(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar *message,
  const void *userParam)
{
  (void)source;
  (void)type;
  (void)id;
  (void)severity;
  (void)length;
  (void)userParam;
  std::cerr << "GL DEBUG: " << message << "\n";
}

void Dem::Demeter::Update()
{
  time.Update(*sdl2);

  for (std::shared_ptr<Dem::IEntity> &entity: entityPool)
    entity->Update(*this);
}

void Dem::Demeter::SetupImGUIFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
}

void Dem::Demeter::Draw()
{
  sdl2->Clear(0.1, 0.12, 0.15, 1.0);

  shader->Use();  // set the shader program before setting mvp

  for (std::shared_ptr<Dem::IEntity> &entity: entityPool)
    entity->Draw(*this);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  sdl2->SwapWindow();
}

void Dem::Demeter::Run()
{
  isRunning = true;
  while (isRunning) {
    while (sdl2->PollEvent()) {
      ImGui_ImplSDL2_ProcessEvent(&sdl2->GetEvent());
      if (sdl2->GetEvent().type == SDL_QUIT)
        isRunning = false;
    }

    SetupImGUIFrame();
    Update();
    Draw();

    if (glDebug) {
      int e = GL_NO_ERROR;
      while ((e = glGetError()) != GL_NO_ERROR)
        std::cerr << "OpenGL Error: " << e << '\n';
    }
  }
}
