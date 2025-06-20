#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "Renderer/asset_dir.hpp"

#include "Demeter.hpp"

Dem::Demeter::Demeter(std::unique_ptr<SDL2> renderer, bool debug)
  : sdl2(std::move(renderer)), glDebug(debug)
{
  std::unique_ptr<VertexShader> vertexShader = std::
    make_unique<VertexShader>(ASSET_DIR "/vertexShader.glsl");
  std::unique_ptr<FragmentShader> fragmentShader = std::
    make_unique<FragmentShader>(ASSET_DIR "/fragmentShader.glsl");
  shader = std::make_unique<ShaderProgram>(
    std::move(vertexShader), std::move(fragmentShader));

  glm::mat4 view = glm::
    translate(glm::mat4(1.0), glm::vec3(0.0, -75.0, -500.0));
  camera = Camera(view, glm::radians(45.0), 800.0 / 600.0);

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

  lastTime = currentTime;
  currentTime = SDL_GetPerformanceCounter();
  deltaTime =
    ((currentTime - lastTime) * 1000 / (double)SDL_GetPerformanceFrequency());

  for (std::shared_ptr<Dem::IEntity> &entity: entityPool)
    entity->Update(*this);
}

void Dem::Demeter::Draw()
{
  sdl2->Clear(0.1, 0.12, 0.15, 1.0);

  shader->Use();  // set the shader program before setting mvp

  for (std::shared_ptr<Dem::IEntity> &entity: entityPool)
    entity->Draw(*this);

  sdl2->SwapWindow();
}

void Dem::Demeter::Run()
{
  isRunning = true;
  while (isRunning) {
    while (sdl2->PollEvent())
      if (sdl2->GetEvent().type == SDL_QUIT)
        isRunning = false;

    Update();
    Draw();

    if (glDebug) {
      int e = GL_NO_ERROR;
      while ((e = glGetError()) != GL_NO_ERROR)
        std::cerr << "OpenGL Error: " << e << '\n';
    }
  }
}
