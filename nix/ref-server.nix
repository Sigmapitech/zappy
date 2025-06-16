{stdenvNoCC}:
stdenvNoCC.mkDerivation {
  pname = "ref-server";
  version = "3.0.1";

  src = ../zappy_ref.tgz;
  sourceRoot = "linux";

  dontBuild = true;

  postInstall = ''
    mkdir -p $out/bin
    install -Dm 577 ./zappy_server $out/bin/ref-server
  '';

  meta.mainProgram = "ref-server";
}
