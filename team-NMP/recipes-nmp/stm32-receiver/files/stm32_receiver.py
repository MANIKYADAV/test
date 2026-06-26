import serial
import time

# STM32 UART port on Raspberry Pi
ser = serial.Serial('/dev/serial0', 115200, timeout=1)

time.sleep(2)  # allow UART to initialize

print("Listening for STM32 data...\n")

while True:
    try:
        data = ser.readline().decode('utf-8', errors='ignore').strip()

        if data:
            print(data)

    except KeyboardInterrupt:
        print("\nStopped")
        break

ser.close()
