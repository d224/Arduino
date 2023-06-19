# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import win32gui

# See PyCharm help at https://www.jetbrains.com/help/pycharm/

import serial
import time
from datetime import datetime
from pmode import POWERBROAD_SETUP, get_display_state
import pystray
from pystray import Icon as icon, Menu as menu, MenuItem as item
from PIL import Image, Image, ImageDraw
import threading
import sys

bRun = True

# try:
#    ser = serial.Serial(
#            port = 'COM8',
#            baudrate = 115200,
#            timeout = 10,
#            xonxoff = False
#    )
#    ser.reset_output_buffer()
#    ser.reset_input_buffer()
#    val = 0
# except Exception as e:
#    print("Exception: %s" % str(e))

# command = ''
# command = input("Enter command: ")

POWERBROAD_SETUP()
image_link = Image.open("link.png")
image_unlink = Image.open("unlink.png")

def create_image(width, height, color1, color2):
    # Generate an image and draw a pattern
    image = Image.new('RGB', (width, height), color1)
    dc = ImageDraw.Draw(image)
    dc.rectangle((width // 2, 0, width, height // 2), fill=color2)
    dc.rectangle((0, height // 2, width // 2, height), fill=color2)
    return image


def send_time(ser_port):
    if ser_port is None:
        return False
    try:
        now = datetime.now()
        command = now.strftime("%H:%M")
        val = ser_port.write(command.encode(encoding='ascii', errors='strict'))
        print("Out [", val, "]: ", command)
        time.sleep(2)
        in_waiting = ser_port.in_waiting
        in_data = ser_port.read(in_waiting)
        print("In  [", in_waiting, "]: ", in_data)
        if in_data == b'OK':
            return True

    except Exception as e:
        print("Exception: %s" % str(e))

    return False


def serial_ports():
    ports = ['COM%s' % (i + 1) for i in range(16)]  # 256
    for port in ports:
        print("Try: ", port)
        try:
            ser = serial.Serial(
                port=port,
                baudrate=115200,
                timeout=10,
                xonxoff=False)

            if send_time(ser) == True:
                return ser
            ser.close()
        except (OSError, serial.SerialException):
            pass

    time.sleep(10)
    return None


# ser = serial_ports()
ser = None

# while serial_ports() == "" :
#    print("Not Detected ... :( ")
#    time.sleep(10)


def menu_Exit():
    print("menu_exit")
    icon.stop()
    global bRun
    bRun = False


def menu_Refresh():
    print("menu_Refresh")
    global ser
    ser = None
    #icon.title = "N.A"
    #serial_ports()
    #icon.title = ser.port


icon = pystray.Icon(name='Clock',
                    title="N.A",
                    icon=image_unlink,
                    menu=menu(item('Refresh', menu_Refresh),
                              item('Exit', menu_Exit)))


def function_icon_run():
    icon.run()


icon_thread = threading.Thread(target=function_icon_run, name="icon_run")
icon_thread.start()


def function_reconnect():
    icon.icon = image_unlink
    icon.title = "N.A."
    new_ser = serial_ports()
    if new_ser:
        print("Found", new_ser.port)
    else:
        print("Not Found")
    return new_ser


while bRun:
    if get_display_state() == "ON":
        try:
            if send_time(ser):
                icon.icon = image_link
                icon.title = ser.port
            else:
                ser = function_reconnect()

        except Exception as e:
            print("Exception: %s" % str(e))
            ser = function_reconnect()

        time.sleep(3)

    win32gui.PumpWaitingMessages()

sys.exit()