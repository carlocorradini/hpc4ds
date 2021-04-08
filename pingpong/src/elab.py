import matplotlib.pyplot as plt
plt.style.use('seaborn-whitegrid')
import numpy as np

BYTE_EXPONENT = 21

raw_data = []
data = []
with open("./input_cluster_1_2.txt", "r") as f:
	for line in f:
		raw_data.append(line.split(" "))

for i in range(BYTE_EXPONENT):
	data.append(raw_data[i][3])		# Number at third address

for i in range(BYTE_EXPONENT):
	l = len(data[i]) - 2
	data[i] = int(data[i][0:l])

print(data)
y = data
x = []

for i in range(BYTE_EXPONENT):
	x.append(2**i)

plt.figure(figsize=(10, 6))
plt.plot(x, y, 'o', color='blue')
csfont = {'fontname':'Helvetica'}
plt.ylabel("Time (microsec)", **csfont)
plt.xlabel("Packet size (byte)", **csfont)
plt.xscale('log')

axes = plt.gca()
axes.xaxis.label.set_size(12)
axes.yaxis.label.set_size(12)

plt.show()
