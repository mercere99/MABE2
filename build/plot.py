import matplotlib.pyplot as plt
import numpy as np

#UD, max_fit, mean_fit, min_fit, first_fit, vals, scores = np.loadtxt('output.csv', usecols=(), delimiter=',', unpack=True)
UD, max_fit, mean_fit, min_fit, first_fit = np.loadtxt('output.csv', usecols=(0,1,2,3,4), delimiter=',', unpack=True)

plt.plot(UD, first_fit, label="First fitness")
plt.plot(UD, max_fit, label="Maximum fitness")
plt.plot(UD, mean_fit, label="Average fitness")

plt.xlabel('UD')
plt.ylabel('Fitness')
plt.title('Fitness over Time')
plt.grid(True)
plt.legend()
plt.savefig("Fitness.png")
plt.show()
