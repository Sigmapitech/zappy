#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace Dem {

  class IScene {
  private:
    // Add private members here if needed

  public:
    ~IScene() = default;

    // Add public methods here if needed
    virtual void Update() = 0;
    virtual void Draw() = 0;
  };

  struct SceneManager {
  public:
    using SceneConstructFunc = std::function<std::unique_ptr<IScene>()>;

  private:
    std::map<std::string, SceneConstructFunc> _scenesPool;
    std::unique_ptr<IScene> _scenesLoaded;

  public:
    SceneManager() = default;
    ~SceneManager() = default;

    void AddScene(const std::string &name, SceneConstructFunc &func)
    {
      _scenesPool[name] = func;
    }

    void DeleteScene(const std::string &name)
    {
      _scenesPool.erase(name);
    }

    std::unique_ptr<IScene> LoadScene(const std::string &name);
  };

}  // namespace Dem

