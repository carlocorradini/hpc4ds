import matplotlib.pyplot as plt
plt.style.use('seaborn-whitegrid')
import numpy as np
import re

BYTE_EXPONENT = 21

raw_data = []
data = []
_JOIN_WHITESPACE = re.compile(r"\s+")

# Extracting lines and joining whitespaces
with open("./input_try.txt", "r") as f:
	for line in f:
		line = _JOIN_WHITESPACE.sub(" ", line).strip()
		raw_data.append(line.split(" "))

# Picking time values at the proper position in the line
for i in range(BYTE_EXPONENT):
	data.append(raw_data[i][3])		# Number at fourth address

# Removing the "us" unit
for i in range(BYTE_EXPONENT):
	l = len(data[i]) - 2
	data[i] = int(data[i][0:l])

#print(data)
y = data
x = []

# Putting the packet size in bytes on the x axis
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
