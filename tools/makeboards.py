#!/usr/bin/env python3

import contextlib
import io
import sys


# mcu: march, mabi, math_lib_gcc, IQ_math_RV32, ch_extra_lib
mcu_list = {
    "QingKe-V2A": {"march": "rv32ecxw", "mabi": "ilp32e", "ch_extra_lib": "-lprintf"},
    "QingKe-V2C": {"march": "rv32ecxw", "mabi": "ilp32e", "ch_extra_lib": "-lprintf"},
    "QingKe-V3A": {"march": "rv32imac", "mabi": "ilp32", "ch_extra_lib": "-lprintf"},
    "QingKe-V4B": {"march": "rv32imacxw", "mabi": "ilp32", "ch_extra_lib": "-lprintf"},
    "QingKe-V4C": {"march": "rv32imacxw", "mabi": "ilp32", "ch_extra_lib": "-lprintf"},
    "QingKe-V4F": {"march": "rv32imafcxw", "mabi": "ilp32f", "ch_extra_lib": "-lprintfloat"},
}

usb_list = {
    "tinyusb_usbd": {
        "name": "Adafruit TinyUSB with USBD",
        "usb_flags": '-DUSBCON -DUSE_TINYUSB -DCFG_TUD_ENABLED=1 -DCFG_TUD_WCH_USBIP_FSDEV=1',
    },
    "tinyusb_usbd_usbfsh": {
        "name": "Adafruit TinyUSB with USBD/USBFS Host",
        "usb_flags": '-DUSBCON -DUSE_TINYUSB -DUSE_TINYUSB_HOST -DCFG_TUD_ENABLED=1 -DCFG_TUD_WCH_USBIP_FSDEV=1 -DCFG_TUH_ENABLED=1 -DCFG_TUH_WCH_USBIP_USBFS=1',
    },
}

upload_method_list = {
    "ispMethod": {
        "name": "WCH-ISP",
        "upload.tool": "wchisp",
        "bootloader.tool": "wchisp",
    },
    "swdMethod": {
        "name": "WCH-SWD",
        "upload.tool": "WCH_linkE",
        "bootloader.tool": "WCH_linkE",
    },
    "catbootMethod": {
        "name": "CAT Bootloader",
        "upload.tool": "catbootloader",
        "upload.use_1200bps_touch": "true",
        "upload.wait_for_upload_port": "false",
        "build.ldscript": "Link_bootloader.ld",
        "build.flash_base": "0x08000000",
        "build.flash_offset": "0x00004000",
        "build.family_id": "0x699b62ec",
    },
}

profiles = {
    "ch32v203c8_suzuno_common": {
        "series": "CH32V20x",
        "mcu": "QingKe-V4B",
        "chip": "CH32V203",
        "maximum_size": 229376,
        "maximum_data_size": 20480,
        "variant": "CH32V20x/CH32V203C8_Suzuno32RV",
        "variant_h": "variant_CH32V203C8_Suzuno32RV.h",
        "buildflags": "",
        "usb": ["tinyusb_usbd", "tinyusb_usbd_usbfsh"],
        "hsi": [144, 120, 96, 72, 56, 48, 0],
        "hse": [144, 120, 96, 72, 56, 48, 0],
        "upload_methods": ["ispMethod", "swdMethod", "catbootMethod"],
        "bootloader": {
            "file": "{runtime.platform.path}/bootloader/catbootloader-v0.3.3.hex",
            "address": "0x08000000",
        },
    },
}

boards = {
    "suzuno32rv": {
        "name": "Suzuno32RV",
        "profile": "ch32v203c8_suzuno_common",
        "build_board": "SUZUNO32RV",
        "variant": "CH32V20x/SUZUNO32RV",
        "variant_h": "variant_SUZUNO32RV.h",
    },
    "suzuno32rv_pm": {
        "name": "Suzuno32RV Pro Micro",
        "profile": "ch32v203c8_suzuno_common",
        "build_board": "SUZUNO32RV_PM",
        "variant": "CH32V20x/SUZUNO32RV_PM",
        "variant_h": "variant_SUZUNO32RV_PM.h",
        "usb": ["tinyusb_usbd"],
        "upload_methods": ["catbootMethod", "ispMethod", "swdMethod"],
    },
    "suzuduino_uno": {
        "name": "Suzuduino UNO",
        "profile": "ch32v203c8_suzuno_common",
        "build_board": "SUZUDUINO_UNO",
        "variant": "CH32V20x/SUZUDUINO_UNO",
        "variant_h": "variant_SUZUDUINO_UNO.h",
        "usb": ["tinyusb_usbd"],
    },
}


