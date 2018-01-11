# StepCounter

This is a simple Arduino Pro Mini project with a 128x64 I2C OLED Display and
a MPU-6050 accelerator+Gyroscop chip.

## Features

folder: flipClockStep

 -  count steps
 -  Display sleep Mode (shake to wake up)
 -  Clock (set button1 = hours, button2 = minutes)
 -  Lipo Charging
 -  low cost (10EUR Aliexpress)
 -  goal bar (till 0 o'clock you need this goal as steps)
 -  serial: Send A to make RGB-LED pink
 -  serial: Send B to make RGB-LED cyan
 -  serial: Send C to make RGB-LED white
 -  serial: Send 123456 000876 080 to set 12:34 and 56sec, day goal=876, step treshold=80 (low)

Every 10sec it sends via bluetooth: steps, actual threshold, daily goal and power in mV

## EEpROM

- on every set: store goal, treshold, steps, hour, minute
- on every 500 steps: store steps
- on every hour: store steps
- on every 20min: store hour and minute

## Circuit (old)

![Circuit](circuit/circuit.png)
