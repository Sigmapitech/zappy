#include <memory>
#include <string>

#include <GL/glew.h>

class AShader {
protected:
  GLuint shader = 0;  // NOLINT

public:
  AShader(GLenum type, const std::string &path);
  AShader(const AShader &) = delete;
  AShader &operator=(const AShader &) = delete;
  ~AShader();

  [[nodiscard]] virtual GLuint Get() const
  {
    return shader;
  }
};

class VertexShader : public AShader {
public:
  VertexShader(const std::string &path) : AShader(GL_VERTEX_SHADER, path) {}
};

class FragmentShader : public AShader {
public:
  FragmentShader(const std::string &path) : AShader(GL_FRAGMENT_SHADER, path)
  {
  }
};

struct ShaderProgram {
private:
  GLuint program = 0;
  std::unique_ptr<VertexShader> vs;
  std::unique_ptr<FragmentShader> fs;

public:
  ShaderProgram(
    std::unique_ptr<VertexShader> vertexShader,
    std::unique_ptr<FragmentShader> fragmentShader);
  ShaderProgram(const ShaderProgram &) = delete;
  ShaderProgram &operator=(const ShaderProgram &) = delete;
  ~ShaderProgram();

  [[nodiscard]] GLuint Get() const
  {
    return program;
  }

  void Use() const
  {
    glUseProgram(program);
  }

  [[nodiscard]] GLint GetUniformLocation(const std::string &name) const
  {
    return glGetUniformLocation(program, name.c_str());
  }
};

