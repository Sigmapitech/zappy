{
  stdenv,
  lib,
  ncurses,
  debugServer ? false,
}:
stdenv.mkDerivation (finalAttrs: let
  srv-bin-name =
    if debugServer
    then "debug_server"
    else "zappy_server";
in {
  pname = "zappy-server";
  version = "0.0.1";

  src = ../.;

  nativeBuildInputs = [ncurses];

  makeFlags = [srv-bin-name];

  enableParallelBuilding = true;

  installPhase = ''
    runHook preInstall

    install -Dm 577 ${srv-bin-name} -t $out/bin

    runHook postInstall
  '';

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = srv-bin-name;
  };
})
