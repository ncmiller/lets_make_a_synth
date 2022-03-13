import matplotlib.pyplot as plt
import numpy as np

fs=48000
freq=440
period=1./freq
dt=1.0/fs

t = np.arange(0, period, step=dt)

y = np.sign(np.sin(2 * np.pi * freq * t))
plt.stem(t, y)
plt.xlabel('time (s)')
plt.ylabel('square(t)')
plt.xticks(np.arange(0, period+dt, step=period/4))
plt.savefig('square.png')
plt.clf()

nsamples = np.ceil(period / dt)
y = np.arange(-1, 1, step=2/nsamples)
plt.stem(t, y)
plt.xlabel('time (s)')
plt.ylabel('saw(t)')
plt.xticks(np.arange(0, period+dt, step=period/4))
plt.savefig('saw.png')
