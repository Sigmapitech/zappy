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
        pyenv = pkgs.python313.withPackages (p:
          with p; [
            pytest
            pytest-cov
          ]);
      in
        pkgs.mkShell {
          inherit (self.checks.${pkgs.system}.pre-commit-check) shellHook;

          env.MAKEFLAGS = "-j";
          hardeningDisable = ["fortify"];
          inputsFrom = with self.packages.${pkgs.system}; [
            ai
            gui
            server
            doc
          ];

          packages =
            (with pkgs; [
              clang-tools
              compiledb
              gcovr
              hl-log-viewer
              doxygen
              graphviz
              pyenv
            ])
            ++ (with self.packages.${pkgs.system}; [
              cpp-fmt
            ]);
        };
    });

    formatter = forAllSystems (pkgs: pkgs.alejandra);

    checks = forAllSystems (
      pkgs: {
        pre-commit-check = pre-commit-hooks.lib.${pkgs.system}.run {
          hooks = import ./nix/pre-commit-hooks.nix {inherit self pkgs;};
          src = ./.;
        };
      }
    );

    packages = forAllSystems (
      pkgs: let
        pypkgs = pkgs.python313.pkgs;
        pkgs' = self.packages.${pkgs.system};
      in
        {
          ai = pypkgs.callPackage ./nix/zappy-ai.nix {};

          gui = pypkgs.callPackage ./nix/zappy-gui.nix {};

          server = pypkgs.callPackage ./nix/zappy-server.nix {};

          default = pkgs.symlinkJoin {
            name = "zappy";
            paths = with pkgs'; [
              server
              gui
              ai
            ];
          };
        }
        // {
          ref-gui = pkgs.callPackage ./nix/ref-gui.nix {};

          ref-server = pkgs.callPackage ./nix/ref-server.nix {};
        }
        // {
          cpp-fmt = pkgs.writeShellScriptBin "cpp-fmt" ''
            find . -type f -name "*.cpp" -or -name "*.hpp" \
              | xargs clang-format -i --verbose
          '';

          exhale = pypkgs.callPackage ./nix/exhale.nix {};

          doc = pkgs.callPackage ./nix/doc.nix {
            inherit (pkgs') exhale;
          };
        }
    );
  };
}
