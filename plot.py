from matplotlib import pyplot as plt
import pandas as pd

def plotXLines(source, path, x):
    data = pd.read_csv(source, header=None)
    a = list(range(x))
    plt.plot(a, data[:x])
    plt.savefig(path)
    plt.clf

def plotAll(source, path):
    data = pd.read_csv(source, header=None)
    a = list(range(len(data.index)))
    plt.plot(a, data)
    plt.savefig(path)
    plt.clf

plotXLines("voltage.csv", "plot1.png", 90)
plotAll("voltage.csv", "plot2.png")
