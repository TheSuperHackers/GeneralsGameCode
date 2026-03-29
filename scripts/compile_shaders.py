#!/usr/bin/env python3
"""
Pixel Shader 1.1 Assembler - Generates C++ header with precompiled shader bytecode
    
This script assembles DirectX 8 Pixel Shader 1.1 assembly code into bytecode
and generates a C++ header file that can be included in the project.

Usage: python3 compile_shaders.py <shader1.psh> [shader2.psh ...]

Note: This is a simple PS 1.1 assembler that handles the basic instructions
used in the water shaders. For D3D8 ps.1.1 format.
"""

import sys
import struct
import re
from pathlib import Path

# PS 1.1 instruction opcodes (D3D8 format)
PS_OPCODES = {
    'tex': 0x42,      # TEX instruction
    'texbem': 0x43,   # TEXBEM instruction  
    'mul': 0x05,      # MUL instruction
    'add': 0x03,      # ADD instruction
    'mad': 0x04,      # MAD instruction (multiply-add)
}

# Register types
REG_TEMP = 0           # r0-r7 (temporary registers)
REG_INPUT = 1          # v0-v1 (input color registers)
REG_CONST = 2          # c0-c7 (constant registers)
REG_TEXTURE = 3        # t0-t3 (texture registers)

def parse_register(reg_str):
    """Parse a register string like 'r0', 'v0', 't0', 'c0' and return (type, index, write_mask)"""
    reg_str = reg_str.strip()
    
    # Handle write masks like r0.rgb, r0.a
    write_mask = 0xF  # Default: all components (xyzw / rgba)
    if '.' in reg_str:
        reg_part, mask_part = reg_str.split('.')
        if mask_part == 'rgb':
            write_mask = 0x7  # xyz only
        elif mask_part == 'a':
            write_mask = 0x8  # w only
        elif mask_part == 'rgba':
            write_mask = 0xF  # all
        reg_str = reg_part
    
    reg_type = reg_str[0]
    reg_index = int(reg_str[1:])
    
    type_map = {'r': REG_TEMP, 'v': REG_INPUT, 'c': REG_CONST, 't': REG_TEXTURE}
    return (type_map[reg_type], reg_index, write_mask)

def encode_register(reg_type, reg_index, write_mask=0xF):
    """Encode a register into D3D8 format"""
    # Simple encoding: type in upper bits, index in lower bits
    # This is a simplified version - actual D3D8 encoding is more complex
    return (write_mask << 16) | (reg_type << 8) | reg_index

def assemble_instruction(line):
    """Assemble a single PS 1.1 instruction into bytecode"""
    line = line.strip()
    
    # Skip empty lines and comments
    if not line or line.startswith(';') or line.startswith('//'):
        return []
    
    # Handle co-issue (+instruction)
    coissue = line.startswith('+')
    if coissue:
        line = line[1:].strip()
    
    # Split instruction and operands
    parts = re.split(r'[,\s]+', line)
    instr = parts[0].lower()
    
    # Handle version declaration
    if instr == 'ps.1.1':
        # PS 1.1 version token: 0xFFFF0101
        return [struct.pack('<I', 0xFFFF0101)]
    
    # Get opcode
    if instr not in PS_OPCODES:
        print(f"Warning: Unknown instruction '{instr}', skipping")
        return []
    
    opcode = PS_OPCODES[instr]
    if coissue:
        opcode |= 0x40000000  # Set co-issue bit
    
    # Encode operands based on instruction
    bytecode = [struct.pack('<I', opcode)]
    
    if instr == 'tex':
        # tex tn - texture load
        if len(parts) >= 2:
            reg_type, reg_idx, write_mask = parse_register(parts[1])
            bytecode.append(struct.pack('<I', encode_register(reg_type, reg_idx, write_mask)))
    
    elif instr == 'texbem':
        # texbem tn, tm - texture load with bump env map
        if len(parts) >= 3:
            dst_type, dst_idx, dst_mask = parse_register(parts[1])
            src_type, src_idx, src_mask = parse_register(parts[2])
            bytecode.append(struct.pack('<I', encode_register(dst_type, dst_idx, dst_mask)))
            bytecode.append(struct.pack('<I', encode_register(src_type, src_idx, src_mask)))
    
    elif instr in ['mul', 'add']:
        # mul/add dst, src1, src2
        if len(parts) >= 4:
            dst_type, dst_idx, dst_mask = parse_register(parts[1])
            src1_type, src1_idx, src1_mask = parse_register(parts[2])
            src2_type, src2_idx, src2_mask = parse_register(parts[3])
            bytecode.append(struct.pack('<I', encode_register(dst_type, dst_idx, dst_mask)))
            bytecode.append(struct.pack('<I', encode_register(src1_type, src1_idx, src1_mask)))
            bytecode.append(struct.pack('<I', encode_register(src2_type, src2_idx, src2_mask)))
    
    elif instr == 'mad':
        # mad dst, src1, src2, src3 - multiply-add
        if len(parts) >= 5:
            dst_type, dst_idx, dst_mask = parse_register(parts[1])
            src1_type, src1_idx, src1_mask = parse_register(parts[2])
            src2_type, src2_idx, src2_mask = parse_register(parts[3])
            src3_type, src3_idx, src3_mask = parse_register(parts[4])
            bytecode.append(struct.pack('<I', encode_register(dst_type, dst_idx, dst_mask)))
            bytecode.append(struct.pack('<I', encode_register(src1_type, src1_idx, src1_mask)))
            bytecode.append(struct.pack('<I', encode_register(src2_type, src2_idx, src2_mask)))
            bytecode.append(struct.pack('<I', encode_register(src3_type, src3_idx, src3_mask)))
    
    return bytecode

