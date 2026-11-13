# Ambiguities and unverified items

The three TMC9660 documents (datasheet, Parameter Mode Reference Manual, Register Mode Reference Manual) disagree with
each other in places, or leave a detail open. For each, the driver reads the documents one way by default and a knob
in `Config` or `Quirks` selects the other reading, so a bench check can settle it without a code change. The frame
encoders are proved against the documents' worked examples with `static_assert`.

| Id | The documents | Default reading | Knob |
| --- | --- | --- | --- |
| A1 | Bootloader CRC-8: the prose reflects the output, the worked examples (`0x1D`, `0xA3`) do not | Not reflected | `Quirks::crc8_reflect_output` |
| A2 | TMCL: where the type bits `[11:8]` sit in the byte shared with the motor number: the manual's bit table puts the motor number in the high nibble, a vendor example the other way | `Auto`: probed at start, refuses if both or neither layout answers (fail closed) | `Quirks::tmcl_layout` (`Auto`, `MotorHigh`, `TypeHigh`) |
| A3 | Register SPI: a 9-bit offset in the table, registers up to `0x208`, and a RAMDebug example with 10 bits | Table layout; offsets above `0x1FF` return `Unsupported` | `Quirks::reg_spi_wide_offset` |
| A4 | Register UART: order of the fields in a datagram | Table order, block in the top 5 bits of byte 3 | `Quirks::reg_uart_msb_fields` |
| A5 | TMCL SPI: what to do on `NOT_READY` or `CHECKSUM_ERROR` | Repeat only the reply request, up to the retry count | `Config::spi_reply_retries` |
| A6 | TMCL UART: the gap that discards a partial message, 100 ms or 10 ms | 100 ms assumed; the resync waits `uart_resync_us` (150 ms, a 50 % margin) before it drains; a port must not pause more than 10 ms inside a frame | `Config::reply_timeout_us` (and your port) |
| A7 | Parameter mode has no reset flag in a UART reply | A canary variable written at start and read before each write (UART); `Service()` also reads `GENERAL_ERROR_FLAGS` and treats a `WATCHDOG_EVENT` that was not set at the start as a restart (a watchdog reset only; a power-on reset sets no flag the driver knows of, and the driver never clears the word) | `Config::uart_canary`, `canary_variable`, `canary_value` |
| A8 | Parameter 165: range 1 to 8, reset value 0 | The reset value is accepted, the range is enforced for other writes | the model accepts 0 |
| A9 | `T_WAKE` is given as a typical value only | `boot_timeout_ms` is 500 and bounds the identity wait (after a pin reset the driver first waits 50 ms): each failed poll is charged its pause and, on UART, its reply wait and resync (see [Configuration](configuration.md)) | `Config::boot_timeout_ms` |
| A10 | ABN2_B offset printed as 32, probably 34 | 34 | raw `BootConfig::SetWord()` |
| A11 | The version string has no usual checksum | `GetInfo` is used for the identity | none |
| A12 | `BL_UART_RX` text mentions the "TX pin" | Read as RX | none |
| A13 | The `Boot` reply is sent before the new configuration applies | The reply is read, then the driver waits | `Config::settle_us` |
| A14 | Errata 2 and 6 need explicit values; erratum 5 may cut the host link | Never applied automatically; `DisableSpi1Slave()` is an explicit call | none |
| A15 | Which flags forbid starting | Any non-zero `GENERAL_ERROR_FLAGS` or `GDRV_ERROR_FLAGS`; a warning bit therefore also stops `EnableDrive()` (stricter than needed) | none: clear the flags first |
| A16 | SPI encoder chip-select polarity printed as bit 19 of a 16-bit word | Not exposed as a typed field | raw `BootConfig::SetWord()` |
| A17 | RAMDebug index width: 5 bits (bit ranges) or 6 (offset example 640) | 5 | `Quirks::ram_debug_index_bits` |
| A18 | FAULTN low in an application: a reset or a fault | Not a reset. With `faultn_is_reset` set, a gate-driver fault that pulls FAULTN low makes every register-UART write, including the write that clears the fault, return `UnexpectedReset` and uninitialise the driver; the stop is never gated by it | `Config::faultn_is_reset` |
| A20 | Register mode: the reset value of `GDRV_HW` is printed as `0x03300FFF` in the manual's text and `0x03000FFF` in its register table | The table (`0x03000FFF`); the driver never relies on the reset value | none |
| A21 | TMCL SPI reply: the reply layout is given as a table and as prose that differ in the position of the status byte | The table: status, module, command, value | none (`TmclSpiRequest` is the same either way) |
| A22 | `GetInfo` type 0 (ID): the format of the value is not given | Compared with the chip ID `0x544D0001` | none |
| A23 | Bootloader SPI Table 3: the literal byte layout | Request `command, value[4]`, reply `status, value[4]`, as the worked examples | none |
| A24 | Bootloader: whether writing the boot-mode word starts the application on a 0-to-1 change of `START_MOTOR_CTRL` or on any write with the bit set (the datasheet, "Launching Applications", p. 76, says only that the manually triggered sequence "is run when the boot configuration register is written to"), and whether the bootloader shows the bit set after `Boot` from an auto-started application (datasheet Figures 32, 34, 35) | Any write of the word with the bit set is treated as a start: the driver follows it and falls back to the bootloader when that still answers | none |
| A19 | UART reply time: neither manual bounds the time from the last request byte to the first reply byte | 1 ms assumed | `kTurnaroundUs` (a port reads it; the driver itself waits on the port) |

