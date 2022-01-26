import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

from io import StringIO
import csv
import numpy
from math import sqrt
from matplotlib import pyplot as plt
import serial
import time


def fit(basename):
    csvfile = basename + ".csv"
    fitfile = basename + ".fit"
    deltafile = basename + ".err"
    s = open(csvfile, mode='r').read()

    b = numpy.genfromtxt(StringIO(s), delimiter=",")

    r = b[:, 3]
    l = b[:, 2]
    db = b[:, 1]
    rv = b[:, 0]

    for i in range(18):
        d = (r + l) / 2
        data = numpy.polyfit(d, db, i)
        delta = numpy.polyval(data, d) - db
        deltaerror = sqrt(numpy.dot(delta, delta)) / delta.size
        numpy.savetxt(fitfile + str(i), data, delimiter=",")
        numpy.savetxt(deltafile, delta, delimiter=",")
        plt.plot(d, delta, linewidth=1)
        plt.title(str(i) + " - " + str(deltaerror))
        plt.show()


def readserial():
    serialPort = serial.Serial(
        port="COM4", baudrate=115200, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
    )

    cal_list = []

    serialString = ""  # Used to hold data coming over UART


    while 1:
        if serialPort.in_waiting > 0:
            # Read data out of the buffer until a carraige return / new line is found
            serialString = serialPort.readline()
            try:
                sstr = serialString.decode("Ascii").strip()
                if sstr == "Prompt>":
                    print("Prompt>")
                    serialPort.write("RVSweep".encode())
                    break
                else:
                    exit()
            except:
                pass
    while 1:
        # Wait until there is data waiting in the serial buffer
        if serialPort.in_waiting > 0:

            # Read data out of the buffer until a carraige return / new line is found
            serialString = serialPort.readline()

            # Print the contents of the serial data
            try:
                sstr = serialString.decode("Ascii").strip()

                if sstr != "Finished":
                    cal_list.append(sstr)
                    print(sstr)
                else:
                    break
            except:
                pass

    textfile = open('cal.csv', "wb")
    for element in cal_list:
        textfile.write((element + "\n").encode('utf-8'))
    textfile.close()


readserial()
# fit('cal')

# fit("RV44")
# fit("RV45")
# fit("RV99")
# fit("RV194")
# fit("RV254")
