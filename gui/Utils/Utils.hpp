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

constexpr unsigned int hash(std::string &str)
{
  const char *tmp_str = str.c_str();

  unsigned int h = 5381;  // Initial hash value
  for (; *tmp_str != '\0'; tmp_str++)
    h = (h * 33) ^ (unsigned char)*tmp_str;
  return h;
}
