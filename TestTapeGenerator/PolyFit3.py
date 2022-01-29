from io import StringIO
import numpy
from math import sqrt
from matplotlib import pyplot as plt
import serial


def write_array(hfile, data, rv_, c):
    RV = str(int(rv_))
    s = open(hfile, mode='wb')
    s.write(str('            std::vector <float64_t> fit64RV' + RV + c + '(' + str(len(data)) + ');\r\n').encode())
    i = len(data) - 1
    for d in data:
        s.write(str('            fit64RV' + RV + c + '[' + str(i) + '] = fp64_atof("' + str(d) + '");\r\n').encode())
        i -= 1
    s.close()


def do_fit(basename, d, x, y, rv_):
    basename = basename + d
    hfile = basename + ".h"
    fitfile = basename + ".fit"
    deltafile = basename + ".err"

    current_deltaerror = 1000000.0
    for i in range(18):
        data = numpy.polyfit(x, y, i)
        delta = numpy.polyval(data, x) - y
        deltaerror = sqrt(numpy.dot(delta, delta)) / delta.size
        error_change = (current_deltaerror - deltaerror) / current_deltaerror
        current_deltaerror = deltaerror

        if (error_change < 0.08):
            numpy.savetxt(fitfile, data, delimiter=",")
            numpy.savetxt(deltafile, delta, delimiter=",")
            write_array(hfile, data, rv_, d)
            print(basename + str(i) + " - " + str(deltaerror) + " - " + str(error_change))
            break
        # plt.plot(d, delta, linewidth=1)
        # plt.title(str(i) + " - " + str(deltaerror))
        # plt.show()


def fit(basename, rv_):
    basename = basename + '_' + str(int(rv_)).zfill(3)
    csvfile = basename + ".csv"
    s = open(csvfile, mode='r').read()

    b = numpy.genfromtxt(StringIO(s), delimiter=",")

    r = b[:, 3]
    l = b[:, 2]
    db = b[:, 1]
    rv = b[:, 0]

    do_fit(basename, '_m', (r + l) / 2, db, rv_)
    do_fit(basename, '_r', r, db, rv_)
    do_fit(basename, '_l', l, db, rv_)


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
        filteredcsvfile = basename + '_' + str(int(rv)).zfill(3) + ".csv"

        fmt = '%d,%1.2f,%d,%d'
        numpy.savetxt(filteredcsvfile, f, delimiter=",", fmt=fmt)
    return rvs


# readserial('rv')
for n in ['rv1', 'rv2']:
    rvs = filtercsv(n)
    # for rv in rvs:
    #    fit(n, rv)
    fit(n, 45)
