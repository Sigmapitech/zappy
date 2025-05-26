{
  stdenv,
  lib,
  ncurses,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "zappy_server";
  version = "0.0.1";

  src = ../.;

  nativeBuildInputs = [ncurses];

  makeFlags = ["zappy_server"];
  enableParallelBuilding = true;

  installPhase = ''
    runHook preInstall

    install -Dm 577 zappy_server -t $out/bin

    runHook postInstall
  '';

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = "zappy_server";
  };
})
