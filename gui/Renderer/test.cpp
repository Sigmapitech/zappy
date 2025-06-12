#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.hpp"

namespace {

  const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;

out vec2 TexCoord;

uniform mat4 mvp;

void main() {
    TexCoord = aTex;
    gl_Position = mvp * vec4(aPos, 1.0);
}
)";

  const char *fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex;

void main() {
    FragColor = texture(tex, TexCoord);
}
)";

  GLuint CompileShader(GLenum type, const char *src)
  {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      std::array<char, 512> log;
      glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
      std::cerr << "Shader Compile Error: " << log.data() << '\n';
    }
    return shader;
  }

  GLuint CreateProgram()
  {
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
  }

  void run(SDL2 &sdl)
  {
    Mesh mesh("cube.obj");
    std::shared_ptr<SDL2::Texture> textureOpt = sdl.LoadTexture("texture.png");
    if (!textureOpt)
      throw std::runtime_error("Failed to load texture from texture.png");
    mesh.LoadTexture(*textureOpt);

    GLuint shader = CreateProgram();

    bool running = true;
    while (running) {
      while (sdl.PollEvent())
        if (sdl.GetEvent().type == SDL_QUIT)
          running = false;

      float t = SDL_GetTicks() / 1000.0;
      glm::mat4 model = glm::
        rotate(glm::mat4(1.0), t, glm::vec3(0.5, 1.0, 0.0));
      glm::mat4 view = glm::
        translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -3.0));
      glm::mat4 proj = glm::
        perspective<float>(glm::radians(45.0), 800.0 / 600.0, 0.1, 100.0);
      glm::mat4 mvp = proj * view * model;

      glClearColor(0.1, 0.12, 0.15, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shader);  // set the shader program before setting mvp
      GLint mvpLoc = glGetUniformLocation(shader, "mvp");
      glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

      mesh.Draw(shader);

      sdl.SwapWindow();
    }

    glDeleteProgram(shader);
  }

}  // namespace

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
