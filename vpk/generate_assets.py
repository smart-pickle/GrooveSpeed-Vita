import os
import struct
import zlib

def make_png(width, height, red, green, blue):
    # PNG signature
    png_signature = b'\x89PNG\r\n\x1a\n'
    
    # IHDR chunk
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
    ihdr_crc = zlib.crc32(b'IHDR' + ihdr_data)
    ihdr_chunk = struct.pack('>I', len(ihdr_data)) + b'IHDR' + ihdr_data + struct.pack('>I', ihdr_crc)
    
    # IDAT chunk (raw RGB image bytes)
    raw_bytes = bytearray()
    for y in range(height):
        raw_bytes.append(0) # Filter byte for scanline
        for x in range(width):
            # Gradient pattern
            r = int(red * (1.0 - (y / height) * 0.3))
            g = int(green * (1.0 - (x / width) * 0.2))
            b = int(blue * (0.8 + (y / height) * 0.2))
            raw_bytes.extend([min(255, max(0, r)), min(255, max(0, g)), min(255, max(0, b))])
            
    compressed_data = zlib.compress(raw_bytes)
    idat_crc = zlib.crc32(b'IDAT' + compressed_data)
    idat_chunk = struct.pack('>I', len(compressed_data)) + b'IDAT' + compressed_data + struct.pack('>I', idat_crc)
    
    # IEND chunk
    iend_crc = zlib.crc32(b'IEND')
    iend_chunk = struct.pack('>I', 0) + b'IEND' + struct.pack('>I', iend_crc)
    
    return png_signature + ihdr_chunk + idat_chunk + iend_chunk

script_dir = os.path.dirname(os.path.abspath(__file__))

# Generate icon0.png (128x128 - Cyan/Gold Dark Theme)
icon_png = make_png(128, 128, 38, 198, 218)
with open(os.path.join(script_dir, 'icon0.png'), 'wb') as f:
    f.write(icon_png)

# Generate bg.png (840x500 - Deep Obsidian Background)
bg_png = make_png(840, 500, 15, 20, 25)
with open(os.path.join(script_dir, 'bg.png'), 'wb') as f:
    f.write(bg_png)

print("Generated icon0.png and bg.png in", script_dir)
