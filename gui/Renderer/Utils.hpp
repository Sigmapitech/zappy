// DJB2 algorithm hashing function
// constexpr used for compile-time
constexpr unsigned int Hash(const char *str)
{
  unsigned int h = 5381;  // Initial hash value
  for (; *str != '\0'; str++)
    h = (h * 33) ^ (unsigned char)*str;
  return h;
}
