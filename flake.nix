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
      default = let
        cpp-fmt = pkgs.writeShellScriptBin "cpp-fmt" ''
          find . -type f -name "*.cpp" -or -name "*.hpp" \
            | xargs clang-format -i --verbose
        '';
      in
        pkgs.mkShell {
          inherit (self.checks.${pkgs.system}.pre-commit-check) shellHook;

          env.MAKEFLAGS = "-j";
          hardeningDisable = ["fortify"];
          inputsFrom = with self.packages.${pkgs.system}; [
            zappy_ai
            zappy_gui
            zappy_server
          ];

          packages = with pkgs; [
            clang-tools
            compiledb
            gcovr
            hl-log-viewer
            cpp-fmt
          ];
        };
    });

    formatter = forAllSystems (pkgs: pkgs.alejandra);

    checks = forAllSystems (
      pkgs:
        import ./nix/pre-commit-hooks.nix {
          inherit self pkgs pre-commit-hooks;
        }
    );

    packages = forAllSystems (
      pkgs: let
        pypkgs = pkgs.python3.pkgs;
      in {
        zappy_ai = pypkgs.callPackage ./nix/zappy_ai.nix {};

        zappy_gui = pypkgs.callPackage ./nix/zappy_gui.nix {};

        zappy_server = pypkgs.callPackage ./nix/zappy_server.nix {};

        default = pkgs.symlinkJoin {
          name = "zappy";
          paths = with self.packages.${pkgs.system}; [
            zappy_server
            zappy_gui
            zappy_ai
          ];
        };
      }
    );
  };
}
