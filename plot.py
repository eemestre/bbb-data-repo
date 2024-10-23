from matplotlib import pyplot as plt
import pandas as pd

data = pd.read_csv("voltage.csv", header=None)
x = list(range(0, 60, 1))
plt.plot(x, data)
plt.savefig('plot.png')
