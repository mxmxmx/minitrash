/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef synth_fm_drum_h_
#define synth_fm_drum_h_
#include "AudioStream.h"
#include "utility/dspinst.h"


class AudioSynthFMDrum : public AudioStream
{
public:
	AudioSynthFMDrum() : AudioStream(1, inputQueueArray), magnitude(16384) {} 

	void frequency(uint16_t frequency) {

		if (frequency <= 16384) {
			aux_envelope_strength_ = 1024;
    	}
    	else if (frequency <= 32768) {
    		aux_envelope_strength_ = 2048 - (frequency >> 4);
    	} else {
    		aux_envelope_strength_ = 0;
    	}
    	frequency_ = (24 << 7) + ((72 << 7) * frequency >> 16);
	}
	void morph(uint16_t x, uint16_t y);
	void fm_amount(uint16_t _fm_a) {
		fm_amount_ = _fm_a >> 2;
	}
	void decay(uint16_t decay) {
		am_decay_ = 16384 + (decay >> 1);
		fm_decay_ = 8192 + (decay >> 2);
	}
	void set_noise(uint16_t noise) {
		uint32_t n = noise;
   	    noise_ = noise >= 32768 ? ((n - 32768) * (n - 32768) >> 15) : 0;
        noise_ = (noise_ >> 2) * 5;
        overdrive_ = noise <= 32767 ? ((32767 - n) * (32767 - n) >> 14) : 0;
	};
	virtual void update(void);
	void init(uint8_t clock_pin);
private:

	inline uint16_t Interpolate824(const uint16_t* table, uint32_t phase);
	inline int16_t Interpolate1022(const int16_t* table, uint32_t phase);
	inline int16_t Mix(int16_t a, int16_t b, uint16_t balance);
	uint32_t ComputeEnvelopeIncrement(uint16_t decay);
	uint32_t ComputePhaseIncrement(int16_t midi_pitch);
	audio_block_t *inputQueueArray[1];
	uint8_t sd_range_;
	int32_t magnitude;
	uint8_t _clk;
	uint8_t _prev_clk;
	uint16_t frequency_;
	uint16_t fm_amount_;
	uint16_t am_decay_;
	uint16_t fm_decay_;
	uint32_t noise_;
	uint32_t overdrive_;
	uint16_t aux_envelope_strength_;
	int16_t  previous_sample_;
	uint32_t phase_;
	uint32_t fm_envelope_phase_;
	uint32_t am_envelope_phase_;
	uint32_t aux_envelope_phase_;
	uint32_t phase_increment_;
};

#endif