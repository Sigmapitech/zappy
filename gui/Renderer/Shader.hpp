#include <memory>
#include <string>

#include <GL/glew.h>

/**
 * @class Shader
 * @brief Base class for handling OpenGL shader objects.
 *
 * This class encapsulates the creation, management, and destruction of an
 * OpenGL shader. It is intended to be used as a base class for specific shader
 * types (e.g., vertex, fragment).
 *
 * @note Copy construction and assignment are disabled to prevent multiple
 * ownership of the shader resource.
 *
 * @param type The type of the shader (e.g., GL_VERTEX_SHADER,
 * GL_FRAGMENT_SHADER).
 * @param path The file path to the shader source code.
 */
class Shader {
protected:
  GLuint shader = 0;  // NOLINT

public:
  /**
   * @brief Constructs a Shader object by loading and compiling a shader from a
   * file.
   *
   * This constructor reads the shader source code from the specified file
   * path, creates an OpenGL shader object of the given type, sets the shader
   * source, and compiles it. If the file cannot be opened or the shader fails
   * to compile, a std::runtime_error is thrown with an appropriate error
   * message.
   *
   * @param type The type of shader to create (e.g., GL_VERTEX_SHADER,
   * GL_FRAGMENT_SHADER).
   * @param path The file path to the shader source code.
   *
   * @throws std::runtime_error If the shader file cannot be opened or
   * compilation fails.
   */
  Shader(GLenum type, const std::string &path);
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  ~Shader();

  [[nodiscard]] virtual GLuint Get() const
  {
    return shader;
  }
};

/**
 * @class VertexShader
 * @brief Represents a vertex shader in the rendering pipeline.
 *
 * This class is responsible for loading, compiling, and managing a vertex
 * shader from a specified file path. It inherits from the Shader base class
 * and specifically handles shaders of type GL_VERTEX_SHADER.
 *
 * Example usage:
 * @code
 * VertexShader vertexShader("shaders/vertex.glsl");
 * @endcode
 *
 * @see Shader
 */
class VertexShader : public Shader {
public:
  /**
   * @brief Constructs a VertexShader object by loading and compiling a vertex
   * shader from a file.
   *
   * @param path The file path to the vertex shader source code.
   *
   * @throws std::runtime_error If the shader file cannot be opened or
   * compilation fails.
   *
   * @see Shader::Shader(GLenum type, const std::string &path)
   */
  VertexShader(const std::string &path) : Shader(GL_VERTEX_SHADER, path) {}
};

/**
 * @class FragmentShader
 * @brief Represents a fragment shader, responsible for handling pixel-level
 * processing in the rendering pipeline.
 *
 * This class inherits from Shader and specifically loads, compiles, and
 * manages a fragment shader from a given file path. Fragment shaders are
 * executed for each pixel and are typically used to compute color and other
 * attributes of each fragment.
 *
 * Example usage:
 * @code
 * FragmentShader fragmentShader("shaders/fragment.glsl");
 * @endcode
 * @see Shader
 */
class FragmentShader : public Shader {
public:
  /**
   * @brief Constructs a FragmentShader object by loading and compiling a
   * fragment shader from a file.
   *
   * @param path The file path to the fragment shader source code.
   *
   * @throws std::runtime_error If the shader file cannot be opened or
   * compilation fails.
   *
   * @see Shader::Shader(GLenum type, const std::string &path)
   */
  FragmentShader(const std::string &path) : Shader(GL_FRAGMENT_SHADER, path) {}
};

/**
 * @brief Encapsulates an OpenGL shader program composed of a vertex and
 * fragment shader.
 *
 * The ShaderProgram class manages the lifecycle of an OpenGL shader program,
 * including creation, usage, and resource cleanup. It takes ownership of the
 * provided vertex and fragment shaders, links them into a program, and
 * provides utility methods for program usage and uniform location retrieval.
 *
 * @note Copy operations are deleted to prevent accidental resource
 * duplication.
 */
struct ShaderProgram {
private:
  GLuint program = 0;
  std::unique_ptr<VertexShader> vs;
  std::unique_ptr<FragmentShader> fs;

public:
  /**
   * @brief Constructs a ShaderProgram by linking a vertex and fragment shader.
   *
   * This constructor takes ownership of the provided VertexShader and
   * FragmentShader objects, attaches them to a new OpenGL program, and links
   * the program.
   *
   * @param vertexShader A unique pointer to a compiled VertexShader.
   * @param fragmentShader A unique pointer to a compiled FragmentShader.
   *
   * @throws std::runtime_error if the shader program fails to link.
   */
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

  /**
   * @brief Activates the shader program for rendering.
   *
   * This method sets the current OpenGL shader program to the one associated
   * with this Shader instance. All subsequent rendering calls will use this
   * shader program until another program is activated.
   */
  void Use() const
  {
    glUseProgram(program);
  }

  /**
   * @brief Retrieves the location of a uniform variable within the shader
   * program.
   *
   * This function queries the OpenGL shader program for the location of a
   * uniform variable specified by its name. The returned location can be used
   * to set the value of the uniform.
   *
   * @param name The name of the uniform variable whose location is to be
   * retrieved.
   * @return GLint The location of the uniform variable, or -1 if the name does
   * not correspond to an active uniform variable.
   */
  [[nodiscard]] GLint GetUniformLocation(const std::string &name) const
  {
    return glGetUniformLocation(program, name.c_str());
  }
};

