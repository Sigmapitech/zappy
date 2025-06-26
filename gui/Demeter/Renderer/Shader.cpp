#include <array>
#include <fstream>

#include "Shader.hpp"
#include "logging/Logger.hpp"

bool Shader::Init(const std::string &path)
{
  std::ifstream file(path);
  if (!file) {
    Log::failed << "Failed to open shader file: " << path;
    return false;
  }
  std::string data(
    (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  const char *src = data.c_str();
  file.close();

  _shader = glCreateShader(_type);
  glShaderSource(_shader, 1, &src, nullptr);
  glCompileShader(_shader);
  GLint success;
  glGetShaderiv(_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::array<char, 512> log;
    glGetShaderInfoLog(_shader, log.size(), nullptr, log.data());
    Log::failed << "Shader Compile Error: " << log.data();
    return false;
  }
  return true;
}

Shader::~Shader()
{
  glDeleteShader(_shader);
}

bool ShaderProgram::Init(
  std::unique_ptr<VertexShader> vertexShader,
  std::unique_ptr<FragmentShader> fragmentShader)
{
  vs = std::move(vertexShader);
  fs = std::move(fragmentShader);
  program = glCreateProgram();
  glAttachShader(program, vs->Get());
  glAttachShader(program, fs->Get());
  glLinkProgram(program);
  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    std::array<char, 512> log;
    glGetProgramInfoLog(program, log.size(), nullptr, log.data());
    Log::failed << "Shader Program Link Error: " << log.data();
    return false;
  }
  return true;
}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(program);
}

