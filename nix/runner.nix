{
  lib,
  buildPythonPackage,
  hatchling,
}:
buildPythonPackage {
  name = "zappy";
  version = "0.0.1";
  pyproject = true;

  src = ../runner;

  build-system = [hatchling];

  pythonImportsCheck = "zappy";

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = "zappy_ai";
  };
}