def assemble_shader(shader_text):
    """Assemble PS 1.1 shader assembly into bytecode"""
    lines = shader_text.split('\n')
    bytecode = []
    
    for line in lines:
        # Remove comments
        if ';' in line:
            line = line[:line.index(';')]
        line = line.strip()
        
        if not line:
            continue
        
        instr_bytecode = assemble_instruction(line)
        bytecode.extend(instr_bytecode)
    
    # Add end token
    bytecode.append(struct.pack('<I', 0x0000FFFF))
    
    return b''.join(bytecode)

def generate_header(shader_files, output_file):
    """Generate C++ header file with precompiled shaders"""
    header_guard = "D3DX_PRECOMPILED_SHADERS_H"
    
    with open(output_file, 'w') as f:
        f.write(f"""/*
 * Precompiled D3D8 Pixel Shader 1.1 Bytecode
 * 
 * This file contains precompiled bytecode for water rendering shaders.
 * Generated by compile_shaders.py from .psh source files.
 * 
 * Do not edit this file manually - regenerate from source .psh files.
 */

#ifndef {header_guard}
#define {header_guard}

#include <windows.h>

namespace PrecompiledShaders {{

""")
        
        for i, shader_file in enumerate(shader_files, 1):
            shader_name = Path(shader_file).stem
            print(f"Assembling {shader_file}...")
            
            with open(shader_file, 'r') as sf:
                shader_text = sf.read()
            
            bytecode = assemble_shader(shader_text)
            
            # Write bytecode array
            f.write(f"// {shader_name} - from {Path(shader_file).name}\n")
            f.write(f"constexpr DWORD {shader_name}_bytecode[] = {{\n")
            
            # Write bytecode as hex DWORDs
            dwords = [bytecode[i:i+4] for i in range(0, len(bytecode), 4)]
            for j, dword in enumerate(dwords):
                if len(dword) == 4:
                    value = struct.unpack('<I', dword)[0]
                    f.write(f"    0x{value:08X},")
                    if j % 4 == 3:
                        f.write("\n")
                    else:
                        f.write(" ")
            
            f.write(f"\n}};\n\n")
            f.write(f"constexpr size_t {shader_name}_size = sizeof({shader_name}_bytecode);\n\n")
        
        f.write("""} // namespace PrecompiledShaders

#endif // """ + header_guard + "\n")
    
    print(f"Generated {output_file}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 compile_shaders.py <shader1.psh> [shader2.psh ...]")
        print("\nExample:")
        print("  python3 compile_shaders.py shaders/water_shader1.psh shaders/water_shader2.psh shaders/water_shader3.psh")
        sys.exit(1)
    
    shader_files = sys.argv[1:]
    output_file = "PrecompiledShaders.h"
    
    # Verify all input files exist
    for shader_file in shader_files:
        if not Path(shader_file).exists():
            print(f"Error: Shader file '{shader_file}' not found")
            sys.exit(1)
    
    generate_header(shader_files, output_file)
    print(f"\nSuccess! Precompiled shaders written to {output_file}")
    print("Include this header in your D3DXCompat.h or shader loading code.")

if __name__ == '__main__':
    main()
