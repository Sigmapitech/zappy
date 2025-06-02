import os
import sys
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
breathe_projects = {"Zappy": "/home/julien/delivery/zappy/docs/doxygen/xml"}
breathe_default_project = "Zappy"
templates_path = ['_templates']
exclude_patterns = []
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

