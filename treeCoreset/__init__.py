# Import the compiled extension module
try:
    import client_coreset  # Import all symbols from the extension
    __all__ = ['client_coreset']  # Keep this for backward compatibility
except ImportError as e:
    import sys
    print(f"Error importing C++ extension: {e}", file=sys.stderr)
    raise