{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    pre-commit-hooks = {
      url = "github:cachix/git-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    self,
    nixpkgs,
    pre-commit-hooks,
  }: let
    forAllSystems = function:
      nixpkgs.lib.genAttrs [
        "x86_64-linux"
      ] (system: function nixpkgs.legacyPackages.${system});
  in {
    devShells = forAllSystems (pkgs: {
      default = pkgs.mkShell {
        inherit (self.checks.${pkgs.system}.pre-commit-check) shellHook;

        env.MAKEFLAGS = "-j";
        hardeningDisable = ["fortify"];
        packages = with pkgs;
          [
            clang-tools
            gcc
            compiledb
            gcovr
            hl-log-viewer
          ]
          ++ (with self.packages.${pkgs.system}; [
            align-slashes
            discard-headers
            cpp-fmt
          ]);
      };
    });

    formatter = forAllSystems (pkgs: pkgs.alejandra);

    checks = forAllSystems (pkgs: let
      inherit (pkgs) lib;
      inherit (self.packages.${pkgs.system}) align-slashes discard-headers;

      hooks = {
        alejandra.enable = true;
        clang-format = {
          enable = true;
          types_or = lib.mkForce ["c" "c++"];
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
            ${pkgs.python310.interpreter} ${./scripts/check_commit_message.py}
          '';
        };
      };
    in {
      pre-commit-check = pre-commit-hooks.lib.${pkgs.system}.run {
        inherit hooks;
        src = ./.;
      };
    });

    packages = forAllSystems (pkgs: let
      python-script = name: path:
        pkgs.writers.writePython3Bin name {} (builtins.readFile path);
    in {
      default = self.packages.${pkgs.system}.plazza;

      cpp-fmt = pkgs.writeShellScriptBin "cpp-fmt" ''
        find . -type f -name "*.cpp" -or -name "*.hpp" \
          | xargs clang-format -i --verbose
      '';

      align-slashes =
        python-script "align-slashes" ./scripts/align_columns.py;

      discard-headers =
        python-script "discard-headers" ./scripts/discard_headers.py;
    });
  };
}
