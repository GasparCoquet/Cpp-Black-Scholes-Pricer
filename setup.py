import os
import sys
from pathlib import Path

try:
    from pybind11.setup_helpers import Pybind11Extension, build_ext
except ImportError:
    # Fallback for build isolation issues
    from setuptools import Extension as Pybind11Extension
    from setuptools.command.build_ext import build_ext

from setuptools import setup

__version__ = "1.0.0"

# The main interface is through Pybind11Extension.
ext_modules = [
    Pybind11Extension(
        "black_scholes",
        ["src/bindings.cpp", "src/BlackScholes.cpp"],
        include_dirs=["src"],
        language="c++",
        extra_compile_args=["/std:c++17"] if sys.platform == "win32" else ["-std=c++17"],
    ),
]

setup(
    name="black_scholes",
    version=__version__,
    author="Gaspar",
    author_email="",
    description="High-performance Black-Scholes option pricing library",
    long_description=Path("README.md").read_text(encoding="utf-8") if Path("README.md").exists() else "",
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
    setup_requires=["pybind11>=2.6.0"],
    install_requires=["pybind11>=2.6.0"],
)
