import os
import sys
from snek8 import core as snek8core
from snek8.snek8_main_window import Snek8MainWindow
from snek8.snek8_screen import Snek8Screen
from typing import List, Dict, Callable
from PyQt6.QtWidgets import QApplication, QFileDialog
from PyQt6.QtGui import QIcon, QGuiApplication
from PyQt6.QtCore import Qt, QBasicTimer, QTimer

PARENT_DIR: str = 'snek8'

class Snek8App(QApplication):
    """
    Py8 Emulator.

    Attributes
    ----------

    Parameters
    ----------
    """
    rom_filepath: str = NotImplemented
    snek8_emulator: snek8core.Snek8Emulator = NotImplemented
    snek8_main_win: Snek8MainWindow = NotImplemented
    snek8_screen: Snek8Screen = NotImplemented
    snek8_impl_bnnn_uses_vx: bool = NotImplemented
    snek8_impl_fx_changes_ir: bool = NotImplemented
    snek8_impl_shifts_use_vy: bool = NotImplemented
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
    def CPU_KEY_MAP(self) -> Dict[int, List[Callable]]:
        return {
            ## Chip8 keyset
            Qt.Key.Key_1: [
                lambda: self.py8_emulator.setKeyValue(0x1, True),
                lambda: self.py8_emulator.setKeyValue(0x1, False),
            ],
            Qt.Key.Key_2: [
                lambda: self.py8_emulator.setKeyValue(0x2, True),
                lambda: self.py8_emulator.setKeyValue(0x2, False),
            ],
            Qt.Key.Key_3: [
                lambda: self.py8_emulator.setKeyValue(0x3, True),
                lambda: self.py8_emulator.setKeyValue(0x3, False),
            ],
            Qt.Key.Key_4: [
                lambda: self.py8_emulator.setKeyValue(0xC, True),
                lambda: self.py8_emulator.setKeyValue(0xC, False),
            ],

            Qt.Key.Key_Q: [
                lambda: self.py8_emulator.setKeyValue(0x4, True),
                lambda: self.py8_emulator.setKeyValue(0x4, False),
            ],
            Qt.Key.Key_W: [
                lambda: self.py8_emulator.setKeyValue(0x5, True),
                lambda: self.py8_emulator.setKeyValue(0x5, False),
            ],
            Qt.Key.Key_E: [
                lambda: self.py8_emulator.setKeyValue(0x6, True),
                lambda: self.py8_emulator.setKeyValue(0x6, False),
            ],
            Qt.Key.Key_R: [
                lambda: self.py8_emulator.setKeyValue(0xD, True),
                lambda: self.py8_emulator.setKeyValue(0xD, False),
            ],

            Qt.Key.Key_A: [
                lambda: self.py8_emulator.setKeyValue(0xC, True),
                lambda: self.py8_emulator.setKeyValue(0xC, False),
            ],
            Qt.Key.Key_S: [
                lambda: self.py8_emulator.setKeyValue(0x8, True),
                lambda: self.py8_emulator.setKeyValue(0x8, False),
            ],
            Qt.Key.Key_D: [
                lambda: self.py8_emulator.setKeyValue(0x9, True),
                lambda: self.py8_emulator.setKeyValue(0x9, False),
            ],
            Qt.Key.Key_F: [
                lambda: self.py8_emulator.setKeyValue(0xE, True),
                lambda: self.py8_emulator.setKeyValue(0xE, False),
            ],

            Qt.Key.Key_Y: [
                lambda: self.py8_emulator.setKeyValue(0xA, True),
                lambda: self.py8_emulator.setKeyValue(0xA, False),
            ],
            Qt.Key.Key_X: [
                lambda: self.py8_emulator.setKeyValue(0x0, True),
                lambda: self.py8_emulator.setKeyValue(0x0, False),
            ],
            Qt.Key.Key_C: [
                lambda: self.py8_emulator.setKeyValue(0xB, True),
                lambda: self.py8_emulator.setKeyValue(0xB, False),
            ],
            Qt.Key.Key_V: [
                lambda: self.py8_emulator.setKeyValue(0xF, True),
                lambda: self.py8_emulator.setKeyValue(0xF, False),
            ],
        }
    @property
    def APP_KEY_MAP(self) -> Dict[int, Callable]:
        return {
            Qt.Key.Key_P: lambda: self.pause(),
            Qt.Key.Key_Escape: lambda: self.py8_main_win.close(),
            Qt.Key.Key_L: lambda: self.loadRom(),
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
        self.timer.timeout.connect(self.emulate)
        self.timer.start(500)

    def initUI(self) -> None:
        self.py8_main_win = Snek8MainWindow("Py8 Chip8 Emulator",
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
        self.py8_impl_shifts_use_vy = False
        self.py8_impl_bnnn_uses_vx = False
        self.py8_impl_fx_changes_ir = False
        self.timer = QTimer()
        self.is_paused = False
        self.fps = 60
        self.py8_emulator = snek8core.Snek8Emulator(implm_flags = 0)

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
        self.py8_main_win.addCheckMenu('Implementation', "Shifts use VY", False, self.implmModeShifts)
        self.py8_main_win.addCheckMenu('Implementation', "BNNN uses VX", False, self.implmModeBNNN)
        self.py8_main_win.addCheckMenu('Implementation', "FX changes I", False, self.implmModeFX)
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
        if self.py8_emulator.is_running:
            self.resetEmulation()
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

    def implmModeShifts(self) -> None:
        if self.py8_main_win.checkable_actions['Shifts use VY'].isChecked():
            self.py8_emulator.turnFlagsOff(py8core.IMPL_MODE_SHIFTS_USE_VY)
            self.py8_main_win.checkable_actions['Shifts use VY'].setChecked(False)
        else:
            self.py8_emulator.turnFlagsOn(py8core.IMPL_MODE_SHIFTS_USE_VY)
            self.py8_main_win.checkable_actions['Shifts use VY'].setChecked(True)

    def implmModeBNNN(self) -> None:
        if self.py8_main_win.checkable_actions['BNNN uses VX'].isChecked():
            self.py8_emulator.turnFlagsOff(py8core.IMPL_MODE_SHIFTS_USE_VY)
            self.py8_main_win.checkable_actions['BNNN uses VX'].setChecked(False)
        else:
            self.py8_emulator.turnFlagsOn(py8core.IMPL_MODE_SHIFTS_USE_VY)
            self.py8_main_win.checkable_actions['BNNN uses VX'].setChecked(True)

    def implmModeFX(self) -> None:
        if self.py8_main_win.checkable_actions['FX changes I'].isChecked():
            self.py8_emulator.turnFlagsOff(py8core.IMPL_MODE_SHIFTS_USE_VY)
            self.py8_main_win.checkable_actions['FX changes I'].setChecked(False)
        else:
            self.py8_emulator.turnFlagsOn(py8core.IMPL_MODE_SHIFTS_USE_VY)
            self.py8_main_win.checkable_actions['FX changes I'].setChecked(True)

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

    def resetEmulation(self) -> None:
        self.py8_screen.clearScreen()
        del(self.py8_emulator)
        impl_flags = 0
        if self.py8_impl_shifts_use_vy:
            impl_flags |= 1
        if self.py8_impl_bnnn_uses_vx:
            impl_flags |= (1 << 1)
        if self.py8_impl_fx_changes_ir:
            impl_flags |= (1 << 2)
        self.py8_emulator = py8core.Py8Emulator(implm_flags = impl_flags)
        self.setStatusBarDefualt()

    def saveState(self) -> None:
        pass

    def showKeyBoardMap(self) -> None:
        pass

    def showAbout(self) -> None:
        pass
