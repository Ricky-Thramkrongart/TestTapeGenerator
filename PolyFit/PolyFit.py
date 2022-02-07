from io import StringIO
import numpy
from math import sqrt
from matplotlib import pyplot as plt
import serial

def write_array(hfile, data):
    s = open(hfile, mode='wb')
    s.write(str('            fit64 = std::vector <float64_t>(' + str(len(data)) + ');\r\n').encode())
    i = len(data) - 1
    for d in data:
        s.write(str('            fit64[' + str(i) + '] = fp64_atof(String(F("' + str(d) + '")).c_str());\r\n').encode())
        i -= 1
    s.close()


def fit(basename):
    csvfile = basename + ".csv"
    hfile = basename + ".h"
    fitfile = basename + ".fit"
    deltafile = basename + ".err"

    current_deltaerror = 1000000.0
    s = open(csvfile, mode='r').read()
    s = s.replace(',', '.')
    s = s.replace(';', ',')

    b = numpy.genfromtxt(StringIO(s), delimiter=",")
    y=b[:,1]
    x=b[:,0]

    for i in range(18):
        data = numpy.polyfit(x, y, i)
        delta = numpy.polyval(data, x) - y
        deltaerror = sqrt(numpy.dot(delta, delta)) / delta.size
        error_change = (current_deltaerror - deltaerror) / current_deltaerror
        current_deltaerror = deltaerror

        if (error_change < 0.01):
            numpy.savetxt(fitfile, data, delimiter=",")
            numpy.savetxt(deltafile, delta, delimiter=",")
            write_array(hfile, data)
            print(basename + str(i) + " - " + str(deltaerror) + " - " + str(error_change))
            break
        # plt.plot(d, delta, linewidth=1)
        # plt.title(str(i) + " - " + str(deltaerror))
        # plt.show()


fit("OutPutTable1")
fit("OutPutTable2")