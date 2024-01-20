# `sx2flash`: an MCU bootloader to flash firmware using only a serial console and XMODEM

_Please note: this is an end-of-course test project. It may contain bugs and be
unreliable._

`sx2flash` is a bootloader program (currently supporting the ATmega32 chip, but
should be easy to port it to other MCUs) that can upload firmware to the MCU
flash storage by requiring the client to only have a UART adapter, a serial
console like `picocom` and an implementation of the XMODEM send tool `sx`.

Normally, one would feed the firmware in Intel Hex (or equivalent) format to a
programmer like `avrdude` to update the MCU. This approach works even when
the flash storage is empty, but requires a separate hardware programmer such as
a dedicated USB programmer or an Arduino. Often, a bootloader is flashed on
devices before shipping, which allow the device to boot and look for firmware
using an alternate method, such as via the UART.

`sx2flash` does exactly this, and is designed to work with widespread and cheap tools:

* physical transfer happens over a UART; the pristine Intel Hex file is sent to
  the MCU: the bootloader performs checksum verification and conversion from ASCII to
  raw binary;
* the Hex file is fragmented and sent to the MCU on top of the XMODEM protocol.

Once the MCU is connected to a UART adapter, the only software you need is a
serial console and an XMODEM upload tool. I usually use `picocom` and `lrzsz`
`sx` implementation, but other tools are good as well.

## Features

* Configurable (at compile time) MCU clock speed, baud rate, maximum Hex file record
  size and timeouts.
* Clean separation between MCU-independent and dependent code, can be ported to other
  MCUs.
* Supports Intel hex files up to 4GB via address extension records (both segmented and linear).

## Constraints

* The code is small enough to fit into the 4kB available for the bootloader on
  the ATmega32, with the current image size being around 2.5kB.
* The MCU must have enough RAM to store:
  * a full XMODEM packet;
  * a dirty flash page;
  * some bookkeeping data used to rebuild hex record broken across XMODEM packets.
    This amount depends on the largest hex record that can be handled. By default,
    the code assumes 16 bytes per record, but this can be extended up to 255.
* Hex records can have any kind of blanks between them, which are ignored.
* Checksums in XMODEM packets are not checked, since each Hex record already carries an
  individual checksum.

The current build requires 311 bytes of RAM for data/BSS, plus what's needed for
the stack.  It's safe to assume 512 bytes as the lower limit.

Caching a full XMODEM packet ensures we can take out time to process the data,
as the client must not send another packet unless we send back an ACK. Page
caching ensures record the go into the same page are coalesced and a single
erase/write is performed.

Note that, if the input Hex file only writes part of a flash page, the old
unaddressed contents are preserved. Only the location appearing in the Hex file
are updated.

## Code structure

At the top level, there are the `tools` and `src` folders. `src` contains the code as well as
the makefiles. Build can be done either inside or outside `src`, by referencing the makefile:

```bash
cd src && make # Inside src
mkdir build && cd build && make -f /path/to/src/Makefile # Outside src
```

At the end of the build, there will be an elf file `sx2flash.elf` and, if
required, an `sx2flash.hex` file, which can then be flashed.

Under `src` the code is divided into MCU-independent stuff, which goes under
`common`, and MCU-dependent stuff, which goes under `device/$DEVICE`.  Both
folders contains a `sx2flash` folder which contains header files.
Device-specific headers carry information which cater to a specific MCU.  The
target device is chosen when running `make` as explained below.

Apart from the `atmega32a` target, there is also a `native` target, which
compiles a native version to be run on the local machine for debugging
purposes. MCU-specific code is replaced with an alternate implementation using
POSIX calls, and flash memory is emulated using a file named `flash.bin` in the
present working directory.

Under `tools` there is Python tool that can take any file and format it as a
sequence of XMODEM packets. This is required by the `native` target, which
expects to read a stream of XMODEM data from the standard input.

## How to build

Supposing we want to perform a build outside `src`, we will call make like this:

```bash
make -f /path/to/src/Makefile [build options]...
```

Build options are make variables that are used to customize certain facets of
the bootloader. Some are generic, while others only apply to a certain target
device. They can all be found at the top of makefiles. This is what is supported
so far:

* Generic:
  * `XMODEM_TIMEOUT`: milliseconds to wait between sending a NAK/ACK to the
    sender.  Also defines how much time the MCU will wait after reset for an
    upload to begin before transferring control to the program area.
  * `IHEX_MAX_DATA_PER_RECORD`: largest hex record that can be handled (it
    refers to the size of the data portion only).
  * `DEVICE`: name of the target device. Must match the name of one folder
    under `src/device`.
* `atmega32a`:
  * `F_CPU`: frequency of the MCU, in Hz.
  * `BAUD_RATE` desired baud rate of the UART. The values chosen for `F_CPU`
    and `BAUD_RATE` must be mutually compatible, since the baud rate is
    selected by passing the CPU clock through a scaler.
* `native`:
  * `FLASH_SIZE`: size of thr emulated flash, in bytes.
  * `FLASH_PAGE_SIZE`: size of the flash page, in bytes. `FLASH_SIZE` must be a
    multiple of this value.

## Notes on the `native` target

When building with `DEVICE=native`, no Hex file is produced, just an ELF file.
This contains debug symbols and no optimizations and can be used to perform
local debug. Of course, MCU-dependant code is replaced by common POSIX calls to
implement serial read/write, timers and file I/O. However, it makes it easier to
test the XMODEM/Intel Hex parsing code.

* This build reads XMODEM data from standard input and writes to standard
  output.  Input data must be already formatted as XMODEM packets. The script
`tools/pack2xmodem.py` can be used to turn an Hex file into a sequence of XMODEM
packets.
* It creates a `flash.bin` file in the current directory and writes data there.
  It can be investigated afterward with an hex editor.