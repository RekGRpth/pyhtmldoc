from distutils.core import Extension, setup

setup(
    author_email='RekGRpth@gmail.com',
    author='RekGRpth',
    ext_modules=[Extension(
        libraries=["htmldoc"],
        name="_pyhtmldoc",
        sources=["pyhtmldoc.i", "pyhtmldoc.c"],
    )],
    license='MIT',
    name="pyhtmldoc",
    py_modules=["pyhtmldoc"],
    url="https://github.com/RekGRpth/pyhtmldoc",
    version='1.0.0',
)
