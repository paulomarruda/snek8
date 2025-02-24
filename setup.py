from setuptools import setup, Extension
import os
import sys

PARENT_DIR: str = 'snek8/'

def read(filename: str) -> str:
    return open(os.path.join(os.path.dirname(__file__), filename)).read()

if os.name != 'nt':
    if sys.platform == 'darwin' and 'APPVEYOR' in os.environ:
        os.environ['CC'] = 'gcc-8'
    snek8_core = Extension(
        name = 'snek8.core',
        sources = [
            os.path.join(PARENT_DIR, '_core/src/cpu.c'),
            os.path.join(PARENT_DIR, '_core/src/core.c'),
        ],
        include_dirs = [
            os.path.join(PARENT_DIR, '_core/include/'),
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
    snek8_core = Extension(
        name = "snek8.core",
        sources = [
            os.path.join(PARENT_DIR, '_core/src/cpu.c'),
            os.path.join(PARENT_DIR, '_core/src/emulator.c'),
        ],
        include_dirs = os.path.join(PARENT_DIR, '_core/include/'),
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
    license = 'GPL-3',
    url = 'https://github.com/paulomarruda/snek8/',
    ext_modules = [snek8_core],
    packages = ['snek8'],
    package_dir = {'snek8': PARENT_DIR},
    python_requires = '>=3.13',
    classifiers = [
        "Topic :: Emulator",
        "Topic :: Emulation",
        "Topic :: Chip8",
        "Programming Language :: Python",
        "Programming Language :: C",
    ],
)
