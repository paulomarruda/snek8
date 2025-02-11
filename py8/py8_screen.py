import os
import sys
from typing import Tuple, List, Dict, Annotated
from py8core import Py8Emulator, SIZE_GRAPHICS_WIDTH, SIZE_GRAPHICS_HEIGHT,\
                    SIZE_GRAPHICS
from PyQt6.QtCore import Qt, QRect
from PyQt6.QtGui import QIcon, QAction, QColor, QKeyEvent, QPainter
from PyQt6.QtWidgets import QApplication, QWidget, QMainWindow, QFileDialog,\
                            QMenuBar, QVBoxLayout, QFrame

class Py8Screen(QFrame):
    """

    Parameters
    ----------

    Attributes
    ----------
    """
    py8_screen: Annotated[List[int], SIZE_GRAPHICS] = NotImplemented


    def __init__(self, parent: QWidget) -> None:
        super().__init__(parent)
        # self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        self.py8_screen = [False for x in range(SIZE_GRAPHICS)]

    @property
    def COLOUR_BCKG(self) -> QColor:
        return QColor(0, 0, 0)

    @property
    def COLOUR_FRGR(self) -> QColor:
        return QColor(78, 154, 6)

    @property
    def SIZE_PIXEL(self) -> int:
        return 10

    def clearScreen(self) -> None:
        self.py8_screen = [False for pix in range(SIZE_GRAPHICS)]
        self.update()

    def updateScreen(self, screen: Annotated[List[int], SIZE_GRAPHICS]) -> None:
        self.py8_screen = screen
        self.update()

    def paintEvent(self, event) -> None:
        painter = QPainter(self)
        painter.eraseRect(0, 0, SIZE_GRAPHICS_WIDTH, SIZE_GRAPHICS_HEIGHT)
        for y in range(SIZE_GRAPHICS_HEIGHT):
            for x in range(SIZE_GRAPHICS_WIDTH):
                if self.py8_screen[y * SIZE_GRAPHICS_WIDTH + x]:
                    colour = self.COLOUR_FRGR
                else:
                    colour = self.COLOUR_BCKG
                painter.fillRect(x * self.SIZE_PIXEL,
                                  y * self.SIZE_PIXEL,
                                  self.SIZE_PIXEL,
                                  self.SIZE_PIXEL,
                                  colour)
