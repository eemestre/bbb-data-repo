import Adafruit_BBIO.ADC as ADC
from time import time, sleep

def readData():
    global a, pin
    volt = ADC.read(pin) * 1.8
    a.append(volt)

ADC.setup()
pin = "P9_37"
hz = 1800
a = []
t = 2

print("Comecando!")

old = time()
j = 0

while(True):
    i = 0
    while i < 60:
        now = time()
        if now - old >= 1/hz:
            old = now
            now = time()
            readData()
            i+=1
    if j == 2:
        break
    j+=1

print(len(a))

with open("voltage.csv", "w") as f:
    for v in a:
        f.write(str(v)+"\n")

print("Fim!")
