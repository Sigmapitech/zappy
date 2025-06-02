#include <iostream>

[[gnu::weak]]
auto main() -> int
{
  std::cout << "Hello, client!\n";
}
