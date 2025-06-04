{
  lib,
  buildPythonPackage,
  setuptools,
}:
buildPythonPackage {
  name = "zappy-ai";
  version = "0.0.1";
  pyproject = true;

  src = ../.;

  build-system = [setuptools];

  doCheck = false; # no tests yet!

  pythonImportsCheck = "ai";

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = "zappy_ai";
  };
}
