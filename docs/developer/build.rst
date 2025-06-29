=======================
How to Build Zappy
=======================

This page explains everything you need to install to build and run the
Zappy project. It is structured into sections for:

- Minimal delivery (AI / Server / GUI)
- Runner
- Documentation
- Development (extra)

Where possible, two approaches are described:

- **Using Nix** (recommended for reproducibility)
- **Traditional/manual** install (for portability)

------------------------
Minimal delivery (AI / Server / GUI)
------------------------

These are the core components needed to *run* or *develop* the server, AI,
and GUI separately.

**Using Nix (recommended)**

If you have Nix installed:

.. code-block:: bash

   nix develop

This will enter a shell with all dependencies available for building the server,
AI, and GUI.

**Traditional (manual) install**

You'll need to install these system packages yourself:

* **Common build tools**:
  - bash
  - coreutils
  - gnugrep
  - findutils
  - GNU Make
  - pkg-config

* **Server**:
  - gcc
  - glibc (with math libraries)

* **AI**:
  - Python 3.13 (or compatible) with pip
  - virtualenv or Python's built-in venv module
  - pip packages:
    - setuptools
    - hatching

* **GUI**:
  - g++ (with libstdc++)
  - SDL2
  - SDL2_image
  - glew
  - GL, GLU
  - glm
  - mesa

* **Optional**:
  - ncurses (for pretty Makefile logs)

------------------------
Runner
------------------------

The **runner** tool (to coordinate AI, server, GUI in one command) requires:

**Using Nix (recommended)**

Nix is the only requirement:

.. code-block:: bash

   nix run github:Sigmapitech/zappy/zappy-runner

This will handle all other builds and dependencies automatically.

**Traditional (manual) install**

There is no non-Nix delivery for the runner. If you want to avoid Nix entirely,
you'll have to run the server, AI, and GUI manually in separate terminals,
using the minimal delivery setup above.

------------------------
Documentation
------------------------

To build the Sphinx documentation:

**Using Nix**

All packages are included in the dev shell:

.. code-block:: bash

   nix develop
   make -C docs html

**Traditional install**

You’ll need:

* **System**
  - doxygen
  - libxml

* **Python packages (install via pip)**
  - sphinx
  - furo
  - breathe
  - exhale
  - beautifulsoup4
  - six
  - linkify-it-py
  - myst-parser
  - sphinx-copybutton
  - sphinx-design
  - sphinx-notfound-page
  - sphinx-sitemap

Example:

.. code-block:: bash

   pip install sphinx furo breathe exhale beautifulsoup4 six linkify-it-py myst-parser \
       sphinx-copybutton sphinx-design sphinx-notfound-page sphinx-sitemap

Then:

.. code-block:: bash

   doxygen
   make -C docs html

------------------------
Development (extra)
------------------------

Additional tools recommended for testing, coverage, linting, and advanced debugging.

**Using Nix**

These are included in the dev shell:

.. code-block:: bash

   nix develop

**Traditional install**

You can install these manually via your system package manager or pip.

* **Testing and coverage**
  - gcovr
  - libcoverage
  - pytest

* **Static analysis and formatting**
  - compiledb
  - clang-tools
  - black
  - mypy
  - isort
  - vulture

* **Logging & profiling**
  - hl-log-viewer
  - valgrind
  - kcachegrind
  - graphviz

------------------------
Recommended approach
------------------------

We strongly recommend using Nix for development:

.. code-block:: bash

   nix develop

This ensures:

- Reproducible builds
- All dependencies present
- Correct versions matched to the project

You can build/run each package from their nix derivation, following the flake schema:

... code-block:: bash

   └───packages
    └───x86_64-linux
        ├───ai: package 'python3.13-zappy-ai'
        ├───bleach: package 'bleach-0.0.1'
        ├───cpp-fmt: package 'cpp-fmt'
        ├───default: package 'zappy'
        ├───doc: package 'doc'
        ├───exhale: package 'python3.13-exhale-0.3.7'
        ├───gui: package 'zappy-gui-0.0.1'
        ├───ref: package 'ref'
        ├───ref-gui: package 'ref-gui-3.0.0'
        ├───ref-server: package 'ref-server-3.0.1'
        ├───runner: package 'python3.13-zappy'
        ├───server: package 'zappy-server-0.0.1'
        └───server-debug: package 'zappy-server-0.0.1'

Note: it is recommended to derectly build/run the final derivation (default),
that contains our delivery and reference binaries, alongside the zappy-runner ...
see instructions on how to use the zappy-runner

If you prefer manual installs, make sure to match the versions in your package
manager as closely as possible to avoid compatibility issues.
