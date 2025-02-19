"""
@file: py8_main_window.py
@author: Paulo Arruda
@license: GPL-3
@brief: Implementation of the emulator' main window.
"""
from typing import Callable, Dict
from PyQt6.QtWidgets import QMenu, QMenuBar, QMainWindow, QLabel
from PyQt6.QtGui import QAction, QKeyEvent
from PyQt6.QtCore import Qt


class Snek8MainWindow(QMainWindow):
    """
    The app's main window. It is responsible to generate and control the menus
    and the status bar of the app, as well as determine the window geometry of
    the app.

    Parameters
    ----------
    width: int
        The main window's width.
    height: int
        The main window's height.
    title: str
        The main window's title.

    Attributes
    ----------
    menu_toolbar: QmenuBar
        The window dropdown menus.
    status_bar: QLabel
        The status bar reposible for displaying the current status of the emulation.
    win_menus: Dict[str, QMenu]
        Contains each of the window menu stored by their respective names.
    checkable_actions: Dict[str, Qaction]
        Contains the checkable options of a given menu.
    cpu_key_map: Dict[int, Callable]
        Contains the mapping of CHIP8's key and the respective function that
        loads the pressed/released key into the emulator.
    app_key_map: Dict[int, callable]
        Contains the mapping of the app's key mapping and the respective function
        that performs the selected action for each pressed key.
    """
    menu_toolbar: QMenuBar = NotImplemented
    status_bar: QLabel
    win_menus: Dict[str, QMenu] = NotImplemented
    checkable_actions: Dict[str, QAction] = NotImplemented
    cpu_key_map: Dict[int, Callable] = NotImplemented
    app_key_map: Dict[int, Callable] = NotImplemented

    def __init__(self, title: str, width: int, height: int) -> None:
        super(Snek8MainWindow, self).__init__()
        self.initUI(title, width, height)
        self.initCore()

    def centerWindowOnScreen(self, screen_w: int, screen_h: int) -> None:
        """
        Given the computer screen geometry, set the position of the
        main window in the middle of the screen.
        """
        move_w = int((screen_w / 2) - (self.frameSize().width() / 2))
        move_h = int((screen_h / 2) - (self.frameSize().height() / 2))
        self.move(move_w, move_h)

    def initCore(self) -> None:
        """
        Init the objects respondible for the correct functioning of the main
        window.
        """
        self.status_bar = QLabel()
        self.statusBar().addWidget(self.status_bar, 1)
        self.menu_toolbar = self.menuBar()
        self.win_menus = {}
        self.checkable_actions = {}

    def initUI(self, title: str, width: int, height: int) -> None:
        """
        Init the correct displaying of the main window.
        """
        self.setGeometry(0, 0, width, height)
        self.setFixedSize(width, height)
        self.setWindowTitle(title)
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        # self.setWindowIcon(icon)
        pass

    def addMenu(self, menu_name: str) -> None:
        """
        Add a menu to the menu toolbar and saves the menu in the `win_menus`
        dictionary.
        """
        self.win_menus[menu_name] = self.menu_toolbar.addMenu("&" + menu_name)

    def addMenuItem(self, menu_name: str, menu_item_name: str, event_fun: Callable) -> None:
        """
        Add a particular option to a given menu.
        """
        if menu_name in self.win_menus:
            action = QAction("&" + menu_item_name, parent = self)
            action.triggered.connect(event_fun)
            self.win_menus[menu_name].addAction(action)

    def addCheckMenu(self, menu_name: str, menu_item_name: str, is_checked: bool,
                     event_fun: Callable) -> None:
        """
        Add a checkable option to a given menu.
        """
        if menu_name in self.win_menus:
            menu_item: QAction = QAction(
                "&" + menu_item_name,
                parent = self,
                checkable = True
            )
            menu_item.setChecked(is_checked)
            self.checkable_actions[menu_item_name] = menu_item
            menu_item.triggered.connect(event_fun)
            self.win_menus[menu_name].addAction(menu_item)

    def setKeys(self, cpu_key_map: Dict[int, Callable], app_key_map: Dict[int, Callable]) -> None:
        """
        Load the key bindings.
        """
        self.cpu_key_map = cpu_key_map.copy()
        self.app_key_map = app_key_map.copy()

    def setStatusBarText(self, text: str) -> None:
        """
        Set the the text of the status bar.
        """
        self.status_bar.setText(text)

    def keyPressEvent(self, a0: QKeyEvent | None) -> None:
        """
        Listen to a key press and execute the respective action associated with the key.
        """
        key = a0.key()
        if key in self.cpu_key_map:
            self.cpu_key_map[key][0]()
        elif key in self.app_key_map:
            self.app_key_map[key]()

    def keyReleaseEvent(self, a0: QKeyEvent | None) -> None:
        """
        Listen to a key release and execute the respective action associated with the key.
        """
        key = a0.key()
        if key in self.cpu_key_map:
            self.cpu_key_map[key][1]()
