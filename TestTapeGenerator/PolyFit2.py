import numpy
from io import StringIO

def fit(basename):
    csvfile = basename + ".csv"
    fitfile = basename + ".fit"
    s = open(csvfile, mode='r').read()

    b = numpy.genfromtxt(StringIO(s), delimiter=",")
    y=b[:,1]
    x=b[:,0]

    data = numpy.polyfit(y, x, 6)
    numpy.savetxt(fitfile, data, delimiter=",")


fit("RV44")
fit("RV45")
fit("RV99")
fit("RV194")
fit("RV254")
