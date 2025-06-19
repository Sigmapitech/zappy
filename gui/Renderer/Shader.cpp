#include <array>
#include <fstream>
#include <stdexcept>

#include <GL/glew.h>

#include "Shader.hpp"

AShader::AShader(GLenum type, const std::string &path)
{
  std::ifstream file(path);
  if (!file)
    throw std::runtime_error("Failed to open shader file: " + path);
  std::string data(
    (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  const char *src = data.c_str();
  file.close();

  shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::array<char, 512> log;
    glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
    throw std::
      runtime_error("Shader Compile Error: " + std::string(log.data()));
  }
}

AShader::~AShader()
{
  glDeleteShader(shader);
}

ShaderProgram::ShaderProgram(
  std::unique_ptr<VertexShader> vertexShader,
  std::unique_ptr<FragmentShader> fragmentShader)
  : vs(std::move(vertexShader)), fs(std::move(fragmentShader))
{
  program = glCreateProgram();
  glAttachShader(program, vs->Get());
  glAttachShader(program, fs->Get());
  glLinkProgram(program);
  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    std::array<char, 512> log;
    glGetProgramInfoLog(program, log.size(), nullptr, log.data());
    throw std::runtime_error("Program Link Error: " + std::string(log.data()));
  }
}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(program);
}

