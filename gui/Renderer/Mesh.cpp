#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include "Mesh.hpp"

Mesh::Mesh(
  std::string name,
  std::unique_ptr<std::vector<Vertex>> vv,
  std::unique_ptr<std::vector<unsigned int>> vi)
  : _name(std::move(name)), _vertices(std::move(vv)), _indices(std::move(vi))
{
  if (_vertices->empty() || _indices->empty())
    throw std::runtime_error("Mesh cannot be empty");

  GenMesh();
}

void Mesh::GenMesh()
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

  // TexCoord
  glVertexAttribPointer(
    1,
    2,
    GL_FLOAT,
    GL_FALSE,
    sizeof(Vertex),
    (void *)offsetof(Vertex, texCoord));  // NOLINT
  glEnableVertexAttribArray(1);

  // Normal
  // layout(location = 2) = normal
  glVertexAttribPointer(
    2,
    3,
    GL_FLOAT,
    GL_FALSE,
    sizeof(Vertex),
    (void *)offsetof(Vertex, normal));  // NOLINT
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

Mesh::~Mesh()
{
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &EBO);
}

void Mesh::LoadTexture(SDL2::Texture &t)
{
  glGenTextures(1, &_texture);
  glBindTexture(GL_TEXTURE_2D, _texture);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    t.GetFormat(),
    t.GetWidth(),
    t.GetHeight(),
    0,
    t.GetFormat(),
    GL_UNSIGNED_BYTE,
    t.GetPixels());

  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Mesh::Draw(
  GLuint shader,
  const glm::mat4 &model,
  const glm::mat4 &view,
  const glm::mat4 &projection) const
{
  glBindVertexArray(VAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texture);
  glUniform1i(glGetUniformLocation(shader, "tex"), 0);

  // Set uniforms
  GLint modelLoc = glGetUniformLocation(shader, "model");
  GLint viewLoc = glGetUniformLocation(shader, "view");
  GLint projLoc = glGetUniformLocation(shader, "projection");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

  glDrawElements(GL_TRIANGLES, _indices->size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);  // unbind VAO
}
