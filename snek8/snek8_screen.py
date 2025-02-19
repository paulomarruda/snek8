"""
@file: py8_screen.py
@author: Paulo Arruda
@license: GPL-3
@brief: Implementation of the emulator' display.
"""
from typing import List, Annotated
from snek8.core import SIZE_GRAPHICS_WIDTH, SIZE_GRAPHICS_HEIGHT, SIZE_GRAPHICS
from PyQt6.QtGui import QColor, QPainter, QPaintEvent
from PyQt6.QtWidgets import QWidget, QFrame

class Snek8Screen(QFrame):
    """
    The app's display responsible to generate the CHIP8's screen.
    
    Parameters
    ----------
    parent: QWidget
        The QT widget that controls the screen. In our implementation, this
        would be the main window.

    Attributes
    ----------
    snek8_screen: Annotaded[List[int], SIZE_GRAPHICS]
        The array representation of CHIP8's screen.
    COLOUR_BCKG: QColor
        The background color to display.
    COLOUR_FRGR: QColor
        The pixel colour
    SIZE_PIXEL: int
        The scale factor to display CHIP8's pixel. Each CHIP8 pixel is represented
        in the app as a SIZE_PIXEL x SIZE_PIXEL square.
    """
    snek8_screen: Annotated[List[int], SIZE_GRAPHICS] = NotImplemented

    def __init__(self, parent: QWidget) -> None:
        super().__init__(parent)
        self.clearScreen()

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
        """
        Clear the app screen.

        This function does not change the emulator screen.
        """
        self.snek8_screen = [False for _ in range(SIZE_GRAPHICS)]
        self.update()

    def updateScreen(self, screen: Annotated[List[int], SIZE_GRAPHICS]) -> None:
        """
        Set the array representation of the CHIP8's pixels.
        """
        self.py8_screen = screen
        self.update()

    def paintEvent(self, a0: QPaintEvent | None) -> None:
        """
        Draw the screen.
        """
        _ = a0
        painter = QPainter(self)
        painter.eraseRect(0, 0, SIZE_GRAPHICS_WIDTH, SIZE_GRAPHICS_HEIGHT)
        for y in range(SIZE_GRAPHICS_HEIGHT):
            for x in range(SIZE_GRAPHICS_WIDTH):
                if self.snek8_screen[y * SIZE_GRAPHICS_WIDTH + x]:
                    colour = self.COLOUR_FRGR
                else:
                    colour = self.COLOUR_BCKG
                painter.fillRect(x * self.SIZE_PIXEL,
                                  y * self.SIZE_PIXEL,
                                  self.SIZE_PIXEL,
                                  self.SIZE_PIXEL,
                                  colour)
