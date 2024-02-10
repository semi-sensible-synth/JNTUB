// See: https://modwiggler.com/forum/viewtopic.php?p=2351483#p2351483
//
// Copyright 2016 DSP Synthesizers Sweden.
// Modified 2024 Semi-sensible Synthesisers, Andrew Perry
// Original author: Jan Ostman
//
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

//
// TODO: This has been modified to work with JNTUB, but does not use the existing
//       JNTUB.h library or coding style. We should start using this for ADC reading,
//       pot handling etc to make it consistent with the rest of the firmware.


#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "Samples.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// ATtiny85 clock speed should be 8Mhz

//--------- JNTUB pin definitions ----------
static const uint8_t PIN_OUT = 1;       // PB1/OC1A, chip pin 6
// TODO: Current codebase uses MUX to select ADC and reads ADCL and ADCH
//static const uint8_t PIN_PARAM1 = A1;   // ADC1, chip pin 7
//static const uint8_t PIN_PARAM2 = A3;   // ADC3, chip pin 2
//static const uint8_t PIN_PARAM3 = A2;   // ADC2, chip pin 3
static const uint8_t PIN_GATE_TRG = 0;  // PB0, chip pin 5

static const uint8_t PARAM1_MUX = 1;    // JNTUB left CV in, third pot (JNTUB param 1)
static const uint8_t PARAM2_MUX = 3;    // JNTUB right CV in, second pot (JNTUB param 2)
static const uint8_t PARAM3_MUX = 2;    // JNTUB top pot (JNTUB param 3, no CV)

// These can be swapped around.
// eg, you may wish to swap PARAM2_MUX and PARAM3_MUX (CV controllable 'snappy' for accent, instead of CV controllable pitch)
// Or, you may wish to stick on a single sample and put SAMPLE_MUX = PARAM3_MUX, and pitch and snappy on the two CV controllable channels
static const uint8_t SAMPLE_MUX = PARAM3_MUX;
static const uint8_t PITCH_MUX = PARAM1_MUX;
static const uint8_t SNAPPY_MUX = PARAM2_MUX;
//-----------------------------------------

//static const uint8_t MIN_PITCH = 49;
static const uint8_t MIN_PITCH = 16;
//static const uint8_t MIN_PITCH = 1;

//--------- Ringbuf parameters ----------
uint8_t Ringbuffer[256];
uint8_t RingWrite = 0;
uint8_t RingRead = 0;
volatile uint8_t RingCount = 0;
//-----------------------------------------

//const uint8_t *snappy_sample = drum2;

static uint8_t ra, rb, rc, rx;

/* return 8-bit pseudorandom number */
uint8_t rnd8() {
  rx++;
  ra = (ra ^ rc) ^ rx;
  rb = rb + ra;
  rc = (rc + (rb >> 1)) ^ ra;
  return rc; 
}

/* Add entropy into the state */
void init_rng(uint8_t s1, uint8_t s2, uint8_t s3) {
  /* XOR new entropy into key state */
  ra ^= s1;
  rb ^= s2;
  rc ^= s3;
  rnd8();
}

void setup() {
  init_rng(123, 8, 54);

  OSCCAL = 255;
  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                                     // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10;  // PWM A, clear on match, 1:1 prescale
  //GTCCR = 1<<PWM1B | 2<<COM1B0;           // PWM B, clear on match
  OCR1A = 128;  //OCR1B = 128;               // 50% duty at start


  pinMode(PIN_OUT, OUTPUT);  // Enable PWM output pin
  pinMode(PIN_GATE_TRG, INPUT);


  //Set up Timer/Counter0 for 20kHz interrupt to output samples.

  TCCR0A = 3 << WGM00;              // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;  // 1/8 prescale
  TIMSK = 1 << OCIE0A;              // Enable compare match, disable overflow
  OCR0A = MIN_PITCH;

  uint16_t dummy = analogRead(0);
}

