# Snek8
Snek8 is a [CHIP8](https://en.wikipedia.org/wiki/CHIP-8) emulator writen in C/Python with [PyQT6](https://doc.qt.io/qtforpython-6/).

## License
Snek8 is licensed under the [GNU Public License](https://www.gnu.org/licenses/gpl-3.0.en.html).

## Executables and Installation
The exacutables are not yet avaiable.

For installation from source and running, execute the following:

```bash
pip install -r requirements.txt
python setup.py install
python app/app.py
```
## Usage
You can either navigate the GUI menu or use the hot keys:

| Key     |                  Action|
|---------|------------------------|
| `p`     | Pause                  |
| `ESC`   | Quit                   |
| `l`     | Reset and load new ROM | 

### Implementations
Different dialects of the CHIP-8 language exists, see [Tobias V. Langhoff' Tutorial](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/). To account for these differences, the `Implementation` menu allows the user to activate flags that modifies the instructions' execution.

| Flag | Instruction Affected | Explanation |
|------|----------------------|-------------|
| `Shifts Use VY` | `SHR Vx, Vy` and `SHL Vx, Vy` | Some interpreters load the content of `Vy` into `Vy` before performing the bitwise shifts.|
| `BNNN Uses VX`  | `JP V0, addr` | Some interpreters decode this instruction as `I := V0 + 0x0NNN` (opcode `0xBNNN`) and others as `I := VX + 0x0XNN` (opcode `0xBXNN`).|
| `FX uses I` | `LD [I], Vx` and `LD Vx, [I]` | Some interpreters increment the `index register` each time the information is exchanged between memory and the registers.|

### CHIP-8 Keys

The COSMAC-VIP had a hexadecimal keypad as follows:

| `1` | `2` | `3` | `C` |
|-----|-----|-----|-----|
| `4` | `5` | `6` | `D` |
| `7` | `8` | `9` | `E` |
| `A` | `0` | `B` | `F` |

Our emulator maps this keypad to a the modern computer keyboard as follows:

| `1` | `2` | `3` | `4` |
|-----|-----|-----|-----|
| `Q` | `W` | `E` | `R` |
| `A` | `S` | `D` | `F` |
| `Z` | `X` | `C` | `V` |


## ROMs
We do not include any ROM file here, but you can encounter CHIP8 ROMs all over internet. We list some repositiories:

- [https://github.com/jamesmcm/chip8go](https://github.com/jamesmcm/chip8go)
- [https://johnearnest.github.io/chip8Archive/](https://johnearnest.github.io/chip8Archive/)
- [https://chip-8.github.io/links/](https://chip-8.github.io/links/)

### Tested ROMs

| ROM Name                                                                                          |      Working       |     Flags     |
|:--------------------------------------------------------------------------------------------------|:------------------:|:-------------:|
|[CHIP-8 splash screen](https://github.com/Timendus/chip8-test-suite/raw/main/bin/1-chip8-logo.ch8)| :heavy_check_mark: | |
|[IBM Logo](https://github.com/kripod/chip8-roms/blob/master/programs/IBM%20Logo.ch8)| :heavy_check_mark: |  |
|[Keypad Test](https://github.com/kripod/chip8-roms/blob/master/programs/Keypad%20Test%20%5BHap%2C%202006%5D.ch8)| :heavy_check_mark: |               |
|[Corax+ opcode test](https://github.com/Timendus/chip8-test-suite/raw/main/bin/3-corax+.ch8)| :heavy_check_mark: | |

## TO-DOs
 - Make an executable avaiable for Windows and Mac.
 - Implement popup windows to indicate execution errors (e.g `NOP`).
