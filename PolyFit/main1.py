#remove bom
csvfile = 'foo.csv'
s = open(csvfile, mode='r', encoding='utf-8-sig').read()
open(csvfile, mode='w', encoding='utf-8').write(s)


import numpy
b = numpy.genfromtxt(csvfile, delimiter=",")
numpy.savetxt(csvfile, b, fmt="%1.2f", delimiter=",")
y=b[:,1] 
x=b[:,0] 

fit = numpy.polyfit(x, y, 6)
print(fit)


