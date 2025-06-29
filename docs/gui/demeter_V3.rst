=======================
DemeterV3 Documentation
=======================

Overview
--------
DemeterV3 is a C++ entity-based game engine designed for high-performance 2D and 3D applications. It features a custom rendering engine built with SDL2 and OpenGL, supporting the loading and rendering of JPEG/PNG textures and Wavefront `.obj` 3D model files.

Features
--------
* **Entity-Based Architecture**: Core logic is built around entities, each inheriting from a common `IEntity` interface.
* **Rendering Engine**: Utilizes SDL2 for window/context management and OpenGL for rendering. Support includes basic texture mapping and `.obj` mesh rendering.
* **Asset Loading**: Built-in support for loading image files (`.jpg`, `.png`) and 3D object files (`.obj`).
* **ImGUI Integration**: Fully integrated support for Dear ImGui, allowing developers to use the library in the standard way for creating in-game or debug UI.

Entity System
-------------
All game objects are represented as entities. Each entity must inherit from the `IEntity` interface defined in the `Dem` namespace. The interface ensures consistent update and render behavior across all entities.

.. code-block:: cpp

```
namespace Dem {

  struct Demeter;

  class IEntity : public std::enable_shared_from_this<IEntity> {
  public:
    virtual ~IEntity() = default;

    // Updates the entity with context from the Demeter engine
    virtual bool Update(Demeter &d) = 0;

    // Draws the entity using the Demeter engine's rendering context
    virtual bool Draw(Demeter &d) = 0;
  };

} // namespace Dem
```

* **Update(Demeter \&d)**: Updates the entity's state using the given Demeter context. Returns `true` on success.
* **Draw(Demeter \&d)**: Renders the entity using the Demeter context. Returns `true` on success.

Main Loop
---------
The engine executes the following steps during each frame of the main loop:

1. Process input events.
2. Call `Update()` on all registered entities.
3. Clear the screen.
4. Call `Draw()` on all registered entities.
5. Present the rendered frame.

Getting Started
---------------
To begin using DemeterV3:

1. Set up an SDL2 + OpenGL development environment.
2. Include the DemeterV3 engine headers and link against its library.
3. Create your own entities by implementing the `IEntity` interface.
4. Register entities with the engine and start the main loop.

Here is an example of a simple entity implementation:

.. code-block:: cpp

```
class MyEntity : public Dem::IEntity {
public:
    bool Update(Dem::Demeter &d) override {
        // update logic
        return true;
    }

    bool Draw(Dem::Demeter &d) override {
        // draw logic
        return true;
    }
};
```

Starting a Game
---------------
To initialize and start a DemeterV3 game, create an instance of the `Demeter` engine and register your entities as shown:

.. code-block:: cpp

```
Game::Game(Args &params)
{
  std::unique_ptr<SDL2> sdl2 = std::make_unique<SDL2>();
  demeter = std::make_unique<Dem::Demeter>(std::move(sdl2));

  // Register entities
  demeter->AddEntity(std::make_shared<E_Default>(params));
}

void Game::Run()
{
  demeter->Run();
}
```

This setup initializes SDL2, creates a Demeter engine instance, registers entities, and starts the main loop.

Dependencies
---------------
* SDL2
* OpenGL
* glew
* glm
* Dear ImGui (for GUI development)
