/*
  Copyright (C) 2021  Ben Reeves
  Copyright (C) 2021  Andrew Perry

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  ==============================================================================

  Project:     JoyfulNoise CV-FIFO
  File:        CV-FIFO.ino
  Description: 4HP FIFO CV sequence recorder

  On trigger, reads the CV on PARAM1. Records CVs into a ring buffer,
  played back in sequence upon trigger. PARAM2 controls recording probability, 
  from off (freeze sequence) to always record every step. PARAM3 sets the ring 
  buffer (sequence) length.

  TODO: IDEA: PARAM2 sets playback offset (eg like a two head tape delay ?). 
  TODO: IDEA: PARAM2 is the playback trigger (GATE/TRIG remains the record trigger),
              so we step the playback head and the record head independently.

  ----------
  PARAMETERS
  ----------

  PARAM 1 - Input CV to record.

  PARAM 2 - Freeze (first quarter) / Record probabilty / Randomize playback (last quarter)

  PARAM 3 - Record buffer length.

  GATE/TRG - Trigger next step. Plays the next position in the buffer, and depending on the recording
             state (PARAM 2?) records the value at PARAM 1 into the previous position.

  -----
  JNTUB
  -----

  FIFO CV is based on the JoyfulNoise Tiny Utility Board (JNTUB), a
  reprogrammable 4HP module with a standard set of inputs and one 8-bit
  analog output.

 */

// JoyfulNoise Tiny Utility Board Library
#include <limits.h>
#include <JNTUB.h>
#define MAX_SEQ_LEN 64

const uint8_t SEQ_LENS[6] = { 2, 4, 8, 16, 32, MAX_SEQ_LEN };

JNTUB::EdgeDetector trigger;

JNTUB::DiscreteKnob lengthKnob(NELEM(SEQ_LENS), 5);

uint16_t recordParam;
uint16_t sequence[MAX_SEQ_LEN];
uint8_t index;

// modulo with floored division.
// used to wrap array indicies around like a ring buffer, eg
// wrappedIndex = mod_floor(outOfBoundsIndex, arrayLength-1)
template<typename T>
T mod_floor(T a, T n) {
    return ((a % n) + n) % n;
}

void setup()
{
  JNTUB::setUpFastPWM();

  index = 0;
  randomSeed(analogRead(JNTUB::PIN_PARAM1) ^ analogRead(JNTUB::PIN_PARAM2));
}

void loop()
{
  trigger.update(digitalRead(JNTUB::PIN_GATE_TRG));
  lengthKnob.update(analogRead(JNTUB::PIN_PARAM3));
  recordParam = analogRead(JNTUB::PIN_PARAM2);

  uint8_t loopMaxIndex = SEQ_LENS[lengthKnob.getValue()] - 1;

  if (trigger.isRising()) {
    uint8_t playIndex = index;

    // Near to full turn of PARAM 2, playback a random nearby position sometimes
    int8_t octave = 0;
    if (recordParam > 768 && random(768, 1023) <= recordParam) {
      playIndex = mod_floor((uint8_t) random(playIndex, playIndex+16), loopMaxIndex);
      // double/halve (octave) playValue sometimes at high randomization levels
      uint8_t r = random(0, 7);
      if (r == 0) {
        octave = 1;
      }
      if (r == 7) {
        octave = -1;
      }
    }
    //uint8_t playValue = map(sequence[playIndex], 0, 1023, 0, 255);
    uint16_t playValue = sequence[playIndex];
    if (octave > 0) {
      playValue = playValue << octave;
    }
    if (octave < 0) {
      playValue = playValue >> abs(octave);
    }
    JNTUB::analogWriteOut(map(playValue, 0, 1023, 0, 255));
    
    index++;
    if (index > loopMaxIndex) {
      index = 0;
    }

    // Increasing probability of recording. 
    // Freeze at full anticlockwise, always record at full turn clockwise.
    if (recordParam > 128 && random(0, 1023) <= recordParam) {
      sequence[index] = analogRead(JNTUB::PIN_PARAM1);
    }
  }
}