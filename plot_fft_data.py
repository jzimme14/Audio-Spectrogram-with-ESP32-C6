import serial
import struct
import matplotlib.pyplot as plt
import numpy as np

# Seriellen Port öffnen
ser = serial.Serial("COM10", 115200, timeout=2)

def read_serial_until(ser, stop_byte):
    """Liest Bytes aus dem seriellen Port, bis das Stop-Byte erreicht wird."""
    data = bytearray()
    while True:
        byte = ser.read(1)
        if byte == stop_byte:
            break
        data.extend(byte)
    return data

def parse_float_values(byte_data):
    """Konvertiert eine Byte-Sequenz in eine Liste von Floats."""
    str_data = byte_data.decode("utf-8")  # Bytes zu String umwandeln
    str_data = str_data.replace(',', ' ')  # ',' durch Leerzeichen ersetzen
    str_values = str_data.split()  # In einzelne Zahlen aufteilen
    return [float(val) for val in str_values]  # In Floats umwandeln

# Warte auf Startsignal 'S'
while ser.read(1) != b'S':
    pass
print("\nStart\n")

# Lese Time-Werte bis 'F'
time_data = read_serial_until(ser, b'F')
time_vals = parse_float_values(time_data)
print("Time Values:", time_vals[:10],time_vals[4090:], len(time_vals))

# Warte auf Startsignal 'H'
while ser.read(1) != b'H':
    pass
print("\nStart\n")

# Lese Frequency-Werte bis 'E'
freq_data = read_serial_until(ser, b'E')
freq_vals = parse_float_values(freq_data)
print("Frequency Values:", freq_vals[:10], len(freq_vals))

# Seriellen Port schließen
ser.close()

# now plot that shit

x_time = np.linspace(0, 4096/44100,4096)
x_freq = np.linspace(0, 44100,2048)

# data manipulation
time_vals[4095] = (time_vals[2046]+time_vals[2045])/2
# buf = 0
# for i in range(len(time_vals)):
#     buf += time_vals[i]
# buf = buf/len(time_vals)
# for i in range(len(time_vals)):
#     time_vals[i] = time_vals[i]-buf

freq_vals[0] = 0
freq_vals[1] = 0


fig, ax = plt.subplots(2, 1, figsize=(8, 6), sharex=True)
ax[0].plot(x_time, time_vals, label="Time Values", color='b')
ax[0].set_xlabel("Zeit")
ax[0].set_ylabel("Amplitude")
ax[0].legend()
ax[0].grid()

ax[1].plot(x_freq, freq_vals, label="Freq Values", color='r')
ax[1].set_ylabel("Amplitude")
ax[1].legend()
ax[1].grid()

plt.tight_layout()  # Sorgt für bessere Darstellung
plt.show()
