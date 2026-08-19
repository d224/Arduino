import time
from datetime import datetime

import serial
from serial.tools import list_ports

from config import BAUDRATE, SERIAL_TIMEOUT, WRITE_WAIT_SECONDS
from day_night import get_command

def safe_close_serial(port):
    try:
        if port is not None:
            port.close()
    except Exception:
        pass

def drain_input_buffer(ser_port):
    try:
        while True:
            pending = getattr(ser_port, "in_waiting", 0)
            if pending <= 0:
                break
            data = ser_port.read(pending)
            print(f"Drained [{pending}]: {data!r}")
            time.sleep(0.05)
    except Exception as e:
        print(f"Drain error: {e}")

def send_time(ser_port):
    if ser_port is None:
        return False

    try:
        if not ser_port.is_open:
            return False

        time.sleep(WRITE_WAIT_SECONDS)
        
        drain_input_buffer(ser_port)

        now = datetime.now().astimezone()
        command = get_command(now)

        written = ser_port.write(command.encode("ascii"))
        ser_port.flush()
        print(f"Out [{written}]: {command}")

        time.sleep(WRITE_WAIT_SECONDS)

        in_waiting = getattr(ser_port, "in_waiting", 0)
        if in_waiting > 0:
            in_data = ser_port.read(in_waiting)
        else:
            in_data = ser_port.read_all()

        print(f"In  [{len(in_data)}]: {in_data!r}")
        return in_data.strip() == b"OK"

    except (OSError, serial.SerialException) as e:
        print(f"Serial error in send_time: {e}")
        return False
    except Exception as e:
        print(f"Exception in send_time: {e}")
        return False

def list_candidate_ports():
    ports = [p.device for p in list_ports.comports()]
    if not ports:
        ports = [f"COM{i}" for i in range(1, 17)]
    return ports

def connect_serial():
    for port_name in list_candidate_ports():
        print("Try:", port_name)
        candidate = None

        try:
            candidate = serial.Serial()
            candidate.port = port_name
            candidate.baudrate = BAUDRATE
            candidate.timeout = SERIAL_TIMEOUT
            candidate.write_timeout = SERIAL_TIMEOUT
            candidate.xonxoff = False
            candidate.rtscts = False
            candidate.dsrdtr = False

            # 2. Windows specific safety configurations BEFORE opening
            candidate.dtr = False  
            candidate.rts = False

            # 3. Open the port safely
            candidate.open()

            # 4. CRUCIAL FOR WINDOWS: Allow the OS driver thread to settle
            time.sleep(0.1) 

            # 5. Reinforce the low state so the hardware EN pin stays pulled high
            candidate.dtr = False
            candidate.rts = False

            if send_time(candidate):
                print(f"Connected on {port_name}")
                return candidate

            safe_close_serial(candidate)

        except (OSError, serial.SerialException) as e:
            print(f"Port {port_name} failed: {e}")
            safe_close_serial(candidate)

        except Exception as e:
            print(f"Unexpected error on {port_name}: {e}")
            safe_close_serial(candidate)

    return None
