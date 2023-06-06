# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import win32gui


def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    print_hi('PyCharm')

# See PyCharm help at https://www.jetbrains.com/help/pycharm/

import serial
import time
from datetime import datetime
from pmode import POWERBROAD_SETUP, get_display_state

ser = serial.Serial(
        port = 'COM8',
        baudrate = 115200,
        timeout = 10,
        xonxoff = False
)
ser.reset_output_buffer()
ser.reset_input_buffer()
val = 0
#command = ''
#command = input("Enter command: ")

POWERBROAD_SETUP()

while(True):
    if get_display_state() == "ON":
        now = datetime.now()
        command = now.strftime("%H:%M")

        val = ser.write(command.encode(encoding = 'ascii', errors = 'strict'))
        print("Waiting bytes: ", ser.in_waiting)
        print("Bytes written: ", val)
        in_data = ''
        #in_data = ser.read_until(b'}')
        in_data = ser.read(ser.in_waiting)
        print(in_data)

        time.sleep(10)

    win32gui.PumpWaitingMessages()
