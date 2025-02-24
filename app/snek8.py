"""
@file snek8.py
@author Paulo Arruda
@license GPL-3
@brief Main.
"""
import sys
from app import Snek8App

def main() -> None:
    app = Snek8App(sys.argv)
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
