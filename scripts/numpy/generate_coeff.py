from scipy import signal
import matplotlib.pyplot as plt
import numpy as np

Fs=48000

b, a = signal.iirfilter(1, [0.2, 0.4], rs=60, btype='band',
                        analog=False, ftype='cheby2')
w, h = signal.freqz(b, a, 1000)
print (b,a)
fig = plt.figure()
ax1 = fig.add_subplot(111)
plt.plot(w, 20 * np.log10(abs(h)), 'b')
plt.ylabel('Amplitude [dB]', color='b')
plt.xlabel('Frequency [rad/sample]')
ax2 = ax1.twinx()
angles = np.unwrap(np.angle(h))
plt.plot(w, angles, 'g')
plt.ylabel('Angle (radians)', color='g')
plt.grid()
plt.axis('tight')
plt.show()