void loop() {
  uint16_t samplecnt1, samplecnt2, samplecnt3, samplecnt4, samplecnt5, samplecnt6, samplecnt7, samplecnt8, samplecnt_snappy;
  uint16_t samplepnt1, samplepnt2, samplepnt3, samplepnt4, samplepnt5, samplepnt6, samplepnt7, samplepnt8, samplepnt_snappy;
  uint8_t dread2;
  uint8_t odread2;
  uint8_t Seldrum;
  uint8_t Snappymix;
  uint8_t MUX = 2;
  sbi(ADCSRA, ADSC);  //start next conversation
  while (1) {
    if (RingCount < 32) {  //if space in ringbuffer
      int16_t total = 0;
      if (samplecnt1) {
        total += (pgm_read_byte_near(drum1 + samplepnt1++) - 128);
        samplecnt1--;
      }
      if (samplecnt2) {
        total += (pgm_read_byte_near(drum2 + samplepnt2++) - 128);
        samplecnt2--;
      }
      if (samplecnt3) {
        total += (pgm_read_byte_near(drum3 + samplepnt3++) - 128);
        samplecnt3--;
      }
      if (samplecnt4) {
        total += (pgm_read_byte_near(drum4 + samplepnt4++) - 128);
        samplecnt4--;
      }
      if (samplecnt5) {
        total += (pgm_read_byte_near(drum5 + samplepnt5++) - 128);
        samplecnt5--;
      }
      if (samplecnt6) {
        total += (pgm_read_byte_near(drum6 + samplepnt6++) - 128);
        samplecnt6--;
      }
      if (samplecnt7) {
        total += (pgm_read_byte_near(drum7 + samplepnt7++) - 128);
        samplecnt7--;
      }
      if (samplecnt8) {
        total += (pgm_read_byte_near(drum8 + samplepnt8++) - 128);
        samplecnt8--;
      }
      if (samplecnt_snappy) {
        // this works quite well as a 'snappy', using the linear decay curve
        //total += ((rnd8() * Snappymix * (pgm_read_byte_near(snappy_sample + samplepnt_snappy++) - 128))  >> 8);
        //samplecnt_snappy--;

        total += ((rnd8() * (Snappymix >> 5) * (pgm_read_byte_near(snappy_sample + samplepnt_snappy++) - 128))  >> 8);
        samplecnt_snappy--;
      }
      

      total >>= 1;
      total += 128;
      if (total > 255) total = 255;
      cli();
      Ringbuffer[RingWrite] = total;
      RingWrite++;
      RingCount++;
      sei();
      dread2 = digitalRead(PIN_GATE_TRG);
      if (dread2 != odread2) {
        odread2 = dread2;
        if (odread2) {  // If trigger changed, and trigger high ...
          
          // snappy always resets to first sample on trigger
          samplepnt_snappy = 0;
          samplecnt_snappy = sizeof(snappy_sample);

          if (Seldrum == 0) {
            samplepnt1 = 0;
            samplecnt1 = sizeof(drum1);
          }
          if (Seldrum == 1) {
            samplepnt2 = 0;
            samplecnt2 = sizeof(drum2);
          }
          if (Seldrum == 2) {
            samplepnt3 = 0;
            samplecnt3 = sizeof(drum3);
          }
          if (Seldrum == 3) {
            samplepnt4 = 0;
            samplecnt4 = sizeof(drum4);
          }
          if (Seldrum == 4) {
            samplepnt5 = 0;
            samplecnt5 = sizeof(drum5);
          }
          if (Seldrum == 5) {
            samplepnt6 = 0;
            samplecnt6 = sizeof(drum6);
          }
          if (Seldrum == 6) {
            samplepnt7 = 0;
            samplecnt7 = sizeof(drum7);
          }
          if (Seldrum == 7) {
            samplepnt8 = 0;
            samplecnt8 = sizeof(drum8);
          }
        }
      }

      if (!(ADCSRA & 64)) {
        // pitch
        if (MUX == PITCH_MUX) OCR0A = MIN_PITCH + ((127 - ((ADCL + (ADCH << 8)) >> 3)));
        
        // 'snappy'
        if (MUX == SNAPPY_MUX) Snappymix = (ADCL | (ADCH << 8)) >> 2;
        
        // sample selection
        if (MUX == SAMPLE_MUX) Seldrum = ((ADCL + (ADCH << 8)) >> 7);

        MUX++;
        if (MUX == 4) MUX = 1;
        ADMUX = MUX;        //Select MUX
        sbi(ADCSRA, ADSC);  //start next conversation
      }
    }
  }
}

ISR(TIMER0_COMPA_vect) {
  //-------------------  Ringbuffer handler -------------------------

  if (RingCount) {                     //If entry in FIFO..
    OCR1A = Ringbuffer[(RingRead++)];  //Output 8-bit DAC
    RingCount--;
  }

  //-----------------------------------------------------------------
}
