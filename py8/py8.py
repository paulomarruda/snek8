import sys
from py8_app import Py8App

def main() -> None:
    app = Py8App(sys.argv)
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
