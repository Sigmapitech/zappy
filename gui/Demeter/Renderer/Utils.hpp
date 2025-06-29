#pragma once

/**
 * @brief Computes a hash value for a given C-style string using the DJB2
 * algorithm.
 *
 * This constexpr function takes a null-terminated string and calculates its
 * hash using a variation of the DJB2 algorithm, which is known for its
 * simplicity and good distribution properties. The hash is initialized to 5381
 * and updated for each character in the string.
 *
 * @param str The input C-style string to hash.
 * @return The computed hash value as an unsigned int.
 */
constexpr unsigned int Hash(const char *str)
{
  unsigned int h = 5381;  // Initial hash value
  for (; *str != '\0'; str++)
    h = (h * 33) ^ (unsigned char)*str;
  return h;
}