def build_global_menu():
    print("""# See: https://arduino.github.io/arduino-cli/latest/platform-specification/

menu.clock=Clock Select
menu.xserial=U(S)ART support
menu.usb=USB support (if available)
menu.xusb=USB speed (if available)
menu.virtio=Virtual serial support

menu.opt=Optimize
menu.dbg=Debug symbols and core logs
menu.rtlib=C Runtime Library
menu.upload_method=Upload method""")


def merged_board(board_id, board):
    profile = profiles[board["profile"]]
    merged = dict(profile)
    merged.update(board)
    merged["id"] = board_id
    return merged


def build_header(board):
    board_id = board["id"]
    mcu = board["mcu"]
    print()
    print()
    print()
    print("#" * 78)
    print(f"##{board['name']} Board")
    print()
    print(f"{board_id}.name={board['name']}")
    print(f"{board_id}.build.core=arduino")
    print(f"{board_id}.build.board={board['build_board']}")
    print(f"{board_id}.upload.maximum_size={board['maximum_size']}")
    print(f"{board_id}.upload.maximum_data_size={board['maximum_data_size']}")
    print(f"{board_id}.build.variant={board['variant']}")
    print(f"{board_id}.build.variant_h={board['variant_h']}")
    print(f"{board_id}.build.series={board['series']}")
    print(f"{board_id}.build.mcu={mcu}")
    print(f"{board_id}.build.chip={board['chip']}")
    print(f"{board_id}.build.march={mcu_list[mcu]['march']}")
    print(f"{board_id}.build.mabi={mcu_list[mcu]['mabi']}")
    print(f"{board_id}.build.math_lib_gcc=-lm")
    print(f"{board_id}.build.IQ_math_RV32=")
    print(f"{board_id}.build.ch_extra_lib={mcu_list[mcu]['ch_extra_lib'] or ''}")
    print(f"{board_id}.build.board_extra_flags={board.get('buildflags', '')}")
    print(f"{board_id}.debug.tool=gdb-WCH_LinkE")
    if "bootloader" in board:
        print(f"{board_id}.bootloader.file={board['bootloader']['file']}")
        print(f"{board_id}.bootloader.address={board['bootloader']['address']}")
    print()


def build_usb(board):
    if not board["usb"]:
        return
    board_id = board["id"]
    print()
    print("# USB support")
    menu = f"{board_id}.menu.usb"
    for usb in board["usb"]:
        print(f"{menu}.{usb}={usb_list[usb]['name']}")
        print(f"{menu}.{usb}.build.usb_flags={usb_list[usb]['usb_flags']}")
    print(f"{menu}.none=None")
    print(f"{menu}.none.build.usb_flags=")


def build_upload(board):
    board_id = board["id"]
    print()
    print("# Upload menu")
    menu = f"{board_id}.menu.upload_method"

    for method in board["upload_methods"]:
        config = upload_method_list[method]
        entry = f"{menu}.{method}"
        print(f"{entry}={config['name']}")
        print(f"{entry}.upload.protocol=")
        print(f"{entry}.upload.options=")
        for key, value in config.items():
            if key == "name":
                continue
            print(f"{entry}.{key}={value}")


def build_clock(board):
    board_id = board["id"]
    print()
    print("# Clock Select")
    menu = f"{board_id}.menu.clock"
    for hsi in board["hsi"]:
        if hsi == 0:
            print(f"{menu}.HSI=HSI Internal")
            print(f"{menu}.HSI.build.flags.clock=-DSYSCLK_FREQ_HSI=HSI_VALUE -DF_CPU=HSI_VALUE")
        elif hsi == "HSI_LP":
            print(f"{menu}.HSI_LP=HSI_LP Internal")
            print(f"{menu}.HSI_LP.build.flags.clock=-DSYSCLK_FREQ_HSI_LP=HSI_LP_VALUE -DF_CPU=HSI_LP_VALUE")
        else:
            print(f"{menu}.{hsi}MHz_HSI={hsi}MHz Internal")
            print(f"{menu}.{hsi}MHz_HSI.build.flags.clock=-DSYSCLK_FREQ_{hsi}MHz_HSI={hsi}000000 -DF_CPU={hsi}000000")
    for hse in board["hse"]:
        if hse == 0:
            print(f"{menu}.HSE=HSE External")
            print(f"{menu}.HSE.build.flags.clock=-DSYSCLK_FREQ_HSE=HSE_VALUE -DF_CPU=HSE_VALUE")
        else:
            print(f"{menu}.{hse}MHz_HSE={hse}MHz External")
            print(f"{menu}.{hse}MHz_HSE.build.flags.clock=-DSYSCLK_FREQ_{hse}MHz_HSE={hse}000000 -DF_CPU={hse}000000")
    print()


