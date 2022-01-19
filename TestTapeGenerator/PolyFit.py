import numpy
from io import StringIO

def fit(basename):
    csvfile = basename + ".csv"
    fitfile = basename + ".fit"
    s = open(csvfile, mode='r', encoding='utf-8-sig').read()
    s = s.replace(',', '.')
    s = s.replace(';', ',')

    b = numpy.genfromtxt(StringIO(s), delimiter=",")
    y=b[:,1]
    x=b[:,0]

    data = numpy.polyfit(x, y, 6)
    numpy.savetxt(fitfile, data, delimiter=",")


fit("OutPutTable1")
fit("OutPutTable2")