## Not verified

These are open because the documents do not settle them and no real chip has been available:

- The format of `GetInfo` type 0 in parameter mode (the driver compares it with the chip ID; `Config` has no knob).
- The SPI parameter-mode frame laid out literally as in PM Table 3 (type in bits 8 to 19 and motor in bits 20 to 23 of
  the 32-bit word, MSB first, so type[11:4] in byte 1) is not supported (A2): a read-only probe decodes type 299 as
  0x2B0, the chip answers an error to both probes, and the start is refused with nothing written. Supporting it needs
  a code change (a third `TmclLayout`); it is a bench item. UART is not affected.
- The nibble order (A2) is probed at start-up and refused when it does not decide; which of the two layouts the chip
  uses, whether it ignores the motor nibble, and the register offset width and UART field order (A3, A4) are bench
  items, and so is what a parameter-mode stop does when it reaches a register-mode chip over SPI (the frame lands on
  block 1 offset 9 and block 63 offset 0x100 in the table layout, on MCC 0x104, 0x109 and 0x22b or RAMDebug 0x300 in
  the wide one).
- Whether `GDRV_STATUS_EN` = 0 stops the status from latching, which status bits latch at power-up, and how long the
  bootstrap capacitors need before the protection words are written are bench items (`settle_us` is used).
- The UART reply time (A19) is an assumption.
- Register-mode UART and bootloader UART have no reset evidence in their replies; only FAULTN can show an unseen
  reset, and only when your port has `GpioGet()`.
- `WATCHDOG_EVENT` is a read-write-clear flag the driver does not clear: a chip adopted with it already set cannot
  show a new watchdog reset through that flag (UART still has the canary, SPI the `FIRST_CMD` status).
- Whether `IDLE_MOTOR_PWM_BEHAVIOR` 0 on a real stage keeps the outputs driven after `COMMUTATION_MODE` 0 as the
  manual says is a bench item; the driver treats it as written.
- Whether scripts on the chip can energise the stage: the driver refuses only `SAP` and `AAP` of `COMMUTATION_MODE` in
  a downloaded script (`Refused`); it does not analyse anything else, and `ScriptRun()` starts whatever the chip
  already holds.
- The meaning of every bit of the warning subset of the flag parameters.
- The parameter-mode `GetStatus` type value (0 assumed).
- The numbering of the SIO and GIO pins for `SetIo()` and `GetIo()`.
- The `Verify` trailing arguments that other drivers of the family have are not implemented here: writes are checked
  by the chip's reply and, for the configuration, by reading back.
- One corrupted status byte on SPI can hide `FIRST_CMD` (see [Safety](safety.md)).
- `readParamRaw` ignores its `guarded` argument for reads; reads never energise anything.
- The ESP32 example is compiled against a stub of ESP-IDF only; a build with the real SDK is pending.
- The MISRA C++:2023 scan (CodeQL) runs in CI, not in this host build.

When you settle one on a bench, report it: the change is a default and a changelog line.
