#pragma once

#include <unordered_map>

#include "Camera.hpp"
#include "Mesh.hpp"

/**
 * @brief Represents a 3D object loaded from a file, containing geometry and
 * mesh data.
 *
 * The Object3D class is responsible for parsing 3D model files (such as .obj),
 * temporarily storing vertex data (positions, texture coordinates, normals),
 * and managing sub-objects and meshes. It provides functionality to draw the
 * object using a given shader program and transformation matrices.
 */
struct Object3D {
private:
  std::unordered_map<std::string, size_t> _idMap;
  std::vector<std::unique_ptr<Mesh>> _meshArr;

  // parsing data
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texCoords;
  std::vector<glm::vec3> normals;

  std::string currentName = "default";
  std::unique_ptr<std::vector<Vertex>> meshVertices = std::
    make_unique<std::vector<Vertex>>();
  std::unique_ptr<std::vector<unsigned int>> meshIndices = std::
    make_unique<std::vector<unsigned int>>();

  std::unordered_map<std::string, unsigned int> uniqueVertexMap;
  unsigned int index = 0;

  /**
   * @brief Handles the parsing and processing of a sub-object from the given
   * input stream.
   *
   * This function reads data from the provided std::istringstream and
   * processes it as a sub-object, updating the state of the parent object
   * accordingly.
   *
   * @param ss Reference to an input string stream containing sub-object data.
   */
  void HandleSubObject(std::istringstream &ss);

  /**
   * @brief Processes a vertex definition from an input string stream.
   *
   * This method reads and handles a vertex line from the provided input
   * stream, parsing vertex coordinates for 3D object construction.
   *
   * @param ss Reference to an input string stream containing the vertex data.
   */
  void HandleVertex(std::istringstream &ss);

  /**
   * @brief Handles the parsing and processing of vertex texture coordinates
   * from an input stream.
   *
   * This function reads texture coordinate data from the provided input string
   * stream, typically as part of loading or parsing 3D object files (such as
   * .obj files). The parsed texture coordinates are then stored or processed
   * as needed for rendering.
   *
   * @param ss Reference to an input string stream containing the texture
   * coordinate data.
   */
  void HandleVertexTexture(std::istringstream &ss);

  /**
   * @brief Handles the parsing and processing of a vertex normal from an input
   * stream.
   *
   * This function reads a vertex normal definition from the provided input
   * string stream, typically as part of parsing a 3D object file (such as OBJ
   * format). It extracts the normal vector components and stores or processes
   * them as needed for rendering.
   *
   * @param ss Reference to an input string stream containing the vertex normal
   * data.
   */
  void HandleVertexNormal(std::istringstream &ss);

  /**
   * @brief Parses and handles a face definition from an input stream.
   *
   * This function reads a face specification from the provided input string
   * stream, typically as part of parsing a 3D object file (such as OBJ
   * format). It processes the face data and updates the internal
   * representation of the 3D object accordingly.
   *
   * @param ss Reference to an input string stream containing the face
   * definition.
   * @note A fan triangulation method is used to convert the face into
   * triangles.
   */
  void HandleFace(std::istringstream &ss);

  /**
   * @brief Parses a line from an input stream with a given prefix.
   *
   * This function processes a line from the provided input string stream
   * (`ss`), using the specified `prefix` to determine how to interpret the
   * line's contents. It is used to extract and handle data from formatted OBJ
   * input.
   *
   * @param prefix The prefix string that identifies the type or category of
   * the line.
   * @param ss The input string stream containing the line to be parsed.
   */
  void ParseLine(const std::string &prefix, std::istringstream &ss);

public:
  /**
   * @brief 4x4 transformation matrix representing the model's local
   * transformations.
   *
   * This matrix is used to apply translation, rotation, and scaling to the 3D
   * object in world space. It is initialized as the identity matrix.
   */
  glm::mat4 modelMatrix = glm::mat4(1.0);  // NOLINT

  /**
   * @brief Constructs an Object3D from an OBJ file.
   *
   * Loads a 3D object from the given file path, initializing its geometry,
   * textures, and other relevant data required for rendering.
   *
   * @param path The file system path to the 3D object file to load.
   *
   * @throws std::runtime_error if the file cannot be opened or parsed.
   */
  Object3D(const std::string &path);

  /**
   * @brief Renders the 3D object using the provided shader and transformation
   * matrices.
   *
   * Iterates through all meshes associated with this Object3D instance and
   * calls their Draw method, passing the shader program and the model, view,
   * and projection matrices.
   *
   * @param shader Reference to the ShaderProgram used for rendering.
   * @param view The view matrix representing the camera's transformation.
   * @param proj The projection matrix defining the camera's lens. (screen
   * space)
   */
  void Draw(ShaderProgram &shader, const Camera &camera) const;

  /**
   * @brief Sets the texture for a specific mesh in the object.
   *
   * This function updates the texture of a mesh identified by its ID within
   * the object.
   *
   * @param meshID The ID of the mesh to update.
   * @param t A shared pointer to the Texture to be applied to the mesh.
   *
   * @throws std::out_of_range if the meshID is invalid.
   */
  void SetTexture(size_t meshID, std::shared_ptr<Texture> t);
};
