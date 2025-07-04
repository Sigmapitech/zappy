#include <random>
#include <string>

/**
 * @brief DJB2 algorithm hashing function
 *
 * @param str
 * @return constexpr unsigned int
 *
 * @note constexpr used for compile-time
 * @note inline for better perf
 */
constexpr unsigned int hash(const char *str)
{
  unsigned int h = 5381;  // Initial hash value
  for (; *str != '\0'; str++)
    h = (h * 33) ^ (unsigned char)*str;
  return h;
}

/**
 * @brief
 *
 * @param str
 * @return constexpr unsigned int
 */
constexpr unsigned int hash(std::string &str)
{
  const char *tmp_str = str.c_str();

  unsigned int h = 5381;  // Initial hash value
  for (; *tmp_str != '\0'; tmp_str++)
    h = (h * 33) ^ (unsigned char)*tmp_str;
  return h;
}

/**
 * @brief Generate a random float number between min and max
 *
 * @param min     Contain the minimum value
 * @param max     Contain the maximum value
 * @return float  Return the random float number
 */
inline float randomFloat(float min, float max)
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(min, max);

  return dis(gen);
}

inline double hashToRange(double x, double btw = 1.0)
{
  double hashed = std::sin(x * 12.9898) * 43758.5453;
  return (hashed - std::floor(hashed)) - btw;
}
