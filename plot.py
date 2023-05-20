import matplotlib.pyplot as plt
import numpy as np

def read_file(filename):
	out = []
	with open(filename, "r") as file:
		for row in file:
			out.append(row.strip().split(","))
	
	return out

horHomoCues = [float(num) for num in read_file("horHomogeneity.csv")[0] if num != '' ]
verHomoCues = [float(num) for num in read_file("verHomogeneity.csv")[0] if num != '' ]
x = np.arange(0, len(horHomoCues))

plt.figure(dpi = 500)
plt.title("Homogeneity with variable displacement")
plt.xlabel("Displacement/Offset")
plt.ylabel("Homogeneity")
plt.plot(x, verHomoCues, ".-", color = "red", label = "Vertical")
plt.plot(x, horHomoCues, ".-", color = "blue", label = "Horizontal")
plt.grid(color = "darkgray", linestyle = "dashed", linewidth = "0.5")
plt.legend()
plt.show()
