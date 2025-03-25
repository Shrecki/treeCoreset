from setuptools import find_packages
from skbuild import setup

setup(
    packages=find_packages(),
    cmake_install_dir="treeCoreset",
    cmake_args=[
        "-DSKBUILD=ON", 
        "-DBUNDLE_DEPENDENCIES=ON",
    ],
)
