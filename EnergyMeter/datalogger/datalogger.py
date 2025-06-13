import array
import struct
import os
import math
import socket
import sys
import traceback
from threading import Thread
from multiprocessing import Event
import datetime as dt
import time 


class Utils:
    def __init__(self):
        self.cmeter_columns = []
        self.cmeter_columns_str = ""
        self.iplug_columns = []
        self.iplug_columns_str = ""

    def get_cmeter_params(self,filename):
        self.cmeter_columns = []
        self.cmeter_columns_str = ""

        with open(filename) as fp:
            line = fp.readline()
            self.cmeter_columns = line.split(',')
            self.cmeter_columns_str = line

    def get_iplug_params(self,filename):
        #self.iplug_columns = ["time", "Vrms", "Irms", "S", "P", "Q"]
        #self.iplug_columns_str = "time,V_rms,I_rms,S,P,Q"
        self.iplug_columns = ["time", "Irms"]
        self.iplug_columns_str = "time,I_rms"


def start_server(host,port,evt_exit,params,max_clients=5):
    soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    soc.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)   
    print("Socket created")

    try:
        soc.bind((host, port))
    except:
        print("Bind failed. Error : " + str(sys.exc_info()))
        sys.exit()

    soc.listen(max_clients)
    soc.settimeout(0.2)
    print("Socket now listening")

    t_clients = []

    while not evt_exit.is_set():
        try:
            connection, address = soc.accept()
            ip, port = str(address[0]), str(address[1])
            print("Connected with " + ip + ":" + port)

            try:
                t = Thread(target=client_thread, args=(connection, ip, port, evt_exit, params))
                t_clients.append(t)
                t.start()
            except:
                print("Thread start error.")
                sys.exit()

        except socket.timeout:
            pass
    
    try:
        soc.close()
        time.sleep(10)
        print("closing server_thread...")
    except:
        print("closing server_thread... error")


def client_thread(connection, ip, port, ev_exit, params, max_buffer_size = 500):
 
    f = None
    board_id = 0

    while not ev_exit.is_set():
        client_input, board_id = receive_input(connection, max_buffer_size)

        if(client_input is not None):
            if board_id == 1:
                filename = "./outputs/site_meter.dat"
            else:
                filename = "./outputs/channel_{}_raw.dat".format(board_id)
                connection.sendall(bytearray("received", encoding="utf8"))

            if f is None:
                f = open(filename,"w+")

                if board_id > 3:
                    f.write("{}\r\n".format(params.iplug_columns_str))
                else:
                    f.write("{}\r\n".format(params.cmeter_columns_str))
    
            else:
                f = open(filename,"a+")

            if board_id > 3:
                columns=params.iplug_columns
            else:
                columns=params.cmeter_columns
            
            args = ""
            for c in columns:
                args += str(client_input[c]) + ','

            f.write("{}\r\n".format(args[:-1]))
            f.close()
            print("Reading meter {}: {}".format(board_id, args[:-1]))

    try:
        connection.sendall(bytearray("close", encoding="utf8"))
        print("closing client_thread...")
        connection.close()
    except:
        print("closing client_thread...error")


def receive_input(connection, max_buffer_size):
    client_input = connection.recv(max_buffer_size)
    client_input_size = len(client_input)

    if client_input_size > max_buffer_size:
        print("The input size is greater than expected {}".format(client_input_size))

    decoded_input = client_input.decode("utf8").rstrip()  # decode and strip end of line
    result = process_input(decoded_input)

    return result


def process_input(input_str):

    try:
        board_id = 0
        if(input_str[0] == '$'):
            args = input_str[1:].split(',')
            args_dict = {}
            args_dict["time"] = dt.datetime.now().replace(microsecond=0).strftime('%Y-%m-%d %H:%M:%S.%f')
            for arg in args:
                [lbl,value] = arg.split(':')
                if lbl == "esp":
                    board_id = 3 + int(value)
                elif lbl == "bbb":
                    board_id = int(value)
                else:
                    args_dict[lbl] = value
            return args_dict,board_id
    except:
        pass
    
    return None, -1


if __name__ == "__main__":
	
    if len(sys.argv) != 4:
        print ("Usage: server.py ip port cmeter.txt")
        sys.exit()

    ip = sys.argv[1]
    port = int(sys.argv[2])
    cmeter_params = sys.argv[3]

    params = Utils()

    if os.path.exists(cmeter_params):
        params.get_cmeter_params(cmeter_params)
    else:
        print ("Invalid cmeter file")
        sys.exit()

    params.get_iplug_params("")

    evt_exit = Event()

    t_server = None

    try:
        t_server = Thread(target=start_server, args=(ip,port,evt_exit,params))
        t_server.start()
    except:
        print("Thread start error.")
        sys.exit()


    print("Enter 'exit' to stop daq")
    if input(" -> ") == "exit":
        evt_exit.set()
        t_server.join(timeout=5)