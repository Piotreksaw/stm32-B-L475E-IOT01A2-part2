import serial
import re
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

PORT = "COM7"
BAUD = 115200
MAX_POINTS = 100

ser = serial.Serial(PORT, BAUD, timeout=1)

packet = {k: None for k in [
    "temperature","humidity",
    "acc_x","acc_y","acc_z",
    "gyro_x","gyro_y","gyro_z",
    "mag_x","mag_y","mag_z"
]}

def reset_packet():
    for k in packet:
        packet[k] = None

x_data, temperature, humidity = [], [], []
acc_x, acc_y, acc_z = [], [], []
gyro_x, gyro_y, gyro_z = [], [], []
mag_x, mag_y, mag_z = [], [], []

counter = 0

def parse_line(line):
    line = line.strip()
    if line.startswith("T:"):
        t = re.findall(r"T:\s*([0-9.\-]+)", line)
        h = re.findall(r"H:\s*([0-9.\-]+)", line)
        if t and h:
            packet["temperature"] = float(t[0])
            packet["humidity"] = float(h[0])
    elif line.startswith("ACC:"):
        vals = re.findall(r"X=([-\d]+)\s*Y=([-\d]+)\s*Z=([-\d]+)", line)
        if vals:
            packet["acc_x"], packet["acc_y"], packet["acc_z"] = map(float, vals[0])
    elif line.startswith("GYRO:"):
        vals = re.findall(r"X=([-\d]+)\s*Y=([-\d]+)\s*Z=([-\d]+)", line)
        if vals:
            packet["gyro_x"], packet["gyro_y"], packet["gyro_z"] = map(float, vals[0])
    elif line.startswith("X:") and "Y:" in line and "Z:" in line:
        vals = re.findall(r"X:\s*([-\d]+)\s*Y:\s*([-\d]+)\s*Z:\s*([-\d]+)", line)
        if vals:
            packet["mag_x"], packet["mag_y"], packet["mag_z"] = map(float, vals[0])

def packet_complete():
    return all(v is not None for v in packet.values())

plt.style.use("seaborn-v0_8")
fig, axes = plt.subplots(4,1,figsize=(10,16))
ax1, ax2, ax3, ax4 = axes

lines = {
    "temperature": ax1.plot([],[],label="Temperature")[0],
    "humidity": ax1.plot([],[],label="Humidity")[0],
    "acc_x": ax2.plot([],[],label="ACC X")[0],
    "acc_y": ax2.plot([],[],label="ACC Y")[0],
    "acc_z": ax2.plot([],[],label="ACC Z")[0],
    "gyro_x": ax3.plot([],[],label="GYRO X")[0],
    "gyro_y": ax3.plot([],[],label="GYRO Y")[0],
    "gyro_z": ax3.plot([],[],label="GYRO Z")[0],
    "mag_x": ax4.plot([],[],label="MAG X")[0],
    "mag_y": ax4.plot([],[],label="MAG Y")[0],
    "mag_z": ax4.plot([],[],label="MAG Z")[0],
}

for ax in axes:
    ax.legend()
    ax.set_xlim(0, MAX_POINTS)

def update(frame):
    global counter
    line = ser.readline().decode(errors="ignore")
    parse_line(line)
    if not packet_complete():
        return

    counter += 1
    x_data.append(counter)
    temperature.append(packet["temperature"])
    humidity.append(packet["humidity"])
    acc_x.append(packet["acc_x"])
    acc_y.append(packet["acc_y"])
    acc_z.append(packet["acc_z"])
    gyro_x.append(packet["gyro_x"])
    gyro_y.append(packet["gyro_y"])
    gyro_z.append(packet["gyro_z"])
    mag_x.append(packet["mag_x"])
    mag_y.append(packet["mag_y"])
    mag_z.append(packet["mag_z"])
    reset_packet()

    for data in [x_data, temperature, humidity, acc_x, acc_y, acc_z,
                 gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z]:
        if len(data) > MAX_POINTS:
            del data[:len(data)-MAX_POINTS]

    lines["temperature"].set_data(x_data, temperature)
    lines["humidity"].set_data(x_data, humidity)
    lines["acc_x"].set_data(x_data, acc_x)
    lines["acc_y"].set_data(x_data, acc_y)
    lines["acc_z"].set_data(x_data, acc_z)
    lines["gyro_x"].set_data(x_data, gyro_x)
    lines["gyro_y"].set_data(x_data, gyro_y)
    lines["gyro_z"].set_data(x_data, gyro_z)
    lines["mag_x"].set_data(x_data, mag_x)
    lines["mag_y"].set_data(x_data, mag_y)
    lines["mag_z"].set_data(x_data, mag_z)

    for ax, data_group in zip(axes, [
        [temperature, humidity],
        [acc_x, acc_y, acc_z],
        [gyro_x, gyro_y, gyro_z],
        [mag_x, mag_y, mag_z]
    ]):
        ymin = min([min(d) for d in data_group]) - 1
        ymax = max([max(d) for d in data_group]) + 1
        ax.set_ylim(ymin, ymax)
        ax.set_xlim(max(0, counter-MAX_POINTS), counter)

ani = FuncAnimation(fig, update, interval=200, cache_frame_data=False)
plt.tight_layout()
plt.show()
