import threading
import time
import os
import serial
import serial.tools.list_ports
ports = serial.tools.list_ports.comports()
# device = serial.Serial()


def listPorts():  # returns an array of available serial ports
    print("\n\nScanning ports...")
    print(" Available ports:")
    devices = []
    for element in ports:
        devices.append(element.device)
    return devices


def getUserPort():
    retry = True
    portsList = []
    portsList = listPorts()
    for i in range(0, len(portsList)):
        print(" ", i, ": " + portsList[i])
    userChoice = input("Which port# do you want to use: ")
    print()

    while(retry is True):  # keep requesting port until valid
        try:
            int(userChoice)  # check if user typed index or name
            if(int(userChoice) < len(portsList)):
                retry = False
                return (portsList[int(userChoice)])  # return valid port name
            else:
                retry = False
        except ValueError:
            for element in portsList:
                if element == userChoice:  # check for valid port name
                    retry = True
                    return element
            retry = False
        if(retry is False):
            userChoice = input("Invalid port#, please re-enter: ")
            print()
    return ("invalid")  # all failed


def deviceWrite(device):  # thread to write to serial device

    # arr = os.listdir()
    # print(arr)
    # cwd = os.getcwd()
    # print("Current working directory: " + cwd)
    filePath = "Debug/upgrade2.bin"
    try:
        fileSize = os.path.getsize(filePath)
    except FileNotFoundError:
        print("File not found")
        # exit()
        os._exit(1)
    binFile = open(filePath, "rb")
    ba = bytearray(binFile.read())
    # for byte in ba:
    #     print(byte)

    # exit(1)

    flashFileName = input("What do you want to name the file: ")
    print()
    string = "vi " + flashFileName + " " + str(fileSize) + "\n"
    device.write(string.encode())

    time.sleep(1)

    device.write(ba)
    print("done")
    time.sleep(1)
    os._exit(1)

    while(True):
        string = input()  # get user input
        device.write(string.encode() + b'\n')  # write to device


def deviceRead(device):  # thread to read from serial device
    while(device.isOpen()):
        while(device.in_waiting):  # check if data available
            string = device.readline()  # read a line
            print(string.decode(), end='')  # display data


if __name__ == '__main__':
    userPort = getUserPort()
    # device = serial.Serial(userPort, 115200)
    try:
        device = serial.Serial(userPort, 115200, timeout=1)
    except serial.SerialException:
        print("Port not found; maybe open elsewhere")
        os._exit(1)
    if(not device.isOpen()):
        device.open()

    if(device.isOpen()):
        print("Port is open")
        deviceWriteThread = threading.Thread(  # create device read thread
            target=deviceWrite, args=(device,))
        deviceReadThread = threading.Thread(  # create device write thread
            target=deviceRead, args=(device,))
        deviceWriteThread.start()  # start write thread
        deviceReadThread.start()  # start read thread
    else:
        print("Port is not open")
        exit(1)
