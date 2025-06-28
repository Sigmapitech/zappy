{stdenv}:
stdenv.mkDerivation {
  pname = "bleach";
  version = "0.0.1";

  src = ../bleach.c;
  dontUnpack = true;

  buildPhase = ''
    runHook preBuild

    $CC $src -o bleach

    runHook postBuild
  '';

  postInstall = ''
    mkdir -p $out/bin
    install -Dm 577 ./bleach $out/bin/bleach
  '';

  meta.mainProgram = "bleach";
}
