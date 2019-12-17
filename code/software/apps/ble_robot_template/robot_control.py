#!/usr/bin/env python3
import struct
import time
import keyboard
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate
import argparse

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
parser.add_argument('addr', metavar='A', type=str, help='Address of the form XX:XX:XX:XX:XX:XX')
args = parser.parse_args()
addr = args.addr.lower()
if len(addr) != 17:
    raise ValueError("Invalid address supplied")

SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = "4607eda1-f65e-4d59-a9ff-84420d87a4ca" # TODO: add your characteristics

class RobotController():

    def __init__(self, address):

        self.robot = Peripheral(addr)
        print("connected")

        # keep state for keypresses
        self.pressed = {"up": False, "down": False, "right": False, "left": False, 
        "d": False, "s": False, "a": False, "c": False, "o": False, "p": False, "h": False}
        # TODO get service from robot
        # TODO get characteristic handles from service/robot
        # TODO enable notifications if using notifications

        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)
        self.ch = self.sv.getCharacteristics(CHAR_UUIDS)[0]

        keyboard.hook(self.on_key_event)

    def on_key_event(self, event):
        # print key name
        print(event.name)
        # if a key unrelated to direction keys is pressed, ignore
        if event.name not in self.pressed: return
        # if a key is pressed down
        if event.event_type == keyboard.KEY_DOWN:
            # if that key is already pressed down, ignore
            if self.pressed[event.name]: return
            # set state of key to pressed
            self.pressed[event.name] = True
            # TODO write to characteristic to change direction
            num = 0
            if (self.pressed["h"]): num = 11
            elif (self.pressed["p"]): num = 22
            elif (self.pressed["o"]): num = 33
            else:
                num = int(self.pressed["c"])
                num = num * 2 + int(self.pressed["a"])
                num = num * 2 + int(self.pressed["s"])
                num = num * 2 + int(self.pressed["d"])
                num = num * 2 + int(self.pressed["up"])
                num = num * 2 + int(self.pressed["down"])
                num = num * 2 + int(self.pressed["right"])
                num = num * 2 + int(self.pressed["left"])
            self.ch.write(bytes([num]))
        else:
            # set state of key to released
            self.pressed[event.name] = False
            # TODO write to characteristic to stop moving in this direction
            num = 0
            if (self.pressed["h"]): num = 11
            elif (self.pressed["p"]): num = 22
            elif (self.pressed["o"]): num = 33
            else:
                num = int(self.pressed["c"])
                num = num * 2 + int(self.pressed["a"])
                num = num * 2 + int(self.pressed["s"])
                num = num * 2 + int(self.pressed["d"])
                num = num * 2 + int(self.pressed["up"])
                num = num * 2 + int(self.pressed["down"])
                num = num * 2 + int(self.pressed["right"])
                num = num * 2 + int(self.pressed["left"])
            self.ch.write(bytes([num]))

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr) as robot:
    getpass('Use arrow keys to control robot')