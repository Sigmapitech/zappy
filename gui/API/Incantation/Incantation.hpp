#include <string>
#include <vector>

class Incantation {
private:
  std::vector<int> _member;
  int _level;
  std::pair<int, int> _position;
  std::string _state;

public:
  /**
   * @brief Construct a new Incantation object
   *
   * @param level  Contain the level of the incantation
   * @param x      Contain the position in x of the incantation
   * @param y      Contain the position in y of the incantation
   */
  Incantation(int level, int x, int y);
  ~Incantation() = default;

  /**
   * @brief Add a member of to the incantation
   *
   * @param id  Contain the id of the incantation
   */
  void AddMember(int id);

  /**
   * @brief Get the position of the incantation
   *
   * @return std::pair<int, int>
   */
  std::pair<int, int> GetPosition();

  /**
   * @brief Set the state of the incantation
   *
   * @param state Contain the new state of the incantation
   */
  void SetState(std::string &state);
};
