"""
@file app.py
@author Paulo Arruda
@license GPL-3
@brief Implementation of the main app functionalities.
"""
import os
import sys
from snek8 import core as snek8core
from main_window import Snek8MainWindow
from screen import Snek8Screen
from typing import List, Dict, Callable
from PyQt6.QtWidgets import QApplication, QFileDialog
from PyQt6.QtGui import QIcon, QGuiApplication
from PyQt6.QtCore import Qt, QBasicTimer, QTimer

PARENT_DIR: str = 'snek8'

class Snek8App(QApplication):
    """
    Snek8.
    A Chip8 emulator writen in Python.

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
                lambda: self.snek8_emulator.setKeyValue(0x1, False),
                lambda: self.snek8_emulator.setKeyValue(0x1, True),
            ],
            Qt.Key.Key_2: [
                lambda: self.snek8_emulator.setKeyValue(0x2, False),
                lambda: self.snek8_emulator.setKeyValue(0x2, True),
            ],
            Qt.Key.Key_3: [
                lambda: self.snek8_emulator.setKeyValue(0x3, False),
                lambda: self.snek8_emulator.setKeyValue(0x3, True),
            ],
            Qt.Key.Key_4: [
                lambda: self.snek8_emulator.setKeyValue(0xC, False),
                lambda: self.snek8_emulator.setKeyValue(0xC, True),
            ],

            Qt.Key.Key_Q: [
                lambda: self.snek8_emulator.setKeyValue(0x4, False),
                lambda: self.snek8_emulator.setKeyValue(0x4, True),
            ],
            Qt.Key.Key_W: [
                lambda: self.snek8_emulator.setKeyValue(0x5, False),
                lambda: self.snek8_emulator.setKeyValue(0x5, True),
            ],
            Qt.Key.Key_E: [
                lambda: self.snek8_emulator.setKeyValue(0x6, False),
                lambda: self.snek8_emulator.setKeyValue(0x6, True),
            ],
            Qt.Key.Key_R: [
                lambda: self.snek8_emulator.setKeyValue(0xD, False),
                lambda: self.snek8_emulator.setKeyValue(0xD, True),
            ],

            Qt.Key.Key_A: [
                lambda: self.snek8_emulator.setKeyValue(0x7, False),
                lambda: self.snek8_emulator.setKeyValue(0x7, True),
            ],
            Qt.Key.Key_S: [
                lambda: self.snek8_emulator.setKeyValue(0x8, False),
                lambda: self.snek8_emulator.setKeyValue(0x8, True),
            ],
            Qt.Key.Key_D: [
                lambda: self.snek8_emulator.setKeyValue(0x9, False),
                lambda: self.snek8_emulator.setKeyValue(0x9, True),
            ],
            Qt.Key.Key_F: [
                lambda: self.snek8_emulator.setKeyValue(0xE, False),
                lambda: self.snek8_emulator.setKeyValue(0xE, True),
            ],

            Qt.Key.Key_Y: [
                lambda: self.snek8_emulator.setKeyValue(0xA, False),
                lambda: self.snek8_emulator.setKeyValue(0xA, True),
            ],
            Qt.Key.Key_X: [
                lambda: self.snek8_emulator.setKeyValue(0x0, False),
                lambda: self.snek8_emulator.setKeyValue(0x0, True),
            ],
            Qt.Key.Key_C: [
                lambda: self.snek8_emulator.setKeyValue(0xB, False),
                lambda: self.snek8_emulator.setKeyValue(0xB, True),
            ],
            Qt.Key.Key_V: [
                lambda: self.snek8_emulator.setKeyValue(0xF, False),
                lambda: self.snek8_emulator.setKeyValue(0xF, True),
            ],
        }
    @property
    def APP_KEY_MAP(self) -> Dict[int, Callable]:
        return {
            Qt.Key.Key_P: lambda: self.pause(),
            Qt.Key.Key_Escape: lambda: self.snek8_main_win.close(),
            Qt.Key.Key_L: lambda: self.loadRom(),
        }

    @property
    def WIDTH(self) -> int:
        return 640

    @property
    def HEIGHT(self) -> int:
        return 360
    
    def __init__(self, argv: List[str]) -> None:
        super(Snek8App, self).__init__(argv)
        self.initCore()
        self.initUI()
        self.snek8_main_win.show()
        self.timer.timeout.connect(self.emulate)
        self.timer.start(1000 // self.fps)

    def initUI(self) -> None:
        self.snek8_main_win = Snek8MainWindow("Snek8 - Chip8 Emulator",
                                      self.WIDTH,
                                      self.HEIGHT
        )
        self.snek8_main_win.centerWindowOnScreen(QGuiApplication.primaryScreen().geometry().width(),
                                           QGuiApplication.primaryScreen().geometry().height())
        self.snek8_main_win.setKeys(self.CPU_KEY_MAP, self.APP_KEY_MAP)
        self.snek8_screen = Snek8Screen(self.snek8_main_win)
        self.snek8_main_win.setCentralWidget(self.snek8_screen)
        self.setStatusBarDefualt()
        self.initMenus()

    def initCore(self) -> None:
        self.snek8_impl_shifts_use_vy = False
        self.snek8_impl_bnnn_uses_vx = False
        self.snek8_impl_fx_changes_ir = False
        self.timer = QTimer()
        self.is_paused = False
        self.fps = 120
        self.snek8_emulator = snek8core.Snek8Emulator(implm_flags = 0)

    def initMenus(self) -> None:
        # File menu
        self.snek8_main_win.addMenu("File")
        self.snek8_main_win.addMenuItem("File", "Load Rom", self.loadRom)
        self.snek8_main_win.addMenuItem("File", "Quit", self.snek8_main_win.close)
        # Options menu
        self.snek8_main_win.addMenu("Options")
        self.snek8_main_win.addMenuItem("Options", "Reset", self.resetEmulation)
        self.snek8_main_win.addMenuItem("Options", "Pause", self.pause)
        self.snek8_main_win.addMenuItem("Options", "Save state", self.saveState)
        # Implementation
        self.snek8_main_win.addMenu("Implementation")
        self.snek8_main_win.addCheckMenu('Implementation', "Shifts use VY", self.implmModeShifts)
        self.snek8_main_win.addCheckMenu('Implementation', "BNNN uses VX", self.implmModeBNNN)
        self.snek8_main_win.addCheckMenu('Implementation', "FX changes I", self.implmModeFX)
        # Help Menu
        self.snek8_main_win.addMenu("Help")
        self.snek8_main_win.addMenuItem("Help", "Key mappings", self.showKeyBoardMap)
        self.snek8_main_win.addMenuItem("Help", "About", self.showAbout)

    def showError(self, err_msg: str) -> None:
        pass

    def handleSound(self, sound_timer: int) -> None:
        pass

    def setStatusBarPaused(self) -> None:
        self.snek8_main_win.status_bar.setText(self.STATUS_BAR_PAUSED)

    def setStatusBarRunning(self) -> None:
        self.snek8_main_win.status_bar.setText(self.STATUS_BAR_RUNNING)

    def setStatusBarDefualt(self) -> None:
        self.snek8_main_win.status_bar.setText(self.STATUS_BAR_DEFAULT)


    def pause(self) -> None:
        if self.snek8_emulator.is_running:
            self.is_paused = False if self.is_paused else True
            if self.is_paused:
                self.setStatusBarPaused()
            else:
                self.setStatusBarRunning()

    def loadRom(self) -> None:
        if self.snek8_emulator.is_running:
            self.resetEmulation()
        self.rom_filepath = QFileDialog.getOpenFileName(
            parent = self.snek8_main_win,
            caption = "Select a ROM file",
            directory = os.getcwd()
        )[0]
        if os.path.isfile(self.rom_filepath):
            out: int = self.snek8_emulator.loadRom(self.rom_filepath)
            match out:
                case snek8core.EXECOUT_SUCCESS:
                    self.setStatusBarRunning()
                    pass
                case snek8core.EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM:
                    pass
                case snek8core.EXECOUT_ROM_FILE_FAILED_TO_OPEN:
                    pass
                case snek8core.EXECOUT_ROM_FILE_FAILED_TO_READ:
                    pass
                case snek8core.EXECOUT_ROM_FILE_NOT_FOUND:
                    pass
                case _:
                    pass

    def implmModeShifts(self) -> None:
        if self.snek8_main_win.checkable_actions['Shifts use VY'].isChecked():
            self.snek8_emulator.turnFlagsOn(snek8core.IMPL_MODE_SHIFTS_USE_VY)
            self.snek8_main_win.checkable_actions['Shifts use VY'].setChecked(True)
        else:
            self.snek8_emulator.turnFlagsOff(snek8core.IMPL_MODE_SHIFTS_USE_VY)
            self.snek8_main_win.checkable_actions['Shifts use VY'].setChecked(False)

    def implmModeBNNN(self) -> None:
        if self.snek8_main_win.checkable_actions['BNNN uses VX'].isChecked():
            self.snek8_emulator.turnFlagsOn(snek8core.IMPL_MODE_BNNN_USES_VX)
            self.snek8_main_win.checkable_actions['BNNN uses VX'].setChecked(True)
        else:
            self.snek8_emulator.turnFlagsOff(snek8core.IMPL_MODE_BNNN_USES_VX)
            self.snek8_main_win.checkable_actions['BNNN uses VX'].setChecked(False)

    def implmModeFX(self) -> None:
        if self.snek8_main_win.checkable_actions['FX changes I'].isChecked():
            self.snek8_emulator.turnFlagsOn(snek8core.IMPL_MODE_FX_CHANGES_I)
            self.snek8_main_win.checkable_actions['FX changes I'].setChecked(True)
        else:
            self.snek8_emulator.turnFlagsOff(snek8core.IMPL_MODE_FX_CHANGES_I)
            self.snek8_main_win.checkable_actions['FX changes I'].setChecked(False)

    def emulate(self) -> None:
        if self.is_paused or (not self.snek8_emulator.is_running):
            return
        out: int = self.snek8_emulator.emulationStep()
        match out:
            case snek8core.EXECOUT_SUCCESS:
                pass
            case snek8core.EXECOUT_EMPTY_STRUCT:
                pass
            case snek8core.EXECOUT_INVALID_OPCODE:
                pass
            case snek8core.EXECOUT_STACK_EMPTY:
                pass
            case snek8core.EXECOUT_STACK_OVERFLOW:
                pass
            case _:
                pass
        self.handleSound(self.snek8_emulator.getST())
        self.snek8_screen.updateScreen(self.snek8_emulator.getGraphics())

    def resetEmulation(self) -> None:
        self.snek8_screen.clearScreen()
        del(self.snek8_emulator)
        impl_flags = 0
        if self.snek8_impl_shifts_use_vy:
            impl_flags |= 1
        if self.snek8_impl_bnnn_uses_vx:
            impl_flags |= (1 << 1)
        if self.snek8_impl_fx_changes_ir:
            impl_flags |= (1 << 2)
        self.snek8_emulator = snek8core.Snek8Emulator(implm_flags = impl_flags)
        self.setStatusBarDefualt()

    def saveState(self) -> None:
        pass

    def showKeyBoardMap(self) -> None:
        pass

    def showAbout(self) -> None:
        pass

def main() -> None:
    app = Snek8App(sys.argv)
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
