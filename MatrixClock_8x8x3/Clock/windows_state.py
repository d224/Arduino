import ctypes
import threading
import uuid

import win32api
import win32con
import win32gui

# -------------------------------------------------------------------
# Windows constants
# -------------------------------------------------------------------
WM_POWERBROADCAST = 0x0218
PBT_POWERSETTINGCHANGE = 0x8013

DEVICE_NOTIFY_WINDOW_HANDLE = 0x00000000

# Console display power state:
# 0 = off
# 1 = on
# 2 = dimmed
GUID_CONSOLE_DISPLAY_STATE = uuid.UUID("6FE69556-704A-47A0-8F24-C28D936FDA47")

# -------------------------------------------------------------------
# ctypes structures
# -------------------------------------------------------------------
class GUID(ctypes.Structure):
    _fields_ = [
        ("Data1", ctypes.c_uint32),
        ("Data2", ctypes.c_uint16),
        ("Data3", ctypes.c_uint16),
        ("Data4", ctypes.c_ubyte * 8),
    ]

    @classmethod
    def from_uuid(cls, u):
        if isinstance(u, str):
            u = uuid.UUID(u)
        data4 = (ctypes.c_ubyte * 8)(*u.bytes[8:])
        return cls(
            u.time_low,
            u.time_mid,
            u.time_hi_version,
            data4,
        )

    def to_uuid(self):
        raw = bytes(
            ctypes.string_at(ctypes.byref(self), ctypes.sizeof(self))
        )
        return uuid.UUID(bytes_le=raw)

    def __eq__(self, other):
        if isinstance(other, GUID):
            return self.to_uuid() == other.to_uuid()
        if isinstance(other, uuid.UUID):
            return self.to_uuid() == other
        return False

class POWERBROADCAST_SETTING_HDR(ctypes.Structure):
    _fields_ = [
        ("PowerSetting", GUID),
        ("DataLength", ctypes.c_uint32),
    ]

# -------------------------------------------------------------------
# State
# -------------------------------------------------------------------
_state_lock = threading.Lock()
_display_on = True
_started = False
_ready = threading.Event()

_console_display_guid = GUID.from_uuid(GUID_CONSOLE_DISPLAY_STATE)

def _set_display_on(value: bool):
    global _display_on
    with _state_lock:
        _display_on = bool(value)

def get_display_state():
    """
    Returns:
        "ON"  -> monitor is on or dimmed
        "OFF" -> monitor is off / asleep
    """
    with _state_lock:
        return "ON" if _display_on else "OFF"

def _handle_power_broadcast(lparam):
    """
    Handle WM_POWERBROADCAST / PBT_POWERSETTINGCHANGE for
    GUID_CONSOLE_DISPLAY_STATE.

    0 = off
    1 = on
    2 = dimmed
    """
    try:
        hdr = POWERBROADCAST_SETTING_HDR.from_address(lparam)
        if hdr.PowerSetting == _console_display_guid:
            data_addr = lparam + ctypes.sizeof(POWERBROADCAST_SETTING_HDR)
            state = ctypes.c_uint32.from_address(data_addr).value

            # Treat dimmed as ON
            _set_display_on(state != 0)

            print(f"windows_state: display state = {'ON' if state != 0 else 'OFF'}")
    except Exception as e:
        print(f"windows_state: power broadcast parse error: {e}")

def _wndproc(hwnd, msg, wparam, lparam):
    if msg == WM_POWERBROADCAST and wparam == PBT_POWERSETTINGCHANGE:
        _handle_power_broadcast(lparam)
        return 1

    if msg == win32con.WM_DESTROY:
        win32gui.PostQuitMessage(0)
        return 0

    return win32gui.DefWindowProc(hwnd, msg, wparam, lparam)

def start_monitor():
    """
    Start a hidden message window that listens for monitor power changes.
    Safe to call multiple times.
    """
    global _started
    if _started:
        return

    _started = True

    def worker():
        try:
            hinst = win32api.GetModuleHandle(None)
            class_name = "OledClockDisplayMonitor"

            wndclass = win32gui.WNDCLASS()
            wndclass.hInstance = hinst
            wndclass.lpszClassName = class_name
            wndclass.lpfnWndProc = _wndproc

            try:
                win32gui.RegisterClass(wndclass)
            except win32gui.error:
                pass

            hwnd = win32gui.CreateWindow(
                class_name,
                class_name,
                0,
                0, 0, 0, 0,
                0, 0,
                hinst,
                None
            )

            try:
                user32 = ctypes.windll.user32
                user32.RegisterPowerSettingNotification(
                    hwnd,
                    ctypes.byref(_console_display_guid),
                    DEVICE_NOTIFY_WINDOW_HANDLE
                )
            except Exception as e:
                print(f"windows_state: power notification registration failed: {e}")

            _ready.set()
            win32gui.PumpMessages()

        except Exception as e:
            print(f"windows_state: monitor thread failed: {e}")
            _ready.set()

    threading.Thread(target=worker, daemon=True).start()
    _ready.wait(timeout=5)

# Start automatically on import
start_monitor()
