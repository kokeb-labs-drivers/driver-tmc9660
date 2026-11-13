# TMC9660 host example

The driver on your PC, with no hardware: the host port in
[`host_tmc9660_port.hpp`](host_tmc9660_port.hpp) connects the real driver to a simulated TMC9660 (the chip model the
host tests use), and [`main.cpp`](main.cpp) takes it from the bootloader to a running drive: it reads the chip type,
starts the application in parameter mode, sets limits, switches the drive on through the guarded call, reads a
value back and switches the drive off.

## Build and run

From the repository root, with CMake 3.16+ and a C++20 compiler:

```bash
cmake -S examples/host -B build-example
cmake --build build-example
./build-example/tmc9660_host_example
```

## Expected output

```text
TMC9660 driver 0.1.0 on the host
tmc9660: initialised
bootloader: chip type 544D0001
parameter mode: chip type 544D0001
MAX_TORQUE 70000: InvalidArgument
drive on, DRV_EN high
MAX_TORQUE 1500
drive off, DRV_EN low
done: 0 errors
```

The host tests build this folder too and run the example (CTest `tmc9660_host_example`, which expects the last
line), so CI fails if it stops compiling or a call fails.

## Files

| File | What it is |
| --- | --- |
| [`main.cpp`](main.cpp) | The example: bootloader, start, limits, guarded drive on, drive off |
| [`host_tmc9660_port.hpp`](host_tmc9660_port.hpp) | The host port: bus hooks over the simulated chip, `GpioSet()`, `DelayUs()` and `Log()` |
| [`CMakeLists.txt`](CMakeLists.txt) | Standalone project; adds the driver with `add_subdirectory(../..)` |
