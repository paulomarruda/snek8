from setuptools import setup, Extension, find_packages
import os
import sys

PARENT_DIR: str = 'snek8/'

def read(filename: str) -> str:
    return open(os.path.join(os.path.dirname(__file__), filename)).read()

if os.name != 'nt':
    if sys.platform == 'darwin' and 'APPVEYOR' in os.environ:
        os.environ['CC'] = 'gcc-8'
    py8_module = Extension(
        name = 'snek8.core',
        sources = [
            os.path.join(PARENT_DIR, 'core/src/cpu.c'),
            os.path.join(PARENT_DIR, 'core/src/core.c'),
        ],
        include_dirs = [
            os.path.join(PARENT_DIR, 'core/include/'),
        ],
        language = 'c',
        extra_compile_args = [
            '-Wall',
            '-Wextra',
            '-Werror',
            '-Wfloat-equal',
            '-Wpedantic',
            # '-O3',
            '-g3',
            '-ggdb'
        ]
    )
else:
    py8_module = Extension(
        name = "snek8.core",
        sources = [
            os.path.join(PARENT_DIR, 'core/src/cpu.c'),
            os.path.join(PARENT_DIR, 'core/src/emulator.c'),
        ],
        include_dirs = os.path.join(PARENT_DIR, 'core/include/'),
        language = 'c',
        extra_compile_args = [
            '/Wall',
            '/Wextra',
            '/Werror',
            '/Wfloat-equal',
            '/Wpedantic',
            '/O3',
        ],
    )

setup(
    name = 'snek8',
    version = '0.1',
    author = 'Paulo Arruda',
    author_email = 'paulo.arruda@alumni.usp.br',
    description = 'Snek8 - A Chip8 Emulator written in Python.',
    long_description = read('README.md'),
    long_description_content_type='text/markdown',
    license = 'MIT',
    url = 'https://github.com/paulomarruda/py8/',
    ext_modules = [py8_module],
    packages = find_packages(include=[PARENT_DIR], exclude=["snek8.core"]),
    py_modules=['snek8.__init__'],
    python_requires = '>=3.13',

)
