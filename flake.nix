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
            black
            mypy
            isort
            vulture

            pytest
            pytest-cov
          ]);
      in
        pkgs.mkShell {
          inherit (self.checks.${pkgs.system}.pre-commit-check) shellHook;

          env = {
            MAKEFLAGS = "-j";
            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath (with pkgs; [
              mesa
            ]);
          };

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
              pkg-config
              SDL2
              SDL2_image
              libGL
              libGLU
              glew
              glm
              doxygen
              graphviz
              pyenv
              valgrind
              libsForQt5.kcachegrind
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

          server-debug = pkgs'.server.override {debugServer = true;};

          runner = pypkgs.callPackage ./nix/runner.nix {
            inherit (pkgs') bleach;
          };

          default = pkgs.symlinkJoin {
            name = "zappy";
            paths = with pkgs'; [
              runner
              server
              gui
              ai
              ref
            ];

            meta.mainProgram = "zappy";
          };

          bleach = pkgs.callPackage ./nix/bleach.nix {};
        }
        // {
          ref-gui = pkgs.callPackage ./nix/ref-gui.nix {};

          ref-server = pkgs.callPackage ./nix/ref-server.nix {};

          ref = pkgs.symlinkJoin {
            name = "ref";
            paths = with pkgs'; [
              ref-gui
              ref-server
            ];
          };
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
