#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <random>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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

    static std::string GetIMGError()
    {
      return IMG_GetError();
    }

    void SetAttribute(SDL_GLattr attr, int value) const  // NOLINT musn't be
                                                         // static
    {
      if (SDL_GL_SetAttribute(attr, value) < 0)
        throw std::runtime_error(
          "SDL_GL_SetAttribute failed! SDL_Error: " + GetError());
    }

  public:
    struct Texture {
      std::shared_ptr<SDL_Surface> surface;
      GLenum format;

      Texture(std::shared_ptr<SDL_Surface> surf, GLenum fmt)
        : surface(std::move(surf)), format(fmt)
      {
      }
    };

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

      if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0)
        throw std::
          runtime_error("SDL_image initialization failed: " + GetIMGError());
    }

    ~SDL()
    {
      IMG_Quit();
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

    [[nodiscard]] std::optional<Texture>
    LoadTexture(const std::string &path) const  // NOLINT musn't be static
    {
      std::shared_ptr<SDL_Surface> surface(IMG_Load(path.c_str()));
      if (!surface.get()) {
        std::cerr
          << "Failed to load image: " << path
          << "\nSDL_image error: " << IMG_GetError() << '\n';
        return std::nullopt;
      }

      GLenum format;
      if (surface->format->BytesPerPixel == 4) {
        format = (surface->format->Rmask == 0x000000ff) ? GL_RGBA : GL_BGRA;
      } else if (surface->format->BytesPerPixel == 3) {
        format = (surface->format->Rmask == 0x000000ff) ? GL_RGB : GL_BGR;
      } else {
        std::cerr << "Unknown image format\n";
        SDL_FreeSurface(surface.get());
        return std::nullopt;
      }
      return Texture(surface, format);
    }
  };

  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;

    friend std::ostream &operator<<(std::ostream &os, const Vertex &v)
    {
      os << v.position.x << ' ' << v.position.y << ' ' << v.position.z << '\t'
         << v.color.r << ' ' << v.color.g << ' ' << v.color.b;
      return os;
    }
  };

  struct Mesh {
  private:
    GLuint VAO, VBO, EBO;
    std::unique_ptr<std::vector<Vertex>> _vertices;
    std::unique_ptr<std::vector<unsigned int>> _indices;

  public:
    Mesh(
      std::unique_ptr<std::vector<Vertex>> vv,
      std::unique_ptr<std::vector<unsigned int>> vi)
      : _vertices(std::move(vv)), _indices(std::move(vi))
    {
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);
      glGenBuffers(1, &EBO);

      glBindVertexArray(VAO);

      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(
        GL_ARRAY_BUFFER,
        _vertices->size() * sizeof(Vertex),
        _vertices->data(),
        GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        _indices->size() * sizeof(unsigned int),
        _indices->data(),
        GL_STATIC_DRAW);

      // Position
      glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void *)offsetof(Vertex, position));  // NOLINT
      glEnableVertexAttribArray(0);

      // Color
      glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void *)offsetof(Vertex, color));  // NOLINT
      glEnableVertexAttribArray(1);

      glBindVertexArray(0);
    }

    ~Mesh()
    {
      glDeleteBuffers(1, &VBO);
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &EBO);
    }

    void Draw() const
    {
      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, _indices->size(), GL_UNSIGNED_INT, nullptr);
    }

    friend std::ostream &operator<<(std::ostream &os, const Mesh &mesh)
    {
      for (const Vertex &v: *mesh._vertices)
        os << v << '\n';
      for (size_t i = 0; i < mesh._indices->size(); i += 3)
        os << (*mesh._indices)[i] << ' ' << (*mesh._indices)[i + 1] << ' '
           << (*mesh._indices)[i + 2] << '\n';
      return os;
    }
  };

  std::unique_ptr<Mesh> LoadOBJ(const std::string &path)
  {
    std::ifstream file(path);
    if (!file.is_open()) {
      std::cerr << "Failed to open OBJ file: " << path << '\n';
      return nullptr;
    }

    std::vector<glm::vec3> positions;
    std::unordered_map<std::string, unsigned int> uniqueVertexMap;

    std::string line;
    unsigned int index = 0;

    std::unique_ptr<std::vector<Vertex>> outVertices = std::
      make_unique<std::vector<Vertex>>();
    std::unique_ptr<std::vector<unsigned int>> outIndices = std::
      make_unique<std::vector<unsigned int>>();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    while (std::getline(file, line)) {
      std::istringstream ss(line);
      std::string prefix;
      ss >> prefix;

      if (prefix == "v") {
        glm::vec3 pos;
        ss >> pos.x >> pos.y >> pos.z;
        positions.push_back(pos);
      } else if (prefix == "f") {
        std::string vertexStr;
        std::vector<unsigned int> faceIndices;

        while (ss >> vertexStr) {
          if (uniqueVertexMap.count(vertexStr) == 0) {
            int vIndex = std::stoi(vertexStr) - 1;
            Vertex vertex;
            vertex.position = positions[vIndex];
            vertex.color = glm::vec3(
              dis(gen),
              dis(gen),
              dis(gen));  // random color
            outVertices->push_back(vertex);
            uniqueVertexMap[vertexStr] = index++;
          }
          faceIndices.push_back(uniqueVertexMap[vertexStr]);
        }

        // Triangulate the polygon (fan method)
        for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
          outIndices->push_back(faceIndices[0]);
          outIndices->push_back(faceIndices[i]);
          outIndices->push_back(faceIndices[i + 1]);
        }
      }
    }

    return std::
      make_unique<Mesh>(std::move(outVertices), std::move(outIndices));
  }

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

  std::unique_ptr<Mesh> meshOpt = LoadOBJ("cube.obj");
  if (!meshOpt) {
    std::cerr << "Failed to load mesh.\n";
    return 84;
  }

  GLuint shader = CreateProgram();

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event))
      if (event.type == SDL_QUIT)
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

    meshOpt->Draw();

    SDL_GL_SwapWindow(window);
  }

  glDeleteProgram(shader);

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
