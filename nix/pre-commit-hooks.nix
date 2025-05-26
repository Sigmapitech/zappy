{
  self,
  pkgs,
  pre-commit-hooks,
}: let
  inherit (pkgs) lib;

  python-script = name: path:
    pkgs.writers.writePython3Bin name {} (builtins.readFile path);

  align-slashes = python-script "align-slashes" ../scripts/align_columns.py;

  discard-headers = python-script "discard-headers" ../scripts/discard_headers.py;

  hooks = {
    alejandra.enable = true;
    clang-format = {
      enable = true;
      types_or = lib.mkForce [
        "c++"
      ];
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
  };
in {
  pre-commit-check = pre-commit-hooks.lib.${pkgs.system}.run {
    inherit hooks;
    src = ./.;
  };
}
