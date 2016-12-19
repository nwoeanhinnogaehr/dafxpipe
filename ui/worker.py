import grpc
from worker_pb2 import *
import threading

def init():
    global worker
    channel = grpc.insecure_channel('localhost:' + str(__worker_port))
    worker = WorkerStub(channel)

# we start these on a thread as a deadlock workaround
# because calling these methods goes C++ -> Python -> C++ -> Python
# and that doesn't play nice with the locks we have to prevent data races.
# we should find some other way to prevent the deadlock
# because this won't allow us to return data from RPC calls.

def silence():
    threading.Thread(target=worker.Silence, args=(Empty(),)).start()
def setNumInputs(num):
    threading.Thread(target=worker.SetNumInputs, args=(NumChannels(num=num),)).start()
def setNumOutputs(num):
    threading.Thread(target=worker.SetNumOutputs, args=(NumChannels(num=num),)).start()
