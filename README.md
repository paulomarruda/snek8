# Snek8
Snek8 is a [CHIP8](https://en.wikipedia.org/wiki/CHIP-8) emulator writen in C/Python. Snek8 aims to ship an easy to use emulator with a comprehensive GUI. Most of CHIP8 emulators rely heavely on the command line, which can frustrate users that are not used to it; hence, this project aims to chip an executable with everything the user needs to play (except the ROMs).

## License
Snek8 is licensed under the [GNU Public License](https://www.gnu.org/licenses/gpl-3.0.en.html).

## Executables and Installation
The link for the executables can be found on the project's [page](https://paulomarruda.github.io/snek8)

If you prefer, you can build the project from source:

### Linux

### Windows

### Mac

```bash
pip install -r requirements.txt
python setup.py install
```
## Usage

### Keys

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
