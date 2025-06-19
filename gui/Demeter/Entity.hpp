#pragma once

#include <memory>

namespace Dem {

  struct Demeter;

  class IEntity : public std::enable_shared_from_this<IEntity> {
  public:
    virtual ~IEntity() = default;

    virtual bool Update(Demeter &d) = 0;
    virtual bool Draw(Demeter &d) = 0;
  };

}  // namespace Dem
