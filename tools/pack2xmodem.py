#!/usr/bin/env python3

import sys
import argparse
import functools

PKT_DATA_BYTES = 128
XMODEM_SOH     = 0x01
XMODEM_PAD     = 0x1A
XMODEM_EOT     = 0x04

def next_packet_num(pkt_num : int) -> int:
    pkt_num += 1
    return 1 if pkt_num > 255 else pkt_num

def xmodem_cksum(pkt_data_iter) -> int:
    return -(functools.reduce(lambda v, e: v + sum(e), pkt_data_iter, 0)) % 256

def main():
    parser = argparse.ArgumentParser(
        prog='pack2xmodem',
        description='Turn a file into a sequence of XMODEM packets'
    )
    parser.add_argument("-i", "--input", action="store", help="input file", required=True)
    parser.add_argument("-o", "--output", action="store", help="output file", required=True)
    args = parser.parse_args()

    pkt_num = 1

    with open(args.input, "rb") as ihex_file, open(args.output, "wb") as xmodem_file:
        while True:
            header = bytes((XMODEM_SOH, pkt_num, 255 - pkt_num))

            pkt_data = ihex_file.read(PKT_DATA_BYTES)
            pad_bytes = bytes((XMODEM_PAD,)) * (PKT_DATA_BYTES - len(pkt_data))

            xmodem_file.write(header)
            xmodem_file.write(pkt_data)
            xmodem_file.write(pad_bytes)
            xmodem_file.write(bytes((xmodem_cksum((header, pkt_data, pad_bytes)),)))

            pkt_num = next_packet_num(pkt_num)

            if len(pad_bytes) > 0:
                xmodem_file.write(bytes((XMODEM_EOT,)))
                break
         
if __name__ == "__main__":
    main()
