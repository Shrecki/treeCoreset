"""
treeCoreset package - a C++ tree-based coreset library with Python bindings
"""
from .client_coreset import *

# Also make the module available as an attribute
import sys as _sys
import os as _os

# Get the directory of this file
_current_dir = _os.path.dirname(_os.path.abspath(__file__))

# Make client_coreset available as an attribute for backward compatibility
try:
    from . import client_coreset
except ImportError:
    # Fall back to absolute import if relative import fails
    _sys.path.insert(0, _current_dir)
    import client_coreset

# Export the module namespace
__all__ = ['client_coreset']