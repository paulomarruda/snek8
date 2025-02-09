import os
import sys
import py8core
from typing import List, Dict, Callable
from PyQt6.QtWidgets import QApplication, QFileDialog
from PyQt6.QtGui import QIcon, QGuiApplication
from PyQt6.QtCore import Qt, QBasicTimer, QTimer
from py8_main_window import Py8MainWindow
from py8_screen import Py8Screen

PARENT_DIR: str = 'py8'

class Py8App(QApplication):
    rom_filepath: str = NotImplemented
    py8_emulator: py8core.Py8Emulator = NotImplemented
    py8_main_win: Py8MainWindow = NotImplemented
    py8_screen: Py8Screen = NotImplemented
    is_paused: bool = NotImplemented
    timer: QTimer = NotImplemented
    fps: int = NotImplemented

    @property
    def STATUS_BAR_DEFAULT(self) -> str:
        return "Please select a ROM file."

    @property
    def STATUS_BAR_RUNNING(self) -> str:
        return f"Now running {self.rom_filepath}"

    @property
    def STATUS_BAR_PAUSED(self) -> str:
        return "Paused."

    @property
    def CPU_KEY_MAP(self) -> Dict[int, Callable]:
        return {
            ## Chip8 keyset
            Qt.Key.Key_1.value: lambda value: self.py8_emulator.setKeyValue(0x1, value),
            Qt.Key.Key_2.value: lambda value: self.py8_emulator.setKeyValue(0x2, value),
            Qt.Key.Key_3.value: lambda value: self.py8_emulator.setKeyValue(0x3, value),
            Qt.Key.Key_C.value: lambda value: self.py8_emulator.setKeyValue(0xC, value),

            Qt.Key.Key_Q.value: lambda value: self.py8_emulator.setKeyValue(0x4, value),
            Qt.Key.Key_W.value: lambda value: self.py8_emulator.setKeyValue(0x5, value),
            Qt.Key.Key_E.value: lambda value: self.py8_emulator.setKeyValue(0x6, value),
            Qt.Key.Key_R.value: lambda value: self.py8_emulator.setKeyValue(0xD, value),

            Qt.Key.Key_A.value: lambda value: self.py8_emulator.setKeyValue(0x7, value),
            Qt.Key.Key_S.value: lambda value: self.py8_emulator.setKeyValue(0x8, value),
            Qt.Key.Key_D.value: lambda value: self.py8_emulator.setKeyValue(0x9, value),
            Qt.Key.Key_F.value: lambda value: self.py8_emulator.setKeyValue(0xE, value),

            Qt.Key.Key_Y.value: lambda value: self.py8_emulator.setKeyValue(0xA, value),
            Qt.Key.Key_X.value: lambda value: self.py8_emulator.setKeyValue(0x0, value),
            Qt.Key.Key_C.value: lambda value: self.py8_emulator.setKeyValue(0xB, value),
            Qt.Key.Key_V.value: lambda value: self.py8_emulator.setKeyValue(0xF, value),
        }
    @property
    def APP_KEY_MAP(self) -> Dict[int, Callable]:
        return {
            Qt.Key.Key_P.value: lambda: self.pause,
            Qt.Key.Key_Escape.value: lambda: self.py8_main_win.close
        }

    @property
    def WIDTH(self) -> int:
        return 640

    @property
    def HEIGHT(self) -> int:
        return 360
    
    def __init__(self, argv: List[str]) -> None:
        super(Py8App, self).__init__(argv)
        self.initCore()
        self.initUI()
        self.py8_main_win.show()
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.emulate)
        self.timer.start(1000 // self.fps)

    def initUI(self) -> None:
        self.py8_main_win = Py8MainWindow("Py8 Chip8 Emulator",
                                      self.WIDTH,
                                      self.HEIGHT
        )
        self.py8_main_win.centerWindowOnScreen(QGuiApplication.primaryScreen().geometry().width(),
                                           QGuiApplication.primaryScreen().geometry().height())
        self.py8_main_win.setKeys(self.CPU_KEY_MAP, self.APP_KEY_MAP)
        self.py8_screen = Py8Screen(self.py8_main_win)
        self.py8_main_win.setCentralWidget(self.py8_screen)
        self.setStatusBarDefualt()
        self.initMenus()

    def initCore(self) -> None:
        self.timer = QTimer()
        self.is_paused = False
        self.fps = 60
        self.py8_emulator = py8core.Py8Emulator(implm = py8core.IMPL_MODE_COSMAC_VIP)

    def initMenus(self) -> None:
        # File menu
        self.py8_main_win.addMenu("File")
        self.py8_main_win.addMenuItem("File", "Load Rom", self.loadRom)
        self.py8_main_win.addMenuItem("File", "Quit", self.py8_main_win.close)
        # Options menu
        self.py8_main_win.addMenu("Options")
        self.py8_main_win.addMenuItem("Options", "Reset", self.resetEmulation)
        self.py8_main_win.addMenuItem("Options", "Pause", self.pause)
        self.py8_main_win.addMenuItem("Options", "Save state", self.saveState)
        # Implementation
        self.py8_main_win.addMenu("Implementation")
        self.py8_main_win.addCheckMenu('Implementation', "COSMAC VIP", True, self.setImplmModeCOSMAC)
        self.py8_main_win.addCheckMenu('Implementation', "Modern", False, self.setImplmModeMODERN)
        # Help Menu
        self.py8_main_win.addMenu("Help")
        self.py8_main_win.addMenuItem("Help", "Key mappings", self.showKeyBoardMap)
        self.py8_main_win.addMenuItem("Help", "About", self.showAbout)

    def showError(self, err_msg: str) -> None:
        pass

    def handleSound(self, sound_timer: int) -> None:
        pass

    def setStatusBarPaused(self) -> None:
        self.py8_main_win.status_bar.setText(self.STATUS_BAR_PAUSED)

    def setStatusBarRunning(self) -> None:
        self.py8_main_win.status_bar.setText(self.STATUS_BAR_RUNNING)

    def setStatusBarDefualt(self) -> None:
        self.py8_main_win.status_bar.setText(self.STATUS_BAR_DEFAULT)


    def pause(self) -> None:
        if self.py8_emulator.is_running:
            self.is_paused = False if self.is_paused else True
            if self.is_paused:
                self.setStatusBarPaused()
            else:
                self.setStatusBarRunning()

    def loadRom(self) -> None:
        self.rom_filepath = QFileDialog.getOpenFileName(
            parent = self.py8_main_win,
            caption = "Select a ROM file",
            directory = os.getcwd()
        )[0]
        if os.path.isfile(self.rom_filepath):
            out: int = self.py8_emulator.loadRom(self.rom_filepath)
            match out:
                case py8core.EXECOUT_SUCCESS:
                    self.setStatusBarRunning()
                    pass
                case py8core.EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM:
                    pass
                case py8core.EXECOUT_ROM_FILE_FAILED_TO_OPEN:
                    pass
                case py8core.EXECOUT_ROM_FILE_FAILED_TO_READ:
                    pass
                case py8core.EXECOUT_ROM_FILE_NOT_FOUND:
                    pass
                case _:
                    pass

    def setImplmModeCOSMAC(self) -> None:
        self.py8_emulator.setMode(py8core.IMPL_MODE_COSMAC_VIP)
        self.py8_main_win.checkable_actions['Modern'].setChecked(False)

    def setImplmModeMODERN(self) -> None:
        self.py8_emulator.setMode(py8core.IMPL_MODE_MODERN)
        self.py8_main_win.checkable_actions['COSMAC VIP'].setChecked(False)

    def emulate(self) -> None:
        if self.is_paused or (not self.py8_emulator.is_running):
            return
        out: int = self.py8_emulator.emulationStep()
        match out:
            case py8core.EXECOUT_SUCCESS:
                pass
            case py8core.EXECOUT_EMPTY_STRUCT:
                pass
            case py8core.EXECOUT_INVALID_OPCODE:
                pass
            case py8core.EXECOUT_STACK_EMPTY:
                pass
            case py8core.EXECOUT_STACK_OVERFLOW:
                pass
            case _:
                pass
        self.handleSound(self.py8_emulator.getST())
        self.py8_screen.updateScreen(self.py8_emulator.getGraphics())
        # QTimer.singleShot(1, self.emulate)

    def resetEmulation(self) -> None:
        pass

    def saveState(self) -> None:
        pass

    def showKeyBoardMap(self) -> None:
        pass

    def showAbout(self) -> None:
        pass
