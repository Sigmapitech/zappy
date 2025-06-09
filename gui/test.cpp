#include <array>
#include <iostream>
#include <memory>
#include <random>

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

namespace {

  const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

flat out vec3 vertexColor;

uniform mat4 mvp;

void main() {
    vertexColor = aColor;
    gl_Position = mvp * vec4(aPos, 1.0);
}
)";

  const char *fragmentShaderSource = R"(
#version 330 core
flat in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

  // Interleaved vertex data: position (x, y, z) + color (r, g, b)
  std::array<float, 48> vertices = [] {
    std::array<float, 48> v = {
      // Positions          // Color (to be randomized)
      -0.5, -0.5, -0.5, 0, 0, 0,  // 0
      +0.5, -0.5, -0.5, 0, 0, 0,  // 1
      +0.5, +0.5, -0.5, 0, 0, 0,  // 2
      -0.5, +0.5, -0.5, 0, 0, 0,  // 3
      -0.5, -0.5, +0.5, 0, 0, 0,  // 4
      +0.5, -0.5, +0.5, 0, 0, 0,  // 5
      +0.5, +0.5, +0.5, 0, 0, 0,  // 6
      -0.5, +0.5, +0.5, 0, 0, 0   // 7
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    for (int i = 0; i < 8; i++) {
      v[(i * 6) + 3] = dis(gen);  // R
      v[(i * 6) + 4] = dis(gen);  // G
      v[(i * 6) + 5] = dis(gen);  // B
    }
    return v;
  }();

  std::array<unsigned int, 36> indices = {
    0, 1, 2, 2, 3, 0,  // back face
    4, 5, 6, 6, 7, 4,  // front face
    4, 0, 3, 3, 7, 4,  // left face
    1, 5, 6, 6, 2, 1,  // right face
    4, 5, 1, 1, 0, 4,  // bottom face
    3, 2, 6, 6, 7, 3   // top face
  };

  struct SDL {
  private:
    bool _isInit = false;
    SDL_Window *_window;
    SDL_GLContext _context = nullptr;
    SDL_Event _event;

    static std::string GetError()
    {
      return SDL_GetError();
    }

    void SetAttribute(SDL_GLattr attr, int value) const  // NOLINT musn't be
                                                         // static
    {
      if (SDL_GL_SetAttribute(attr, value) < 0)
        throw std::runtime_error(
          "SDL_GL_SetAttribute failed! SDL_Error: " + GetError());
    }

  public:
    SDL()
    {
      if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::
          runtime_error("SDL initialization failed! SDL_Error: " + GetError());
      SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
      SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

      _window = SDL_CreateWindow(
        "SDL2 OpenGL Cube",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
      if (_window == nullptr)
        throw std::
          runtime_error("SDL_CreateWindow failed! SDL_Error: " + GetError());

      _context = SDL_GL_CreateContext(_window);
      if (_context == nullptr)
        throw std::runtime_error(
          "SDL_GL_CreateContext failed! SDL_Error: " + GetError());
      glewExperimental = GL_TRUE;
      if (glewInit() != GLEW_OK)
        throw std::runtime_error("GLEW initialization failed!");
      glEnable(GL_DEPTH_TEST);
    }

    ~SDL()
    {
      SDL_GL_DeleteContext(_context);
      SDL_DestroyWindow(_window);
      SDL_Quit();
    }

    void SwapWindow() const
    {
      SDL_GL_SwapWindow(_window);
    }

    [[nodiscard]] bool PollEvent()
    {
      return SDL_PollEvent(&_event);
    }

    [[nodiscard]] const SDL_Event &GetEvent() const
    {
      return _event;
    }
  };


  GLuint compileShader(GLenum type, const char *src)
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

  void setupVAO(GLuint &VAO, GLuint &VBO, GLuint &EBO)
  {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      sizeof(indices),
      indices.data(),
      GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
  }

}  // namespace

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

  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  setupVAO(VAO, VBO, EBO);

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
    glBindVertexArray(VAO);
    GLint mvpLoc = glGetUniformLocation(shader, "mvp");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);
  }

  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shader);

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

