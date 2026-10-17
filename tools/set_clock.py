#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Set EVA's offline RTC to this computer's local wall time, without resetting it."""
import argparse
from datetime import datetime
import sys
import time

def exchange(port, command, prefix, timeout=3):
    port.write((command+'\n').encode('ascii'))
    deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        line=port.readline().decode('ascii', errors='replace').strip()
        if line.startswith('ERR '):
            raise RuntimeError(line)
        if line.startswith(prefix):
            return line
    raise RuntimeError('No reply from EVA. Close other serial monitors and check the selected port.')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',required=True,help='Explicit EVA serial port, e.g. /dev/cu.usbmodem… or COM5')
    parser.add_argument('--read',action='store_true',help='Read the clock without changing it')
    args=parser.parse_args()
    import serial
    # Configure modem signals before opening so BOOT/reset are not intentionally asserted.
    port=serial.Serial(port=None,baudrate=115200,timeout=0.2,write_timeout=2)
    port.dtr=False;port.rts=False;port.port=args.port
    with port:
        identity=exchange(port,'INFO','EVA ')
        if 'BOARD=WAVESHARE-ESP32-S3-RLCD-4.2' not in identity:
            raise RuntimeError('This port did not identify itself as the EVA Waveshare firmware.')
        print(identity)
        if not args.read:
            now=datetime.now()
            if not 2000<=now.year<=2099:
                raise RuntimeError('RTC supports years 2000–2099.')
            stamp=now.strftime('%Y-%m-%dT%H:%M:%S')
            ack=exchange(port,'TIME '+stamp,'OK TIME ')
            if ack!='OK TIME '+stamp:
                raise RuntimeError('RTC acknowledgement did not match the requested time.')
            print(ack)
        value=exchange(port,'TIME?','TIME ')
        actual=datetime.strptime(value[5:],'%Y-%m-%dT%H:%M:%S')
        if not args.read and abs((actual-datetime.now()).total_seconds())>3:
            raise RuntimeError('RTC readback is too far from the computer clock.')
        print(value+' (local wall time; no automatic timezone/DST changes)')

if __name__=='__main__':
    try:
        main()
    except (RuntimeError, OSError, ValueError) as exc:
        print(f'Clock sync failed: {exc}',file=sys.stderr)
        sys.exit(1)
