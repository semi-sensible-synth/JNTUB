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

  Project:     JoyfulNoise D-RAND
  File:        FlipFlopSlew.ino
  Description: 4HP Source Switching Slew

  Slews between the current value and the value sampled at an input when triggered,
  swapping the source input between PARAM 1 or PARAM 2 when with each trigger. 
  Can be thought of as a flip flop switching between using PARAM 1 and PARAM 2 as a
  sample and hold source, with slew toward the new sampled value.

  ----------
  PARAMETERS
  ----------

  PARAM 1 - Input voltage target 1.

  PARAM 2 - Input voltage target 2.

  PARAM 3 - Slew
    Slew rate between current and sampled value.

  GATE/TRG - Switch input source and sample voltage.

  -----
  JNTUB
  -----

  FlipFlopSlew is based on the JoyfulNoise Tiny Utility Board (JNTUB), a
  reprogrammable 4HP module with a standard set of inputs and one 8-bit
  analog output.

 */

// JoyfulNoise Tiny Utility Board Library
#include <JNTUB.h>

JNTUB::EdgeDetector trigger;

const uint16_t SLEW_RATE_CURVE[] = {
  0,  // no slew
  150,  // 150ms slew
  1000,  // 1s slew
};
JNTUB::CurveKnob<uint16_t> slewRateKnob(SLEW_RATE_CURVE, NELEM(SLEW_RATE_CURVE));

// Used to time the slew
JNTUB::Stopwatch stopwatch;

bool targetTwo;
uint8_t prevVal;
uint8_t targetVal;

void setup()
{
  JNTUB::setUpFastPWM();

  prevVal = 128;
  targetVal = 128;
  targetTwo = true;
}

void loop()
{
  stopwatch.update(millis());
  trigger.update(digitalRead(JNTUB::PIN_GATE_TRG));
  slewRateKnob.update(analogRead(JNTUB::PIN_PARAM3));

  if (trigger.isRising()) {
    prevVal = targetVal;
    if (targetTwo) {
      uint16_t oneRaw = analogRead(JNTUB::PIN_PARAM1);
      targetVal = map(oneRaw, 0, 1023, 0, 255);
    } else {
       uint16_t twoRaw = analogRead(JNTUB::PIN_PARAM2);
      targetVal = map(twoRaw, 0, 1023, 0, 255);
    }
    targetTwo = !targetTwo;
    stopwatch.reset();
  }

  uint16_t slewTimeMs = slewRateKnob.getValue();
  uint32_t timeSinceLastTrg = stopwatch.getTime();

  if (slewTimeMs == 0 || timeSinceLastTrg >= slewTimeMs) {
    JNTUB::analogWriteOut(targetVal);
  } else {
    // Continue slewing to target value
    uint8_t currentVal = map(
        timeSinceLastTrg, 0, slewTimeMs, prevVal, targetVal);
    JNTUB::analogWriteOut(currentVal);
  }
}