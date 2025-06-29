#pragma once

#include <memory>

namespace Dem {

  struct Demeter;

  /**
   * @brief Interface for all entities in the Demeter system.
   *
   * This abstract class defines the basic interface that all entities must
   * implement, including update and draw operations. It inherits from
   * std::enable_shared_from_this to allow entities to create shared_ptr
   * instances to themselves.
   */
  class IEntity : public std::enable_shared_from_this<IEntity> {
  public:
    virtual ~IEntity() = default;

    /**
     * @brief Updates the state of the entity.
     *
     * This pure virtual function must be implemented by derived classes to
     * handle the update of the entity's state based on the current state of
     * the Demeter instance.
     *
     * @param d Reference to the Demeter instance providing context for the
     * update.
     * @return true if the update was successful, false otherwise.
     */
    virtual bool Update(Demeter &d) = 0;

    /**
     * @brief Draws the entity using the provided Demeter context.
     *
     * This pure virtual function must be implemented by derived classes to
     * handle the rendering or visualization of the entity within the given
     * Demeter instance.
     *
     * @param d Reference to a Demeter object used for drawing operations.
     * @return true if the drawing operation was successful, false otherwise.
     */
    virtual bool Draw(Demeter &d) = 0;
  };

}  // namespace Dem
