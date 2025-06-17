{
  lib,
  buildPythonPackage,
  hatchling,
}:
buildPythonPackage {
  name = "zappy-ai";
  version = "0.0.1";
  pyproject = true;

  src = ../.;

  build-system = [hatchling];

  pythonImportsCheck = "ai";

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = "zappy_ai";
  };
}
