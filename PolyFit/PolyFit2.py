import numpy
from io import StringIO

def fit(basename):
    csvfile = basename + ".csv"
    fitfileleft = basename + ".left.fit"
    fitfileright = basename + ".right.fit"
    s = open(csvfile, mode='r').read()

    b = numpy.genfromtxt(StringIO(s), delimiter=",")
    exit	
    r=b[:,2]
    l=b[:,1]
    db=b[:,0]

    data = numpy.polyfit(r, db, 6)
    numpy.savetxt(fitfileright, data, delimiter=",")

    data = numpy.polyfit(l, db, 6)
    numpy.savetxt(fitfileleft, data, delimiter=",")


fit("d")
#fit("RV44")
#fit("RV45")
#fit("RV99")
#fit("RV194")
#fit("RV254")
