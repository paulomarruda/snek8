from typing import Callable, List, Dict
from py8core import IMPL_MODE_COSMAC_VIP, IMPL_MODE_MODERN
from py8_screen import Py8Screen
from PyQt6.QtWidgets import QMenu, QMenuBar, QMainWindow, QLabel, QFileDialog
from PyQt6.QtGui import QIcon, QAction, QKeyEvent


class Py8MainWindow(QMainWindow):
    """
    Parameters
    ----------
    Attributes
    ----------
    """
    menu_toolbar: QMenuBar = NotImplemented
    status_bar: QLabel
    win_menus: Dict[str, QMenu] = NotImplemented
    checkable_actions: Dict[str, QAction] = NotImplemented
    cpu_key_map: Dict[int, Callable] = NotImplemented
    app_key_map: Dict[int, Callable] = NotImplemented

    def __init__(self, title: str, width: int, height: int) -> None:
        super(Py8MainWindow, self).__init__()
        self.initUI(title, width, height)
        self.initCore()

    def centerWindowOnScreen(self, screen_w: int, screen_h: int) -> None:
        move_w = int((screen_w / 2) - (self.frameSize().width() / 2))
        move_h = int((screen_h / 2) - (self.frameSize().height() / 2))
        self.move(move_w, move_h)

    def initCore(self) -> None:
        self.status_bar = QLabel()
        self.statusBar().addWidget(self.status_bar, 1)
        self.menu_toolbar = self.menuBar()
        self.win_menus = {}
        self.checkable_actions = {}

    def initUI(self, title: str, width: int, height: int) -> None:
        self.setGeometry(0, 0, width, height)
        self.setFixedSize(width, height)
        self.setWindowTitle(title)
        # self.setWindowIcon(icon)
        pass

    def addMenu(self, menu_name: str) -> None:
        self.win_menus[menu_name] = self.menu_toolbar.addMenu("&" + menu_name)

    def addMenuItem(self, menu_name: str, menu_item_name: str, event_fun: Callable) -> None:
        if menu_name in self.win_menus:
            action = QAction("&" + menu_item_name, parent = self)
            action.triggered.connect(event_fun)
            self.win_menus[menu_name].addAction(action)

    def addCheckMenu(self, menu_name: str, menu_item_name: str, is_checked: bool,
                     event_fun: Callable) -> None:
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
        self.cpu_key_map = cpu_key_map
        self.app_key_map = app_key_map

    def setStatusBarText(self, text: str) -> None:
        self.status_bar.setText(text)

    def keyPressEvent(self, event: QKeyEvent) -> None:
        key = event.key()
        if key in self.cpu_key_map:
            action = self.cpu_key_map[key]
            action(True)
        elif key in self.app_key_map:
            action = self.app_key_map[key]
            action()

    def keyReleaseEvent(self, event: QKeyEvent) -> None:
        key = event.key()
        if key in self.cpu_key_map:
            action = self.cpu_key_map[key]
            action(False)
