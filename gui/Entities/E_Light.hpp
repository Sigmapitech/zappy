#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Demeter/Demeter.hpp"
#include "Demeter/Entity.hpp"

class E_Light : public Dem::IEntity {
private:

public:
  bool Init(Dem::Demeter &) override
  {
    return true;
  }

  bool Update(Dem::Demeter &) override
  {
    return true;
  }

  bool Draw(Dem::Demeter &d) override
  {
    // Draw light effects if needed
    glm::vec3 lightPos(200.0, 500.0, 200.0);  // arbitrary light position
    glm::vec3 viewPos = glm::
      vec3(glm::inverse(d.camera.GetView())[3]);  // camera position

    GLint lightPosLoc = d.GetShader()->GetUniformLocation("lightPos");
    GLint viewPosLoc = d.GetShader()->GetUniformLocation("viewPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

    return true;
  }
};
