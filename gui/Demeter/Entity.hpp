#include <cstddef>
#include <cstdint>
#include <string>

namespace Dem {

  class Demeter;

  enum Priority : uint8_t {
    ENT_PRIORITY_LOW,
    ENT_PRIORITY_MEDIUM,
    ENT_PRIORITY_HIGH
  };

  class IEntity {
  protected:
    const std::string _name;
    void *_data;
    size_t _id;
    size_t _scene;
    Priority _priority;

  public:
    virtual ~IEntity() = default;

    virtual bool Update(Demeter &d, size_t id) = 0;
    virtual bool Draw(Demeter &d, size_t id) = 0;
  };

}  // namespace Dem
