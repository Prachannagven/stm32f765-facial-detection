import serial
import numpy as np
from PIL import Image

# ============================================================
# CONFIGURATION
# ============================================================

PORT = "/dev/ttyACM1"      # Linux
# PORT = "COM5"           # Windows

BAUDRATE = 115200

WIDTH = 320
HEIGHT = 240

FRAME_SIZE = WIDTH * HEIGHT * 2

# ============================================================
# OPEN SERIAL PORT
# ============================================================

print(f"Opening {PORT}...")

ser = serial.Serial(
    PORT,
    BAUDRATE,
    timeout=10
)

print("Waiting for frame...")

# ============================================================
# RECEIVE FRAME
# ============================================================

frame = bytearray()

while len(frame) < FRAME_SIZE:

    chunk = ser.read(FRAME_SIZE - len(frame))

    if not chunk:
        raise RuntimeError(
            f"Timeout after receiving {len(frame)} bytes"
        )

    frame.extend(chunk)

    print(
        f"\rReceived {len(frame):6d}/{FRAME_SIZE}",
        end="",
        flush=True
    )

print("\nFrame received!")

# ============================================================
# DEBUG
# ============================================================

print("First 32 bytes:")
print(frame[:32].hex())

# ============================================================
# RGB565 -> RGB888
# ============================================================

# OV7670 RGB565 interpreted as:
#
# Byte0 = MSB
# Byte1 = LSB
#
# RGB565:
# RRRRRGGGGGGBBBBB

data = np.frombuffer(frame, dtype=">u2")

r = (data >> 11) & 0x1F
g = (data >> 5) & 0x3F
b = data & 0x1F

# Expand to full 8-bit precision

r = (r << 3) | (r >> 2)
g = (g << 2) | (g >> 4)
b = (b << 3) | (b >> 2)

rgb = np.stack((r, g, b), axis=-1).astype(np.uint8)

rgb = rgb.reshape((HEIGHT, WIDTH, 3))

# ============================================================
# SAVE IMAGE
# ============================================================

img = Image.fromarray(rgb, "RGB")

img.save("capture.png")

print("Saved capture.png")

img.show()
