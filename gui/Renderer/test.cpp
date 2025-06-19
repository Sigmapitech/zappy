#include <iostream>
#include <memory>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "asset_dir.h"

#include "Object3D.hpp"

namespace {

  // // OpenGL debug callback function
  // void myDebugCallback(GLenum source, GLenum type, GLuint id, GLenum
  // severity,
  //   GLsizei length, const GLchar* message, const void* userParam)
  // {
  //   std::cerr << "GL DEBUG: " << message << "\n";
  // }

  void run(SDL2 &sdl)
  {
    // glEnable(GL_DEBUG_OUTPUT);
    // glDebugMessageCallback(myDebugCallback, nullptr);

    Object3D object(ASSET_DIR "/maxwell.obj");
    std::shared_ptr<Texture> t_body = std::
      make_shared<Texture>(sdl, ASSET_DIR "/Dingus.png");
    std::shared_ptr<Texture> t_moustache = std::
      make_shared<Texture>(sdl, ASSET_DIR "/mikustache.png");
    object.SetTexture(0, t_body);
    object.SetTexture(1, t_moustache);

    std::unique_ptr<VertexShader> vertexShader = std::
      make_unique<VertexShader>(ASSET_DIR "/vertexShader.glsl");
    std::unique_ptr<FragmentShader> fragmentShader = std::
      make_unique<FragmentShader>(ASSET_DIR "/fragmentShader.glsl");
    ShaderProgram shader = ShaderProgram(
      std::move(vertexShader), std::move(fragmentShader));

    bool running = true;
    while (running) {
      while (sdl.PollEvent())
        if (sdl.GetEvent().type == SDL_QUIT)
          running = false;

      float t = SDL_GetTicks() / 1000.0;

      glm::mat4 view = glm::
        translate(glm::mat4(1.0), glm::vec3(0.0, -75.0, -500.0));
      glm::mat4 proj = glm::
        perspective<float>(glm::radians(45.0), 800.0 / 600.0, 0.1, 10000.0);

      object.modelMatrix = glm::
        rotate(glm::mat4(1.0), t, glm::vec3(0.0, 1.0, 0.0));

      glClearColor(0.1, 0.12, 0.15, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader.Use();  // set the shader program before setting mvp

      glm::vec3 lightPos(200.0, 500.0, 200.0);  // arbitrary light position
      glm::vec3 viewPos = glm::vec3(glm::inverse(view)[3]);  // camera position

      GLint lightPosLoc = shader.GetUniformLocation("lightPos");
      GLint viewPosLoc = shader.GetUniformLocation("viewPos");
      glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
      glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

      object.Draw(shader, view, proj);

      // int e =  GL_NO_ERROR;
      // while ((e = glGetError()) != GL_NO_ERROR)
      //   std::cerr << "OpenGL Error: " << e << '\n';

      sdl.SwapWindow();
    }
  }

}  // namespace

/*
// test main function
int main()
{
  try {
    SDL2 sdl;
    run(sdl);
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << '\n';
    return 84;
  }
  return 0;
}
*/
