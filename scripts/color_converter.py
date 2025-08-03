def hex_to_rgb565(hex_color):
    hex_color = hex_color.lstrip('#')
    r = int(hex_color[0:2], 16)
    g = int(hex_color[2:4], 16)
    b = int(hex_color[4:6], 16)

    r_565 = (r >> 3) & 0x1F
    g_565 = (g >> 2) & 0x3F
    b_565 = (b >> 3) & 0x1F

    return (r_565 << 11) | (g_565 << 5) | b_565

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        for hex_color in sys.argv[1:]:
            rgb565 = hex_to_rgb565(hex_color)
            print(f'Hex: {hex_color}, RGB565: {rgb565:#06x}')
    else:
        print("Usage: python color_converter.py <hex_color_1> <hex_color_2> ...")
