



void process(uint8_t **samples_in, int size);

// the FIR filter function
void firFixed( int16_t *coeffs, int16_t *input, int16_t *output,
       int length, int filterLength );
