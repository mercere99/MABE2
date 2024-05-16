import matplotlib.pyplot as plt
import numpy as np
import sys

UD, best_tot_score, ave_tot_score, dom_tot_score, best_active_pos, active_pos_richness, active_pos_entropy, best_active_count, ave_active_count = np.loadtxt('explore/T10/explore-T10-N1024-G2500-100.csv', delimiter=',', unpack=True, skiprows=1, usecols=[0,1,2,3,4,5,6,7,8])

plt.plot(UD, best_tot_score, label="Best Total Score")
plt.plot(UD, ave_tot_score, label="Average Total Score")
plt.plot(UD, dom_tot_score, label="Mode Total Score")

plt.xlabel('UD')
plt.ylabel('Score')
plt.title('Total Score over Time')
plt.grid(True)
plt.legend()
plt.savefig("TotalScore.png")
plt.show()
