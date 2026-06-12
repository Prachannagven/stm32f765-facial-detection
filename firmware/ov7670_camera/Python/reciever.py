import serial
import os
from datetime import datetime as dt
import numpy as np
from PIL import Image

# ============================================================
# CONFIGURATION
# ============================================================

PORT = "/dev/ttyACM1"      # Linux
# PORT = "COM5"           # Windows

BAUDRATE = 115200

WIDTH = 640
HEIGHT = 240

FRAME_SIZE = WIDTH * HEIGHT * 2

INIT_MESSAGE = "Sending Frame!!!!!!"
INIT_MSG_LEN = len(INIT_MESSAGE)

RECIEVED_SIZE = FRAME_SIZE + INIT_MSG_LEN

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
# FIND FRAME START MARKER
# ============================================================

MARKER = b"!!!!!!"

print("Waiting for frame marker...")

window = bytearray()

while True:
    b = ser.read(1)

    if not b:
        raise RuntimeError("Timeout waiting for marker")

    window.extend(b)

    if len(window) > len(MARKER):
        window.pop(0)

    if bytes(window) == MARKER:
        print("Marker found")
        break

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

data = np.frombuffer(frame, dtype=">u2") # Big endian output, so no reversal of byte order
# data = np.frombuffer(frame, dtype="<u2")

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

t = dt.now().strftime("%Y%m%d_%H%M%S")
capture_dir = os.path.expanduser("~/PersonalProjects/kamikaze-drone/firmware/ov7670_camera/Python/captures")
os.makedirs(capture_dir, exist_ok=True)
img.save(os.path.join(capture_dir, f"{t}.png"))

print("Saved capture.png")

img.show()
