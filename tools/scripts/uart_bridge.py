#!/usr/bin/env python3
"""
uart_bridge.py — UART COM1 → oo_voice_state.json bridge
Reads OO_VOICE:{...} lines from QEMU serial and writes JSON body
to oo_voice_state.json (read by preview_hud.py at 20 Hz).

Usage:
  python uart_bridge.py --port /tmp/qemu-serial.sock  (QEMU UNIX socket)
  python uart_bridge.py --port COM3                   (real hardware Windows)
  python uart_bridge.py --port /dev/ttyS0             (real hardware Linux)
  python uart_bridge.py --demo                        (generate fake state for testing)

Dependencies: pyserial (pip install pyserial)
"""
import argparse, json, os, sys, time, socket, random, math

VOICE_STATE_FILE = os.path.join(os.path.dirname(__file__),
                                "..", "..", "desktop_display", "oo_voice_state.json")
VOICE_STATE_FILE = os.path.normpath(VOICE_STATE_FILE)

PREFIX = "OO_VOICE:"

EMOTION_CYCLE = [
    "FOCUSED", "CURIOUS", "SPEAKING", "CAUTIOUS", "ALERT",
    "PROUD",   "DORMANT", "FOCUSED",
]

def write_state(state: dict):
    tmp = VOICE_STATE_FILE + ".tmp"
    with open(tmp, "w") as f:
        json.dump(state, f)
    os.replace(tmp, VOICE_STATE_FILE)

def parse_line(line: str):
    line = line.strip()
    if not line.startswith(PREFIX):
        return None
    body = line[len(PREFIX):]
    try:
        return json.loads(body)
    except Exception:
        return None

def demo_loop():
    print(f"[uart_bridge] DEMO MODE -> {VOICE_STATE_FILE}")
    t = 0
    while True:
        t += 1
        emotion = EMOTION_CYCLE[(t // 60) % len(EMOTION_CYCLE)]
        inference = (t // 80) % 4 != 0
        wake = (t % 300 == 0)
        waveform = [int((math.sin((t + i * 3) * math.pi / 32) * 0.5 + 0.5) * 255)
                    for i in range(64)]
        speed_mul = 2.5 if inference else 1.0
        state = {
            "emotion":          emotion,
            "vortex_speed_mul": speed_mul,
            "wake_pulse":       1 if wake else 0,
            "glitch":           8 if wake else 0,
            "inference":        inference,
            "waveform":         waveform,
            "response":         "Processing..." if inference and t % 120 == 0 else "",
        }
        write_state(state)
        time.sleep(1.0 / 20)

def serial_loop(port, baud):
    try:
        import serial
    except ImportError:
        print("ERROR: pyserial not installed. Run: pip install pyserial", file=sys.stderr)
        sys.exit(1)
    print(f"[uart_bridge] Opening serial {port} @ {baud} baud -> {VOICE_STATE_FILE}")
    with serial.Serial(port, baud, timeout=0.5) as ser:
        buf = ""
        while True:
            chunk = ser.read(256).decode("ascii", errors="replace")
            buf += chunk
            while "\n" in buf:
                line, buf = buf.split("\n", 1)
                state = parse_line(line)
                if state:
                    write_state(state)

def socket_loop(path):
    print(f"[uart_bridge] Connecting to QEMU socket {path} -> {VOICE_STATE_FILE}")
    while True:
        try:
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
                s.connect(path)
                buf = ""
                while True:
                    data = s.recv(1024).decode("ascii", errors="replace")
                    if not data:
                        break
                    buf += data
                    while "\n" in buf:
                        line, buf = buf.split("\n", 1)
                        state = parse_line(line)
                        if state:
                            write_state(state)
        except (ConnectionRefusedError, FileNotFoundError, AttributeError):
            time.sleep(1.0)

def main():
    ap = argparse.ArgumentParser(description="OO UART -> voice state bridge")
    ap.add_argument("--port",  default="", help="Serial port or UNIX socket path")
    ap.add_argument("--baud",  type=int, default=38400, help="Baud rate")
    ap.add_argument("--demo",  action="store_true", help="Generate demo state (no HW)")
    ap.add_argument("--out",   default="", help="Override output JSON path")
    args = ap.parse_args()

    global VOICE_STATE_FILE
    if args.out:
        VOICE_STATE_FILE = args.out

    os.makedirs(os.path.dirname(os.path.abspath(VOICE_STATE_FILE)), exist_ok=True)

    if args.demo:
        demo_loop()
    elif args.port.startswith("/tmp/") or args.port.endswith(".sock"):
        socket_loop(args.port)
    elif args.port:
        serial_loop(args.port, args.baud)
    else:
        print("Usage: uart_bridge.py --port <serial|socket> OR --demo")
        sys.exit(1)

if __name__ == "__main__":
    main()
