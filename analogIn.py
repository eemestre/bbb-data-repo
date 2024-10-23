import Adafruit_BBIO.ADC as ADC
from time import time, sleep

ADC.setup()
pin = "P9_37"
hz = 1800
old = time()
a = []

while(True):
    i = 0
    while(i < 60):
        now = time()
        if(now - old >= 1/hz):
            old = now
            now = time()

            raw = ADC.read(pin)
            volt = raw*1.8
            a.append(volt)
            i+=1
    with open('voltage.csv', 'a') as f:
        for v in a:
            f.write(str(v)+"\n")
    a = []
    sleep(60)
