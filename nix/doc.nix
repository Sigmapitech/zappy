{
  stdenv,
  doxygen,
  python313,
  exhale,
  lib,
  graphviz,
  writableTmpDirAsHomeHook,
  ncurses,
}:
stdenv.mkDerivation {
  name = "doc";
  src = ../.;

  buildInputs = [doxygen];
  nativeBuildInputs = with python313.pkgs; [
    linkify-it-py
    myst-parser
    sphinx
    furo
    sphinx-copybutton
    sphinx-design
    sphinx-notfound-page
    sphinx-sitemap
    breathe
    exhale
    writableTmpDirAsHomeHook
    ncurses
  ];

  preBuild = ''
    substituteInPlace Doxyfile \
      --replace-fail "DOT_PATH = dot" "DOT_PATH = ${lib.getExe' graphviz "dot"}"
  '';

  makeFlags = ["html-doc"];

  installPhase = ''
    mkdir -p $out/
    cp -R docs/build/html/* $out/
  '';

  meta.maintainers = with lib.maintainers; [sigmanificient];
}
