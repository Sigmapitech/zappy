#include <iostream>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Demeter.hpp"
#include "Demeter/Renderer/asset_dir.hpp"
#include "logging/Logger.hpp"

void Dem::Demeter::Time::Update(const SDL2 &sdl2Instance)
{
  current = sdl2Instance.GetTicks64();
  delta = (current - last) / 1000.0F;  // convert ms to seconds
  last = current;
}

bool Dem::Demeter::Init(std::unique_ptr<SDL2> renderer, bool activateDebug)
{
  sdl2 = std::move(renderer);
  time = Time(*sdl2);
  glDebug = activateDebug;
  std::unique_ptr<VertexShader> vertexShader = std::
    make_unique<VertexShader>();
  if (!vertexShader->Init(ASSET_DIR "/vertexShader.glsl")) {
    Log::failed << "Failed to initialize vertex shader.";
    return false;
  }
  std::unique_ptr<FragmentShader> fragmentShader = std::
    make_unique<FragmentShader>();
  if (!fragmentShader->Init(ASSET_DIR "/fragmentShader.glsl")) {
    Log::failed << "Failed to initialize fragment shader.";
    return false;
  }

  if (!shader->Init(std::move(vertexShader), std::move(fragmentShader))) {
    Log::failed << "Failed to initialize shader program.";
    return false;
  }

  camera = Camera(glm::radians(90.0), 800.0 / 600.0);

  if (glDebug) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugCallback, nullptr);
  }
  return true;
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
    if (!entity->Update(*this))
      Log::warn << "Entity update failed";
}

void Dem::Demeter::Draw()
{
  sdl2->Clear(0.1, 0.12, 0.15, 1.0);

  shader->Use();  // set the shader program before setting mvp

  for (std::shared_ptr<Dem::IEntity> &entity: entityPool)
    if (!entity->Draw(*this))
      Log::warn << "Entity draw failed";

  if (isImGUIWindowCreated) {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  sdl2->SwapWindow();
}

void Dem::Demeter::HandleEvent()
{
  switch (sdl2->GetEvent().type) {
    case SDL_QUIT:
      isRunning = false;
      break;
    case SDL_KEYDOWN:
      input.keys[sdl2->GetEvent().key.keysym.scancode] = true;
      break;
    case SDL_KEYUP:
      input.keys[sdl2->GetEvent().key.keysym.scancode] = false;
      break;
    case SDL_MOUSEMOTION:
      input.mouseDeltaX = sdl2->GetEvent().motion.xrel;
      input.mouseDeltaY = sdl2->GetEvent().motion.yrel;
      input.mouseX = sdl2->GetEvent().motion.x;
      input.mouseY = sdl2->GetEvent().motion.y;
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (sdl2->GetEvent().button.button < 5)
        input.mouseButtons[sdl2->GetEvent().button.button] = true;
      break;
    case SDL_MOUSEBUTTONUP:
      if (sdl2->GetEvent().button.button < 5)
        input.mouseButtons[sdl2->GetEvent().button.button] = false;
      break;
    case SDL_WINDOWEVENT:
      {
        if (sdl2->GetEvent().window.event == SDL_WINDOWEVENT_RESIZED
            || sdl2->GetEvent().window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          int new_width = sdl2->GetEvent().window.data1;
          int new_height = sdl2->GetEvent().window.data2;
          sdl2->SetWindowSize(new_width, new_height);
          camera.SetAspectRatio((float)new_width / (float)new_height);
        }
        break;
      }
    default:
      break;
  }
}

void Dem::Demeter::Run()
{
  isRunning = true;
  while (isRunning) {
    while (sdl2->PollEvent()) {
      if (
        sdl2->GetEvent().type != SDL_KEYDOWN
        && sdl2->GetEvent().type != SDL_KEYUP
        && sdl2->GetEvent().type != SDL_TEXTINPUT)
        ImGui_ImplSDL2_ProcessEvent(&sdl2->GetEvent());
      HandleEvent();
    }

    Update();
    Draw();

    if (glDebug) {
      int e = GL_NO_ERROR;
      while ((e = glGetError()) != GL_NO_ERROR)
        std::cerr << "OpenGL Error: " << e << '\n';
    }
  }
}

std::shared_ptr<Texture> Dem::Demeter::AddTexture(const std::string &path)
{
  if (textureMap.contains(path))
    return texturePool[textureMap[path]];
  std::shared_ptr<Texture> tex = std::make_shared<Texture>();
  if (!tex->Init(*sdl2, path)) {
    Log::failed
      << "Failed to load texture from path: " << path
      << ". Using default texture instead.";
    tex = std::make_shared<Texture>();
    tex->Init(*sdl2, ASSET_DIR "/no-texture.png");
  }
  texturePool.push_back(tex);
  textureMap[path] = texturePool.size() - 1;
  return tex;
}

[[nodiscard]] std::optional<std::shared_ptr<Object3D>>
Dem::Demeter::AddObject3D(const std::string &path)
{
  if (objectMap.contains(path))
    return objectPool[objectMap[path]];
  std::shared_ptr<Object3D> object = std::make_shared<Object3D>();
  if (!object->Init(path)) {
    Log::failed << "Failed to initialize Object3D from path: " << path;
    return std::nullopt;
  }
  objectPool.push_back(object);
  objectMap[path] = objectPool.size() - 1;
  return object;
}
