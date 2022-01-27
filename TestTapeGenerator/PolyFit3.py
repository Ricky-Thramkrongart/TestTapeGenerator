from io import StringIO
import numpy
from math import sqrt
from matplotlib import pyplot as plt
import serial


def write_array(hfile, data):
    s = open(hfile, mode='wb')
    s.write('    std::vector <float64_t> fit64(14);\r\n'.encode())
    i = len(data)-1
    for d in data:
        s.write(str('    fit64[' + str(i) + '] = fp64_atof("' + str(d) + '")\r\n').encode())
        i -= 1
    s.close()


def fit(basename):
    hfile = basename + ".h"
    csvfile = basename + ".csv"
    fitfile = basename + ".fit"
    deltafile = basename + ".err"
    s = open(csvfile, mode='r').read()

    b = numpy.genfromtxt(StringIO(s), delimiter=",")

    r = b[:, 3]
    l = b[:, 2]
    db = b[:, 1]
    rv = b[:, 0]
    current_deltaerror = 1000000.0


    for i in range(18):
        d = (r + l) / 2
        data = numpy.polyfit(d, db, i)
        delta = numpy.polyval(data, d) - db
        deltaerror = sqrt(numpy.dot(delta, delta)) / delta.size
        error_change = (current_deltaerror-deltaerror)/current_deltaerror
        current_deltaerror = deltaerror
        if (error_change < 0.1):
            numpy.savetxt(fitfile, data, delimiter=",")
            numpy.savetxt(deltafile, delta, delimiter=",")
            write_array(hfile, data)
            print(str(i) + " - " + str(deltaerror) + " - " + str(error_change))
            break
        #plt.plot(d, delta, linewidth=1)
        #plt.title(str(i) + " - " + str(deltaerror))
        #plt.show()


def readserial(basename):
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

    textfile = open(csvfile, "wb")
    for element in cal_list:
        textfile.write((element + "\n").encode('utf-8'))
    textfile.close()

def get_array(b, val):
    for row in b:
        if (row[0] == val):
            if 'a' in locals():
                a = numpy.vstack([a, row])
            else:
                a = row
    return a

def remove_last_value(b):
    repeat_count = 0
    for row in reversed(b[0:b.shape[0] - 1]):
        if b[-1][2] == row[2] or b[-1][3] == row[3]:
            repeat_count += 1
    if repeat_count > 1:
        return b[0:-repeat_count]
    else:
        return b

def filtercsv(basename):
    csvfile = basename + ".csv"
    s = open(csvfile, mode='r').read()
    b = numpy.genfromtxt(StringIO(s), delimiter=",")

    rvs = numpy.unique(b[0:-1, 0:1])
    for rv in rvs:
        f = remove_last_value(get_array(b, rv))
        filteredcsvfile = basename + str(int(rv)) + ".csv"

        fmt = '%d,%1.2f,%d,%d'
        numpy.savetxt(filteredcsvfile, f, delimiter=",", fmt=fmt)
    return rvs

#readserial('rv')
rvs = filtercsv('rv')
for rv in rvs:
    fit("rv" + str(int(rv)))
