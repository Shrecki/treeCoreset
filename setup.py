from setuptools import find_packages
from skbuild import setup

setup(
    name="treeCoreset",
    version="0.1.0",
    description="C++ tree-based coreset library with Python bindings",
    author="Fabrice Guibert",
    author_email="fabrice.guibert@epfl.ch", 
    url="https://github.com/Shrecki/treeCoreset",
    packages=find_packages(),
    cmake_install_dir="treeCoreset",
    cmake_args=[
        "-DSKBUILD=ON", 
        "-DBUNDLE_DEPENDENCIES=ON",
    ],
    python_requires=">=3.10",
    install_requires=[
        "numpy",
    ],
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Data scientists",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: C++",
        "Topic :: Scientific/Engineering",
    ],
    license="GPL-3.0",
    readme="README.md"
)
