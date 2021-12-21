from distutils.core import setup, Extension

module = Extension(
    "_pyhtmldoc",
    ["pyhtmldoc.i", "pyhtmldoc.c"],
    libraries=["htmldoc"],
)

setup(
    ext_modules=[module],
    name="pyhtmldoc",
    py_modules=["pyhtmldoc"],
    url="https://github.com/RekGRpth/pyhtmldoc",
    version='1.0.0',
)
