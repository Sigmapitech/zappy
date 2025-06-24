{
  stdenv,
  lib,
  ncurses,
  pkg-config,
  SDL2,
  SDL2_image,
  libGL,
  libGLU,
  glew,
  glm,
  imgui,
  libX11,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "zappy-gui";
  version = "0.0.1";

  src = ../.;

  nativeBuildInputs = [
    ncurses
    pkg-config
  ];

  buildInputs = [
    SDL2
    SDL2_image
    libGL
    libGLU
    glew
    glm
    libX11
  ];

  makeFlags = ["zappy_gui"];
  enableParallelBuilding = true;

  env = {
    GUI_ASSET_DIR = "${placeholder "out"}/share/gui/assets";
    IMGUI_DIR = "${imgui.src}";
  };

  installPhase = ''
    runHook preInstall

    install -Dm 577 zappy_gui -t $out/bin

    mkdir -p $out/share/gui
    cp -r assets $out/share/gui/assets

    runHook postInstall
  '';

  meta = {
    maintainers = with lib.maintainers; [sigmanificient];
    mainProgram = "zappy_gui";
  };
})
