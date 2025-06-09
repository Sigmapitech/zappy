#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 mvp;

void main() {
    vertexColor = aColor;
    gl_Position = mvp * vec4(aPos, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

// Interleaved vertex data: position (x, y, z) + color (r, g, b)
float cubeVertices[] = {
  //        Position            Color
  -0.5, -0.5, -0.5, 1.0, 0.0, 0.0,                                   // Red
  0.5,  -0.5, -0.5, 0.0, 1.0, 0.0,                                   // Green
  0.5,  0.5,  -0.5, 0.0, 0.0, 1.0,                                   // Blue
  0.5,  0.5,  -0.5, 0.0, 0.0, 1.0, -0.5, 0.5,  -0.5, 1.0, 1.0, 0.0,  // Yellow
  -0.5, -0.5, -0.5, 1.0, 0.0, 0.0,

  -0.5, -0.5, 0.5,  0.5, 0.0, 0.5, 0.5,  -0.5, 0.5,  0.0, 1.0, 1.0,
  0.5,  0.5,  0.5,  1.0, 0.0, 1.0, 0.5,  0.5,  0.5,  1.0, 0.0, 1.0,
  -0.5, 0.5,  0.5,  0.0, 1.0, 0.5, -0.5, -0.5, 0.5,  0.5, 0.0, 0.5,

  -0.5, 0.5,  0.5,  1.0, 0.5, 0.0, -0.5, 0.5,  -0.5, 0.0, 0.0, 1.0,
  -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5,
  -0.5, -0.5, 0.5,  0.1, 0.9, 0.1, -0.5, 0.5,  0.5,  1.0, 0.5, 0.0,

  0.5,  0.5,  0.5,  0.8, 0.2, 0.3, 0.5,  0.5,  -0.5, 0.3, 0.9, 0.2,
  0.5,  -0.5, -0.5, 0.7, 0.1, 0.5, 0.5,  -0.5, -0.5, 0.7, 0.1, 0.5,
  0.5,  -0.5, 0.5,  0.2, 0.4, 0.6, 0.5,  0.5,  0.5,  0.8, 0.2, 0.3,

  -0.5, -0.5, -0.5, 0.6, 0.6, 0.1, 0.5,  -0.5, -0.5, 0.4, 0.4, 0.9,
  0.5,  -0.5, 0.5,  0.1, 0.8, 0.3, 0.5,  -0.5, 0.5,  0.1, 0.8, 0.3,
  -0.5, -0.5, 0.5,  0.7, 0.2, 0.9, -0.5, -0.5, -0.5, 0.6, 0.6, 0.1,

  -0.5, 0.5,  -0.5, 0.1, 0.1, 0.1, 0.5,  0.5,  -0.5, 0.5, 0.5, 0.5,
  0.5,  0.5,  0.5,  0.9, 0.9, 0.9, 0.5,  0.5,  0.5,  0.9, 0.9, 0.9,
  -0.5, 0.5,  0.5,  0.4, 0.8, 0.2, -0.5, 0.5,  -0.5, 0.1, 0.1, 0.1};

GLuint compileShader(GLenum type, const char *src)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[512];
    glGetShaderInfoLog(shader, 512, nullptr, log);
    std::cerr << "Shader Compile Error: " << log << std::endl;
  }
  return shader;
}

GLuint createProgram()
{
  GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return prog;
}

void setupVAO(GLuint &VAO, GLuint &VBO)
{
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // Color attribute
  glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
}

int main()
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(
    SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_Window *window = SDL_CreateWindow(
    "SDL2 OpenGL Cube",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800,
    600,
    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  SDL_GLContext context = SDL_GL_CreateContext(window);
  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_DEPTH_TEST);

  GLuint VAO, VBO;
  setupVAO(VAO, VBO);

  GLuint shader = createProgram();

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event))
      if (event.type == SDL_QUIT)
        running = false;

    float t = SDL_GetTicks() / 1000.0;
    glm::mat4 model = glm::rotate(glm::mat4(1.0), t, glm::vec3(0.5, 1.0, 0.0));
    glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -3.0));
    glm::mat4 proj = glm::
      perspective<float>(glm::radians(45.0), 800.0 / 600.0, 0.1, 100.0);
    glm::mat4 mvp = proj * view * model;

    glClearColor(0.1, 0.12, 0.15, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);
    GLint mvpLoc = glGetUniformLocation(shader, "mvp");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    SDL_GL_SwapWindow(window);
  }

  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteProgram(shader);

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

