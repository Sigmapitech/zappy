{
  appimageTools,
  nix-update-script,
  stdenvNoCC,
}: let
  version = "3.0.0";

  src = stdenvNoCC.mkDerivation {
    pname = "ref-gui-src";
    inherit version;

    src = ../zappy_ref.tgz;
    sourceRoot = "linux";

    dontBuild = true;

    postInstall = ''
      install -Dm 644 ./zappy_gui.AppImage $out
    '';
  };
in
  appimageTools.wrapType2 {
    pname = "ref-gui";

    inherit version src;

    passthru.updateScript = nix-update-script {
      extraArgs = ["--version-regex=v([\\d.]+)"];
    };
  }
