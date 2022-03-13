import matplotlib.pyplot as plt
import numpy as np

fs=48000
freq=440
period=1./freq
dt=1.0/fs
nsamples = int(np.round(period / dt))

t = np.linspace(0, period, nsamples)

y = np.sign(np.sin(2 * np.pi * freq * t))
plt.stem(t, y)
plt.xlabel('time (s)')
plt.ylabel('square(t)')
plt.xticks(np.linspace(0, period, 5))
plt.savefig('square.png')
plt.clf()

y = np.linspace(-1, 1, nsamples)
plt.stem(t, y)
plt.xlabel('time (s)')
plt.ylabel('saw(t)')
plt.xticks(np.linspace(0, period, 5))
plt.savefig('saw.png')
plt.clf()

y0 = np.linspace(1, -1, round(nsamples/2)+1)
y1 = np.linspace(-1, 1, round(nsamples/2))
y = np.concatenate((y0, y1))
plt.stem(t, y)
plt.xlabel('time (s)')
plt.ylabel('triangle(t)')
plt.xticks(np.linspace(0, period, 5))
plt.savefig('triangle.png')
