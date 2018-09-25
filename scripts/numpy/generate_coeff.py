from scipy import signal
import matplotlib.pyplot as plt
import numpy as np

Fs=44100;

b, a = signal.cheby2(2, 40, float(2000)/float(Fs), 'low', analog=False)
w, h = signal.freqs(b, a)
print (b , a)
plt.plot(w, 20 * np.log10(abs(h)))
plt.title('Chebyshev Type II frequency response (rs=40)')
plt.xlabel('Frequency [radians / second]')
plt.ylabel('Amplitude [dB]')
plt.margins(0, 0.1)
plt.grid(which='both', axis='both')
plt.axvline((2*2000)/Fs, color='green') # cutoff frequency
plt.axhline(-40, color='green') # rs
plt.show()


