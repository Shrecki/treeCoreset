"""
treeCoreset package
"""
import os
import sys
import importlib.util

# Add the package directory to sys.path if not already there
package_dir = os.path.dirname(os.path.abspath(__file__))
if package_dir not in sys.path:
    sys.path.insert(0, package_dir)

# Try first to import as a direct module
try:
    # Attempt to import the C++ extension module
    from . import client_coreset
except ImportError:
    # If that fails, try to find it manually
    try:
        # Look for the module file in the package directory
        module_paths = [
            os.path.join(package_dir, "client_coreset.so"),  # Linux/Mac
            os.path.join(package_dir, "client_coreset.pyd"),  # Windows
        ]

        # Find the first one that exists
        module_path = next((path for path in module_paths if os.path.exists(path)), None)

        if module_path:
            # Load the module manually
            spec = importlib.util.spec_from_file_location("client_coreset", module_path)
            client_coreset = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(client_coreset)
            sys.modules["treeCoreset.client_coreset"] = client_coreset
            # Make it available in the treeCoreset namespace
            globals()["client_coreset"] = client_coreset
        else:
            raise ImportError(f"Could not find client_coreset module in {package_dir}")
    except Exception as e:
        print(f"Error loading C++ extension: {e}", file=sys.stderr)
        print(f"Files in package directory: {os.listdir(package_dir)}", file=sys.stderr)
        raise

# Export the module
__all__ = ["client_coreset"]