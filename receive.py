import serial
import json

# Adjust the serial port to match your Arduino (e.g., COM3 on Windows, /dev/ttyUSB0 on Linux)
ser = serial.Serial("COM3", 115200, timeout=1)

while True:
    try:
        # Read a line from Serial
        line = ser.readline().decode("utf-8").strip()

        if line:
            # Parse JSON
            data = json.loads(line)
            thrust = data["Thrust"]

            # Print received thrust values
            print("Received thrust:", thrust)
    except json.JSONDecodeError:
        print("Invalid JSON received:", line)
    except KeyboardInterrupt:
        print("Exiting...")
        ser.close()
        break
