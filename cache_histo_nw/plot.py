import matplotlib.pyplot as plt

hit = []
miss = []
x = []

for line in open("hist.csv"):
    l = line.split(",")
    x.append(int(l[0]))
    hit.append(int(l[1]))
    miss.append(int(l[2]))

plt.plot(x,hit,label="hits")
plt.plot(x,miss,label="misses")
plt.legend()
plt.show()
