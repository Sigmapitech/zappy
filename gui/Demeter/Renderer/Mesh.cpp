#include <glm/gtc/type_ptr.hpp>

#include "Mesh.hpp"

#include "logging/Logger.hpp"

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

void Mesh::SetTexture(std::shared_ptr<Texture> t)
{
  _texture.swap(t);
}

void Mesh::
  Draw(ShaderProgram &shader, const glm::mat4 &model, const Camera &camera)
    const
{
  glBindVertexArray(VAO);

  glActiveTexture(GL_TEXTURE0);
  if (_texture)
    glBindTexture(GL_TEXTURE_2D, _texture->GetGL());
  else {
    Log::warn
      << "Mesh '" << _name << "' has no texture set, using default texture.";
    glBindTexture(GL_TEXTURE_2D, 0);  // unbind texture if not set
  }
  glUniform1i(shader.GetUniformLocation("tex"), 0);

  // Set uniforms
  GLint modelLoc = shader.GetUniformLocation("model");
  GLint viewLoc = shader.GetUniformLocation("view");
  GLint projLoc = shader.GetUniformLocation("projection");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.view));
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera.GetProj()));

  glDrawElements(GL_TRIANGLES, _indices->size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);  // unbind VAO
}
