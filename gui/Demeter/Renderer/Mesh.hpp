#pragma once

#include <vector>

#include "Camera.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Vertex.hpp"

/**
 * @brief Represents a 3D mesh with associated vertex and index data, OpenGL
 * buffers, and optional texture.
 *
 * The Mesh class encapsulates the data and operations required to render a 3D
 * mesh using OpenGL. It manages the Vertex Array Object (VAO), Vertex Buffer
 * Object (VBO), and Element Buffer Object (EBO), as well as the mesh's
 * vertices, indices, and an optional texture. The mesh is identified by a
 * name.
 *
 * @note Copying of Mesh objects is disabled to prevent resource duplication.
 */
struct Mesh {
private:
  GLuint VAO, VBO, EBO = 0;
  std::string _name;
  std::unique_ptr<std::vector<Vertex>> _vertices;
  std::unique_ptr<std::vector<unsigned int>> _indices;
  std::shared_ptr<Texture> _texture = nullptr;

  /**
   * @brief Generates the mesh data for the object.
   *
   * This function initializes or updates the mesh by generating the necessary
   * vertex, index, and other buffer data required for rendering. It should be
   * called whenever the mesh needs to be created or refreshed.
   *
   * @note Must only be called once after the mesh is constructed
   */
  void GenMesh();

public:
  /**
   * @brief Constructs a Mesh object with the given name, vertices, and
   * indices.
   *
   * @param name The name of the mesh.
   * @param vv A unique pointer to a vector containing the mesh's vertices.
   * @param vi A unique pointer to a vector containing the mesh's indices.
   *
   * @throws std::runtime_error if the vertices or indices are empty.
   */
  Mesh(
    std::string name,
    std::unique_ptr<std::vector<Vertex>> vv,
    std::unique_ptr<std::vector<unsigned int>> vi);
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  ~Mesh();

  [[nodiscard]] const std::string &GetName() const
  {
    return _name;
  }

  /**
   * @brief Sets the texture for the mesh.
   *
   * Associates the given texture with this mesh, replacing any previously set
   * texture.
   *
   * @param t A shared pointer to the Texture to be applied to the mesh.
   */
  void SetTexture(std::shared_ptr<Texture> t);

  /**
   * @brief Renders the mesh using the specified shader and transformation
   * matrices.
   *
   * This function draws the mesh by applying the given model, view, and
   * projection matrices, and using the provided shader program for rendering.
   *
   * @param shader Reference to the ShaderProgram used for rendering the mesh.
   * @param modelMatrix The model transformation matrix to position, scale, and
   * rotate the mesh in world space.
   * @param view The view matrix representing the camera's transformation.
   * @param projection The projection matrix defining the camera's lens.
   * (screen space)
   */
  void Draw(
    ShaderProgram &shader,
    const glm::mat4 &modelMatrix,
    const Camera &camera) const;

  /**
   * @brief Overloads the stream insertion operator to output the contents of a
   * Mesh.
   *
   * This function prints all vertices and indices of the given Mesh object to
   * the provided output stream. Each vertex is printed on a separate line,
   * followed by the mesh indices in groups of three per line.
   *
   * @param os The output stream to write to.
   * @param mesh The Mesh object whose contents are to be printed.
   * @return Reference to the output stream after writing.
   */
  friend std::ostream &operator<<(std::ostream &os, const Mesh &mesh)
  {
    for (const Vertex &v: *mesh._vertices)
      os << v << '\n';
    for (size_t i = 0; i < mesh._indices->size(); i += 3)
      os << mesh._indices->at(i) << ' ' << mesh._indices->at(i + 1) << ' '
         << mesh._indices->at(i + 2) << '\n';
    return os;
  }
};
