#!/usr/bin/env python3

import sys
from PIL import Image

def rgb565_conversion(input_path, output_path):
    try:
        img = Image.open(input_path).convert('RGB')
        width, height = img.size
        rgb565_data = []

        for y in range(height):
            for x in range(width):
                r, g, b = img.getpixel((x, y))
                r = (r >> 3) & 0x1F
                g = (g >> 2) & 0x3F
                b = (b >> 3) & 0x1F
                rgb565 = (r << 11) | (g << 5) | b
                rgb565_data.append(rgb565)

        with open(output_path, 'wb') as f:
            for pixel in rgb565_data:
                f.write(pixel.to_bytes(2, byteorder='big'))

        print(f"Conversion to RGB565 complete. File saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found.")
    except Exception as e:
        print(f"An error occurred during RGB565 conversion: {e}")

def argb8888_conversion(input_path, output_path):
    try:
        img = Image.open(input_path).convert('RGBA')
        width, height = img.size
        argb8888_data = []

        for y in range(height):
            for x in range(width):
                r, g, b, a = img.getpixel((x, y))
                argb8888 = (a << 24) | (r << 16) | (g << 8) | b
                argb8888_data.append(argb8888)

        with open(output_path, 'wb') as f:
            for pixel in argb8888_data:
                f.write(pixel.to_bytes(4, byteorder='big'))

        print(f"Conversion to ARGB8888 complete. File saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found.")
    except Exception as e:
        print(f"An error occurred during ARGB8888 conversion: {e}")

def grayscale8_conversion(input_path, output_path):
    try:
        img = Image.open(input_path).convert('L')  # Convert to 8-bit grayscale
        width, height = img.size
        grayscale8_data = list(img.getdata())

        with open(output_path, 'wb') as f:
            f.write(bytes(grayscale8_data))

        print(f"Conversion to 8-bit grayscale complete. File saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found.")
    except Exception as e:
        print(f"An error occurred during 8-bit grayscale conversion: {e}")

def grayscale4_conversion(input_path, output_path):
    try:
        img = Image.open(input_path).convert('L')  # Convert to 8-bit grayscale
        width, height = img.size
        grayscale_data = list(img.getdata())
        grayscale4_data = []

        for i in range(0, len(grayscale_data), 2):
            g1 = grayscale_data[i] >> 4  # Keep upper 4 bits
            g2 = grayscale_data[i+1] >> 4 if i+1 < len(grayscale_data) else 0  # Handle odd pixel count
            grayscale4_data.append((g1 << 4) | g2)

        with open(output_path, 'wb') as f:
            f.write(bytes(grayscale4_data))

        print(f"Conversion to 4-bit grayscale complete. File saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found.")
    except Exception as e:
        print(f"An error occurred during 4-bit grayscale conversion: {e}")

def argb1555_conversion(input_path, output_path):
    try:
        img = Image.open(input_path).convert('RGBA')  # Convert to RGBA to get alpha
        width, height = img.size
        argb1555_data = []

        for y in range(height):
            for x in range(width):
                r, g, b, a = img.getpixel((x, y))
                a = 1 if a >= 128 else 0  # Use 1-bit alpha threshold
                r = (r >> 3) & 0x1F
                g = (g >> 3) & 0x1F
                b = (b >> 3) & 0x1F
                argb1555 = (a << 15) | (r << 10) | (g << 5) | b
                argb1555_data.append(argb1555)

        with open(output_path, 'wb') as f:
            for pixel in argb1555_data:
                f.write(pixel.to_bytes(2, byteorder='big'))

        print(f"Conversion to ARGB1555 complete. File saved to: {output_path}")

    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found.")
    except Exception as e:
        print(f"An error occurred during ARGB1555 conversion: {e}")

def main():
    if len(sys.argv) != 4:
        print("Usage: convert_image.py <input_image> <output_file> <format>")
        print("Format options: rgb565, argb8888, grayscale8, grayscale4, argb1555")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    format_option = sys.argv[3].lower()

    if format_option == 'rgb565':
        rgb565_conversion(input_path, output_path)
    elif format_option == 'argb8888':
        argb8888_conversion(input_path, output_path)
    elif format_option == 'grayscale8':
        grayscale8_conversion(input_path, output_path)
    elif format_option == 'grayscale4':
        grayscale4_conversion(input_path, output_path)
    elif format_option == 'argb1555':
        argb1555_conversion(input_path, output_path)
    else:
        print("Error: Invalid format. Supported formats are 'rgb565', 'argb8888', 'grayscale8', 'grayscale4', and 'argb1555'.")
        sys.exit(1)

if __name__ == "__main__":
    main()