def build_optimization(board):
    board_id = board["id"]
    print()
    print("# Optimizations")
    menu = f"{board_id}.menu.opt"

    print(f"{menu}.osstd=Smallest (-Os default)")
    print(f"{menu}.osstd.build.flags.optimize=-Os")
    print(f"{menu}.oslto=Smallest (-Os) with LTO")
    print(f"{menu}.oslto.build.flags.optimize=-Os -flto")

    print(f"{menu}.o1std=Fast (-O1)")
    print(f"{menu}.o1std.build.flags.optimize=-O1")
    print(f"{menu}.o1lto=Fast (-O1) with LTO")
    print(f"{menu}.o1lto.build.flags.optimize=-O1 -flto")

    print(f"{menu}.o2std=Faster (-O2)")
    print(f"{menu}.o2std.build.flags.optimize=-O2")
    print(f"{menu}.o2lto=Faster (-O2) with LTO")
    print(f"{menu}.o2lto.build.flags.optimize=-O2 -flto")

    print(f"{menu}.o3std=Fastest (-O3)")
    print(f"{menu}.o3std.build.flags.optimize=-O3")
    print(f"{menu}.o3lto=Fastest (-O3) with LTO")
    print(f"{menu}.o3lto.build.flags.optimize=-O3 -flto")

    print(f"{menu}.ogstd=Debug (-Og)")
    print(f"{menu}.ogstd.build.flags.optimize=-Og")
    print(f"{menu}.o0std=No Optimization (-O0)")
    print(f"{menu}.o0std.build.flags.optimize=-O0")
    print()


def build_debug(board):
    board_id = board["id"]
    print()
    print("# Debug information")
    menu = f"{board_id}.menu.dbg"
    print(f"{menu}.none=None")
    print(f"{menu}.none.build.flags.debug=-DNDEBUG")
    print(f"{menu}.enable_sym=Symbols Enabled (-g)")
    print(f"{menu}.enable_sym.build.flags.debug=-g -DNDEBUG")
    print(f"{menu}.enable_log=Core logs Enabled")
    print(f"{menu}.enable_log.build.flags.debug=")
    print(f"{menu}.enable_all=Core Logs and Symbols Enabled (-g)")
    print(f"{menu}.enable_all.build.flags.debug=-g")
    print()


def build_runtimelib(board):
    board_id = board["id"]
    print()
    print("# C Runtime Library")
    menu = f"{board_id}.menu.rtlib"
    print(f"{menu}.nano=Newlib Nano (default)")
    print(f"{menu}.nano.build.flags.ldflags=--specs=nano.specs --specs=nosys.specs")
    print(f"{menu}.nanofp=Newlib Nano + Float Printf")
    print(f"{menu}.nanofp.build.flags.ldflags=--specs=nano.specs --specs=nosys.specs -u _printf_float")
    print(f"{menu}.nanofs=Newlib Nano + Float Scanf")
    print(f"{menu}.nanofs.build.flags.ldflags=--specs=nano.specs --specs=nosys.specs -u _scanf_float")
    print(f"{menu}.nanofps=Newlib Nano + Float Printf/Scanf")
    print(f"{menu}.nanofps.build.flags.ldflags=--specs=nano.specs --specs=nosys.specs -u _printf_float -u _scanf_float")
    print(f"{menu}.full=Newlib Standard")
    print(f"{menu}.full.build.flags.ldflags=--specs=nosys.specs")
    print()


def make_board(board_id, board):
    merged = merged_board(board_id, board)
    build_header(merged)
    build_usb(merged)
    build_upload(merged)
    build_clock(merged)
    build_optimization(merged)
    build_debug(merged)
    build_runtimelib(merged)


def emit_boards():
    build_global_menu()
    for board_id, board in boards.items():
        make_board(board_id, board)


if __name__ == "__main__":
    if len(sys.argv) > 1:
        buffer = io.StringIO()
        with contextlib.redirect_stdout(buffer):
            emit_boards()
        with open(sys.argv[1], "w", encoding="utf-8", newline="\n") as fh:
            fh.write(buffer.getvalue())
    else:
        emit_boards()
