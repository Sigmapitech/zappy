import os
import sys
import pathlib

sys.path.insert(0, os.path.abspath('../'))

project = 'ZAPPY'
author = 'Team Hermitcraft'
release = '1.0'
extensions = [
    'breathe',
    'sphinx.ext.autodoc',
    'sphinx.ext.napoleon',
    'sphinx.ext.viewcode',
    "myst_parser",
    "exhale",
]
html_theme = 'furo'

ZAPPY_ROOT = pathlib.Path(__file__).parent.parent
breathe_projects = {"Zappy": f"{ZAPPY_ROOT}/.build/doxygen/xml"}
breathe_default_project = "Zappy"

exclude_patterns = []
templates_path = ['_templates']
html_static_path = ['_static']
html_extra_path = ['assets']
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

exhale_args = {
    "containmentFolder": "./api",
    "rootFileName": "api.rst",
    "rootFileTitle": "API Documentation",
    "doxygenStripFromPath": "..",
    "createTreeView": True,
}
