import sys
import time
import threading

from config import (
    MAIN_LOOP_DELAY_SECONDS,
    LOCK_POLL_DELAY_SECONDS,
    INITIAL_RECONNECT_DELAY_SECONDS,
    MAX_RECONNECT_DELAY_SECONDS
)
from windows_state import get_display_state
from serial_comm import connect_serial, safe_close_serial, send_time
from image_utils import load_icons
from tray_app import TrayApp

stop_event = threading.Event()
ser = None
tray = None

def menu_exit(icon_obj, _item):
    global ser
    stop_event.set()
    safe_close_serial(ser)
    ser = None
    try:
        icon_obj.stop()
    except Exception:
        pass

def menu_refresh(icon_obj, _item):
    global ser
    safe_close_serial(ser)
    ser = None
    tray.set_disconnected()

def run_tray():
    tray.run()

def main():
    global ser, tray

    image_link, image_unlink = load_icons()
    tray = TrayApp(image_link, image_unlink, menu_refresh, menu_exit)

    tray_thread = threading.Thread(target=run_tray, daemon=True)
    tray_thread.start()

    reconnect_delay = INITIAL_RECONNECT_DELAY_SECONDS

    try:
        while not stop_event.is_set():
            if get_display_state() == "OFF":
                if ser is not None:
                    safe_close_serial(ser)
                    ser = None
                tray.set_disconnected()
                stop_event.wait(LOCK_POLL_DELAY_SECONDS)
                continue

            if ser is None or not getattr(ser, "is_open", False):
                ser = connect_serial()
                if ser is None:
                    tray.set_disconnected()
                    stop_event.wait(reconnect_delay)
                    reconnect_delay = min(reconnect_delay * 2, MAX_RECONNECT_DELAY_SECONDS)
                    continue

                reconnect_delay = INITIAL_RECONNECT_DELAY_SECONDS
                tray.set_connected(getattr(ser, "port", "Connected"))

            if send_time(ser):
                tray.set_connected(getattr(ser, "port", "Connected"))
                stop_event.wait(MAIN_LOOP_DELAY_SECONDS)
            else:
                safe_close_serial(ser)
                ser = None
                tray.set_disconnected()
                stop_event.wait(reconnect_delay)
                reconnect_delay = min(reconnect_delay * 2, MAX_RECONNECT_DELAY_SECONDS)

    except KeyboardInterrupt:
        stop_event.set()

    finally:
        safe_close_serial(ser)
        ser = None
        try:
            tray.stop()
        except Exception:
            pass
        sys.exit(0)

if __name__ == "__main__":
    main()
