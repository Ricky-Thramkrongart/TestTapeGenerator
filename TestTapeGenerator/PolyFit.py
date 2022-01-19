#remove bom
csvfile = 'OutPutTable2.csv'
fitfile = 'OutPutTable2.fit'

s = open(csvfile, mode='r', encoding='utf-8-sig').read()
open(csvfile, mode='w', encoding='utf-8').write(s)


with open(csvfile, 'r') as file :
  filedata = file.read()


# Replace the target string
filedata = filedata.replace(',', '.')
filedata = filedata.replace(';', ',')

# Write the file out again
with open(csvfile, 'w') as file:
  file.write(filedata)


import numpy
b = numpy.genfromtxt(csvfile, delimiter=",")
numpy.savetxt(csvfile, b, fmt="%1.2f", delimiter=",")
y=b[:,1] 
x=b[:,0] 

fit = numpy.polyfit(x, y, 6)
numpy.savetxt(fitfile, fit, delimiter=",")


