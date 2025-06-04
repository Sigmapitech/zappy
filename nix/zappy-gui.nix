{
  stdenv,
  lib,
  ncurses,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "zappy-gui";
  version = "0.0.1";

  src = ../.;

  nativeBuildInputs = [ncurses];

  makeFlags = ["zappy_gui"];
  enableParallelBuilding = true;

  installPhase = ''
    runHook preInstall

    install -Dm 577 zappy_gui -t $out/bin

    runHook postInstall
  '';

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = "zappy_gui";
  };
})
