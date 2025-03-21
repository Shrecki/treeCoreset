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
    python_requires=">=3.8",
    install_requires=[
        "numpy",
    ],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
        "Topic :: Scientific/Engineering",
    ],
)
