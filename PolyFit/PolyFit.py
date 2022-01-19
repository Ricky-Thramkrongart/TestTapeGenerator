import numpy
from io import StringIO

def fit(basename):
    csvfile = basename + ".csv"
    fitfile = basename + ".fit"
    s = open(csvfile, mode='r', encoding='utf-8-sig').read()
    open(csvfile, mode='w', encoding='utf-8').write(s)
    with open(csvfile, 'r') as file :
      filedata = file.read()


    # Replace the target string
    filedata = filedata.replace(',', '.')
    filedata = filedata.replace(';', ',')

    b = numpy.genfromtxt(csvfile, delimiter=",")
    y=b[:,1]
    x=b[:,0]

    data = numpy.polyfit(x, y, 6)
    numpy.savetxt(fitfile, data, delimiter=",")


fit("OutPutTable1")