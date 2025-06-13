# -*- coding: utf-8 -*-

import math
import numpy as np
import datetime as dt
from datetime import timedelta
import pandas as pd
import sys
import os

if __name__ == "__main__":

    if len(sys.argv) != 4:
        print("Usage: python convert_raw.py <cmeter_params.txt> <raw.bin> <datetime>")
        sys.exit()

    date_time_str = sys.argv[3]
    try:
        date_time_obj = dt.datetime.strptime(date_time_str, '%Y-%m-%d %H:%M:%S')
    except:
        print ("Invalid datetime: {}".format(date_time_str))
        sys.exit()

    raw_filename = sys.argv[2]
    if not os.path.exists(raw_filename):
        print ("Invalid raw file: {}".format(raw_filename))
        sys.exit()


    filename = sys.argv[1]
    cmeter_columns = []

    #obtém o nome dos parâmetros armazenados no arquivo raw
    if os.path.exists(filename):
        with open(filename) as fp:
            line = fp.readline()
            cmeter_columns = line.split(',')
    else:
        print ("Invalid cmeter file")
        sys.exit()


    #cria as colunas com especificador de tipo
    data_type = []
    for param in cmeter_columns:
        if param != "time":
            data_type.append((param,'f4'))

    dt = np.dtype(data_type)

    
    data = np.fromfile(raw_filename, dtype=dt)
    df = pd.DataFrame.from_records(data)

    FREQ = 40.0
    MS = int((1/FREQ)*1000000)
    
    len_blocks = int(df.shape[0]/int(FREQ))
    print(len_blocks)
    raw_date = []

    base_time = np.datetime64(date_time_obj, unit='s')
    
    for i in range(len_blocks):
        first = base_time + np.timedelta64(i*1000000, unit='s')
        
        for j in range(int(FREQ)):
            raw_date.append(first)
            first = first + np.timedelta64(MS)

    
    columns = df.columns
    df.insert(0, 'time', pd.DataFrame(raw_date))
    df["time"] = pd.to_datetime(df["time"], unit='ns')
    df.index = df["time"]

    columns = [item for item in columns if 'dummy_' not in item]
    df[columns].to_csv('./outputs/site_meter_raw.dat', encoding='utf-8', mode='w', header=True, index=True)
    