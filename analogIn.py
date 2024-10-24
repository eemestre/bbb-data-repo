import Adafruit_BBIO.ADC as ADC
from time import time, sleep
import signal

def readData(signum, frame):
    global a, pin
    volt = ADC.read(pin) * 1.8
    a.append(volt)

ADC.setup()
pin = "P9_37"
hz = 1800
a = []
t = 2

# set signal -> map handler function
signal.signal(signal.SIGALRM, readData)

print("Comecando!")

# start signal timer after 0.000001 seconds with activation interval of 1/hz
signal.setitimer(signal.ITIMER_REAL, 0.000001, 1/hz)

sleep(t)

# stop signal timer
signal.setitimer(signal.ITIMER_REAL, 0, 0.0)

print(len(a))

with open("voltage.csv", "w") as f:
    for v in a:
        f.write(str(v)+"\n")

print("Fim!")