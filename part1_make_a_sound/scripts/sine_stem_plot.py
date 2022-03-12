import matplotlib.pyplot as plt
import numpy as np

fs=48000
freq=440
period=1./freq
dt=1.0/fs
t = np.arange(0, period, step=dt)
y = np.sin(2 * np.pi * freq * t)
plt.stem(t, y)
plt.xlabel('time (s)')
plt.ylabel('sine(t)')
plt.xticks(np.arange(0, period+dt, step=period/4))
plt.show()
