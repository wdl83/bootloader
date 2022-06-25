AVR bootloader with Modbus RTU support
========================================

Overview
--------
**bootloader** can be used to update/manage devices connected to Modbus network.
Without the need for ISP programmer or physical contact with the device.

Dependencies
------------
Compile time dependencies are managed with git submodules. Please keep in mind
to use appropriate switches (like --recurse-submodules).

1. [ATmega328 peripheral drivers](https://github.com/wdl83/atmega328p_drv)
1. [Modbus RTU state machine](https://github.com/wdl83/modbus_c)

Supported devices
-----------------
atmega328p: all development and testing is done with Arduino Pro Mini (clones).

Building
--------
1. git clone --recurse-submodules https://github.com/wdl83/bootloader
1. cd bootloader
1. RELEASE=1 make

Building with Docker
--------------------
1. git clone https://github.com/wdl83/bootloader
1. cd bootloader
1. ./make_env.sh # generate .env
1. sudo docker-compose up
1. grep DST= .env # directory where artifacts are located

Usage
-----
Basic usage scenario will require 2 additional components:

1. [MDP Broker and Client](https://github.com/wdl83/mdp)
1. [Modbus MDP Worker](https://github.com/wdl83/modbus_mdp)
1. [MDP Client with support for firmware update](https://github.com/wdl83/fwupdate_mdp)

Boot sequencing
---------------
In case of power on reset (POR) sequencing will execute **bootloader** code:

```c
    exec_bootloader_code();
```
which initializes Modbus RTU state machine, enables **hardware watchdog**
(with 8 seconds timeout) and waits for RTU commands. In case **hardware watchdog**
is not disabled via RTU command (or reset every 8 seconds), MCU goes into
**hardware watchdog** reset (WDRE). After **hardware watchdog** reset (WDRE),
boot sequencing code checks **reset_signature** and if it matches
**RESET_SIGNATURE_BOOT_APP**, application code is executed.

```c
    if(
        (fixed__.mcusr & M1(WDRF))
        && RESET_SIGNATURE_BOOT_APP == fixed__.reset_signature)
    {
        fixed__.reset_signature = (uint8_t)~RESET_SIGNATURE_BOOT_APP;
        exec_app_code();
    }
```

Clearing **reset_signature** before jump to application code ensures that any
crash in app code will result in executing **bootloader** code on next
**hardware watchdog** reset (which is enabled just before jump to app code).

```c
    __attribute__((noreturn))
    void exec_app_code(void)
    {
        ++fixed__.app_counter;
        fixed__.app_reset_code.last = fixed__.app_reset_code.curr;
        fixed__.app_reset_code.curr = RESET_CODE_APP_EXEC_FAILED;
        watchdog_enable(WATCHDOG_TIMEOUT_16ms);
        asm("jmp 0000");
        for(;;) {}
    }
```

Application code is expected to reset **hardware watchdog** every 16ms or disable
**hardware watchdog** (which should be avoided). Its possible to change watchdog
timeout in case 16ms is not adequate.

fixed__ (fixed_t)
-----------------------------
Special region of SRAM which is located outside of C-runtime:

[Linker script](https://github.com/wdl83/bootloader/blob/master/atmega328p.ld)

is called **fixed__** of type **fixed_t**. It can be used to preserve data between
resets (assuming power was not disconnected from device). Its used to pass data
between **bootloader** and application code. Region is cleared with zeros on POR.

```c
    if(fixed__.mcusr & M1(PORF))
    {
        /* if power was lost SRAM state is undefined
         * (memset/bzero not used because of volatile)*/
        for(uint8_t i = 0; i < FIXED_SIZE; ++i) fixed__.bytes[i] = 0;
    }
```

Supported Modbus RTU Commands
-----------------------------

Quick command summary
---------------------

Start MDP broker:
```console
broker -a tcp://0.0.0.0:6060 &
```
Start Modbus MDP worker:
```console
master_worker -a tcp://127.0.0.1:6060 -d /dev/ttyUSB0 -s ModbusOverSerial0 
```

Put device in boot sequencing. If device is executing application code,
request reset (via application RTU command) or power on/off the device.
```console
client -a tcp://127.0.0.1:6060 -s ModbusOverSerial0 -i app_reset_request.json
```

update device which Modbus address is **slaveID** with firmware from fw.ihex:
```console
./fwupdate.elf -a tcp://127.0.0.1:6060 -s ModbusOverSerial0 -f fw.ihex -t slaveID
```

Boot decision
-------------
![diagram](diagrams/boot_decision.png)
