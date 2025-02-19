import sys
from snek8_app import Snek8App

def main() -> None:
    app = Snek8App(sys.argv)
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
