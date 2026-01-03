#!/usr/bin/python3
import time
import os
import sys

# CGI headers
print("Content-Type: text/plain")
print()  # blank line separates headers from body

print("CGI started")
print("PID =", os.getpid())
sys.stdout.flush()  # flush so server sees output immediately

i = 0
while i < 4:  # run 10 seconds
    print(f"Running... second {i}")
    sys.stdout.flush()  # flush every iteration
    time.sleep(1)
    i += 1

print("CGI finished")
sys.stdout.flush()

