"""
treeCoreset package
"""
import os
import sys

# First locate the actual module file
package_dir = os.path.dirname(os.path.abspath(__file__))

# Directly import the module using an absolute import rather than a relative one
sys.path.insert(0, package_dir)  # Add package dir to path if needed

# Import the client_coreset directly (not as a relative import)
# This avoids the circular import issue
import client_coreset

# Export the module
__all__ = ["client_coreset"]

# Make it available in the package namespace
globals()["client_coreset"] = client_coreset