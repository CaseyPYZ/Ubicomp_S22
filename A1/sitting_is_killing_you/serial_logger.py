import serial

arduino_port = "/dev/cu.usbmodem14101" #serial port of Arduino
baud = 9600 #arduino uno runs at 9600 baud
fileName="sitting_alarm_data.csv" #name of the CSV file generated

ser = serial.Serial(arduino_port, baud)
print("Connected to Arduino port:" + arduino_port)
file = open(fileName, "a")
print("Created file")

samples = 200 #how many samples to collect
print_labels = False
line = 0 #start at 0 because our header is 0 (not real data)
while line <= samples:
    # incoming = ser.read(9999)
    # if len(incoming) > 0:
    if print_labels:
        if line==0:
            print("Printing Column Headers")
        else:
            print("Line " + str(line) + ": writing...")

    if line==0:
     data = "x, y, z, gx, gy, gz"
    else:
        getData=str(ser.readline())
        data=getData[2:-3][:-2]
    print(data)
    
    file = open(fileName, "a")
    file.write(data + "\n") #write data with a newline
    line = line+1

print("Data collection complete!")
file.close()