#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.hpp"

namespace {

  const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTex;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

  const char *fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D tex;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0); // white light

    vec3 texColor = texture(tex, TexCoord).rgb;
    vec3 ambient = 0.1 * texColor;
    vec3 finalColor = ambient + diffuse * texColor;

    FragColor = vec4(finalColor, 1.0);
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

      glClearColor(0.1, 0.12, 0.15, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shader);  // set the shader program before setting mvp
      GLint modelLoc = glGetUniformLocation(shader, "model");
      GLint viewLoc = glGetUniformLocation(shader, "view");
      GLint projLoc = glGetUniformLocation(shader, "projection");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

      glm::vec3 lightPos(2.0, 2.0, 2.0);  // arbitrary light position
      glm::vec3 viewPos = glm::vec3(glm::inverse(view)[3]);  // camera position

      GLint lightPosLoc = glGetUniformLocation(shader, "lightPos");
      GLint viewPosLoc = glGetUniformLocation(shader, "viewPos");
      glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
      glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

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
