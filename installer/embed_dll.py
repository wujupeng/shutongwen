# -*- coding: utf-8 -*-
import sys

def generate_embedded_header(dll_path, output_header):
    with open(dll_path, 'rb') as f:
        dll_data = f.read()
    
    size = len(dll_data)
    lines = []
    
    lines.append('#pragma once')
    lines.append('')
    lines.append('extern "C" const unsigned char ShuTongWenIME_dll[] = {')
    
    for i in range(0, size, 16):
        chunk = dll_data[i:i+16]
        hex_values = ', '.join(f'0x{byte:02x}' for byte in chunk)
        lines.append(f'    {hex_values},')
    
    lines.append('};')
    lines.append(f'extern "C" const size_t ShuTongWenIME_dll_size = {size};')
    
    with open(output_header, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))
    
    print(f"Generated {output_header} with {size} bytes")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python embed_dll.py <dll_path> <output_header>")
        sys.exit(1)
    
    generate_embedded_header(sys.argv[1], sys.argv[2])
