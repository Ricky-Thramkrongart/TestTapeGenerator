import numpy
from io import StringIO
from matplotlib import pyplot as plt

def fit(basename):
    csvfile = basename + ".csv"
    fitfileleft = basename + ".left.fit"
    fitfileright = basename + ".right.fit"
    s = open(csvfile, mode='r').read()

    b = numpy.genfromtxt(StringIO(s), delimiter=",")

    r=b[:,2]
    l=b[:,1]
    db=b[:,0]



    data = numpy.polyfit(l, db ,1)
    data2 = numpy.polyval(data, l)-db
    plt.plot(l, data2)
    plt.show()       
#

    #numpy.savetxt(fitfileleft, data, delimiter=",")

    #data = numpy.polyfit(numpy.log(r), db,1)
    #numpy.savetxt(fitfileright, data, delimiter=",")








fit("d")
#fit("RV44")
#fit("RV45")
#fit("RV99")
#fit("RV194")
#fit("RV254")
