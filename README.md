# PDP11MONLOADER
"pdp11monloader" is a tool to download code into vintage PDP-11 computers over serial
connection using the console monitor.
After download the code can be started, even a simple non-graphical terminal emualtion
is implemented.

Several code file formats are supported, and several PDP-11 types can be controlled.

pdp11monloader is a Linux command line tool written in C.
It can also compiled under CYGWIN to run on MS_Windows, in this the support file "cygwin.dll"
is needed.

The commandline syntax is:

```NAME

pdp11monloader - Load binary code into PDP-11 over console monitor.
                 Version v1.0.0, compile Jun 23 2017 08:07:10.
(C) 2017 Joerg Hoppe <j_hoppe@t-online.de>


SYNOPSIS

pdp11monloader  --help --version --debug --baudrate <baudrate>
          --format <bits_parity_stop> --port <serial_device> --odt --m9312
          --m9301 --octaltext <inputfile> --macro11listing <inputfile>
          --papertape <inputfile> --go [<start_addr>]
          --gocomitted [<start_addr>] --tty
          --usbdelay <milliseconds>

-?     | --help
          Print help.
-v     | --version
          Output version string.
-dbg   | --debug
          Enable debug output to console.
-b     | --baudrate <baudrate>
          Set serial line speed to 300..3000000 baud. Default: "38400"
-f     | --format <bits_parity_stop>
          Set format parameters for serial line as a 3 char string <bitcount><parity><stopbits>
          <bitcount> maybe 7 or 8, <parity> is n (no), e (even) or o (odd), <stopbits> is 1 or 2.
          Set to special console params for --boot operation. leave default for device emulation.
           Default: "8N1"
          Simple example:  -f 7e2
              Set for 7 bit even parity with 2 stop bits (--boot)
-p     | --port <serial_device>
          Select serial port: "COM<serial_device>:" or <serial_device> is a node like "/dev/ttyS1"
-odt   | --odt
          Set PDP-11 console monitor type to "ODT", for all QBUS machines
-m9312 | --m9312
          Set PDP-11 console monitor type to "M9312".
          This BOOT-terminator card is used on many UNIBUS machines
          and is also build into the 11/44.
-m9301 | --m9301
          Older variant of M9312 BOOT terminator.
-ot    | --octaltext <inputfile>
          Input file is interpreted as text file containing octal address and data words.
          Words made of characters '0' to '7', all other chars are interpreted as white space.
          First words in each text line is address, next words are data.
          For example, the "octaltext" format allows to read SimH "deposit" scripts.
          Simple example:  -ot hello.txt
              load memory from hello.txt
-ml    | --macro11listing <inputfile>
          Input file is interpreted as MACRO-11 listing
          Code is parsed and DEPOSITed into memory.
          Simple example:  -ml hello.lst
              load memory from hello.lst
-pt    | --papertape <inputfile>
          Input file is interpeted as "Standard Absolute Paper Tape" image.
-go    | --go [<start_addr>]
          Issue a "<start_addr>G" to start program execution.
          The RUN/HALT switch must be in RUN position.
          <start_addr> can be omitted, if code loaded by previous --ptap
          Simple example:  -go 1000
              Start program execution at address 1000 (octal)
-gc    | --gocomitted [<start_addr>]
          Same as "go", but the final "G" is not sent.
          Typical used if a terminal emulation is started next:
          User types a single "G" to start and no output is lost
          Simple example:  -gc
              Start program execution at address set by papertape file (if any)
-t     | --tty
          After operation a simple line based terminal remains active on the serial port,
          so you can operate the PDP-11 over serial console immediately.
          If not used after "go" you have to start a more comfortable terminal emulator
          and will probably loose some output of the started program.
-ud    | --usbdelay <milliseconds>
          Specifies extra delay for console monitor protocol.
          Some USB-RS232 adapters have large delays when polling input (for example FTDIs).
          Other brands (Prolific) and non-USB RS232 ports should never need this.
          Experiment for an optimum between download speed and reliability, recommended range 5-20.

Option names are case insensitive.

EXAMPLES

sudo ./pdp11monloader -p /dev/ttyS2 -b 9600 -odt --octaltext pattern.bin
    The PDP-11 is connected to serial RS232 port "ttyS1" under Linux,
    Baud rate is 9600, data format is "8N1" by default.
    Access to serial line device requires "sudo".
    It is a QBUS LSI-11 (11/03,11/23,11/73) with ODT console monitor.
    The PDP-11 must have been halted to show the "@" prompt.
    Data is read from file "pattern.bin", in the form of
    one octal address and one or more octal 16bits words per line.
    The data is loaded into the PDP-11 memory over console "Deposit" commands.

sudo ./pdp11monloader -p /dev/ttyS2 -b 9600 -odt -ml dd.lst -go 1000 -tty
    Same PDP-11 and connection as before, booting a TU58 tape device:
    pdp11monloader eliminates the need for a Boot ROM.
    The boot loader code is parsed from MACRO-11 listing file "dd.lst"
    After loading the code program execution is started at address 1000,
    then program switches mode and user can operate the running PDP-11 over a
    simple terminal emulation.

pdp11monloader.exe -p 1 -b 9600 -odt --macro11listing cxcpag.lst -go 220 -tty
    Call when running under MS Windows, serial port COM1 is used for connection.
    This time the diagnostic CXCPAG is executed at start address 000220.

pdp11monloader.exe -p 1 -b 9600 -odt -pt DEC-11-AJPB-PB.ptap -go -tty
    Code is loaded from an "Absolute Papertape" image, its the
    "Papertape BASIC" from 1972. Execution is started.
    The program start address is read from paper tape and not given on command line.

pdp11monloader.exe -p 1 -b 4800 -fmt 7e1 -m9312 -pt DEC-11-AJPB-PB.ptap -go -tty
    BASIC is run on an UNIBUS PDP-11, which executes the console monitor from a
    M9312 Boot Terminator card. The DL11 serial console on this PDP-11 is jumpered
    to 38400 baud, serial line format is 7 bit with even parity.

pdp11monloader.exe -p 7 -b 115200 --usbdelay 7 -m9312 -pt DEC-11-AJPB-PB.ptap -go -tty
    As before. An USB-to-RS232 adapter is used, plug'n'play assigns port number COM7.
    The PDP-11 console DL11 hardware is tuned for 115200 baud,
    this USB dongle is FTDI and needs 7 ms extra delay.
```
