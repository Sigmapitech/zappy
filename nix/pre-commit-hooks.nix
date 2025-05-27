{
  self,
  pkgs,
}: let
  inherit (pkgs) lib;

  python-script = name: path:
    pkgs.writers.writePython3Bin name {} (builtins.readFile path);

  align-slashes = python-script "align-slashes" ../scripts/align_columns.py;

  discard-headers = python-script "discard-headers" ../scripts/discard_headers.py;
in {
  alejandra.enable = true;
  clang-format = {
    enable = true;
    name = "format the code";
    entry = lib.getExe self.packages.${pkgs.system}.cpp-fmt;
  };

  align-slashes = {
    enable = true;
    name = "align blackslashes";
    entry = lib.getExe align-slashes;
  };

  discard-headers = {
    enable = true;
    name = "discard headers";
    entry = lib.getExe discard-headers;
  };

  trim-trailing-whitespace.enable = true;

  commit-name = {
    enable = true;
    name = "commit name";
    stages = ["commit-msg"];
    entry = ''
      ${pkgs.python310.interpreter} ${../scripts/check_commit_message.py}
    '';
  };
}
