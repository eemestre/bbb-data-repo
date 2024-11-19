import Adafruit_BBIO.ADC as ADC
from time import time, sleep

def readData():
    global a, pin
    volt = ADC.read(pin) * 1.8
    a.append(volt)

ADC.setup()
pin = "P9_37"
hz = 2700
a = []
t = 2

print("Comecando!")

while(True):
    readData()
    sleep(1/hz)
    if len(a) >= (hz/60)*2:
        break

print(len(a))

with open("voltage.csv", "w") as f:
    for v in a:
        f.write(str(v)+"\n")

print("Fim!")
