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

  # do not generate bytecode in the pythonImportsCheckPhase phase
  env.PYTHONDONTWRITEBYTECODE = 1;

  postInstallCheck = ''
    find $out -type d -name "__pycache__" -exec rm -rv {} +
  '';

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = "zappy_ai";
  };
}
