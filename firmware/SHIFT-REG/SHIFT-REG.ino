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

  Project:     JoyfulNoise SHIFT-REG
  File:        SHIFT-REG.ino
  Description: 4HP CV recording analog shift register

  On trigger, records the CV on PARAM1 into a 16 step ring buffer, and plays back
  a CV from the ring buffer. The offset between the recording and playback
  position is determined by PARAM 2. PARAM3 determines if recording is enabled,
  and if randomization is applied.

  ----------
  PARAMETERS
  ----------

  PARAM 1 - Input CV to record.

  PARAM 2 - Playback step offset.

  PARAM 3 - Freeze / Randomize playback / Record.

  GATE/TRG - Trigger moves the playback and record heads to the next step.

  -----
  JNTUB
  -----

  SHIFT-REG is based on the JoyfulNoise Tiny Utility Board (JNTUB), a
  reprogrammable 4HP module with a standard set of inputs and one 8-bit
  analog output.

 */

// JoyfulNoise Tiny Utility Board Library
#include <limits.h>
#include <JNTUB.h>
#define SEQ_LEN 16
const uint8_t SEQ_MAX_INDEX = (uint8_t)SEQ_LEN - 1;

JNTUB::EdgeDetector trigger;

JNTUB::DiscreteKnob offsetParam(SEQ_LEN, 5);
JNTUB::DiscreteKnob recordKnob(3, 5);
//uint8_t recordKnob;
uint16_t sequence[SEQ_LEN];
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
  recordKnob.update(analogRead(JNTUB::PIN_PARAM3));
  //recordKnob = analogRead(JNTUB::PIN_PARAM3);
  offsetParam.update(analogRead(JNTUB::PIN_PARAM2));

  if (trigger.isRising()) {
    uint8_t playIndex = mod_floor((uint8_t)(index + offsetParam.getValue()), SEQ_MAX_INDEX);
    
    /*
    if (recordKnob.getValue() == 0) {
      // freeze (don't record)
    }
    if (recordKnob.getValue() == 1) {
      // randomize playback
      if (recordKnob.getValueRaw() > )
    }
    if (recordKnob.getValue() == 2) {
      // record
    }
    */

    // Near to full turn of PARAM 3, playback a random nearby position sometimes
    int8_t octave = 0;
    if (recordKnob.getValueRaw() > 960) {
      uint8_t r = random(0, 7);
      playIndex = mod_floor((uint8_t) random(playIndex, playIndex+r), SEQ_MAX_INDEX);
      // double/halve (octave) playValue sometimes at high randomization levels
      if (r < 1) {
        octave = 1;
      }
      if (r > 7) {
        octave = -1;
      }
    }
    
    uint16_t playValue = sequence[playIndex];
    if (octave > 0) {
      playValue = playValue << octave;
    }
    if (octave < 0) {
      playValue = playValue >> abs(octave);
    }
    JNTUB::analogWriteOut(map(playValue, 0, 1023, 0, 255));
    
    if (recordKnob.getValueRaw() > 100 && 
        random(0, 1024) <= recordKnob.getValueRaw()) { // start recording with increasing probabilty
      sequence[index] = analogRead(JNTUB::PIN_PARAM1);
    }

    index++;
    if (index > SEQ_MAX_INDEX) {
      index = 0;
    }
  }
}