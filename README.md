# Lueftermessstation
Fan test station with ESP32. It automatically tests ~~3 and~~ 4-pin fans.

The code does not use WiFi so far.

Make sure to insert a Micro SD formatted with FAT into the slot! The ESP will only work if a SD card is inserted. If you insert one make sure to restart the board byd  pressing the **EN** button (which triggens a reset).

## Measuring fans with 4 pins

After initializing the SD the Cirtuit supplies 100% PWM (12V) to the fan. It should immediately start rotating at maximum speed. Every 5 seconds the firmware reduces the PWM ratio by 5%. The PWM is pulsed at the standardized 25kHz. After it reaches 0% PWM ratio the Firmware raises the ratio again in 5% steps until it reaches 100%. It cycles vom 100% to 0% and up for infinity. It protocols all the RPM measurements during that time. You may measure several cycles to average more than one run.

## CSV format

The Firmware writes a file named `data.csv` to the SD. It will apeend to the file if it already exists so make sure to erase it manually at your PC before starting a new measurement. If the file is newly created it will write a header into the first line.

The CSV uses the following format:
 1. Percentage of the supply voltage. This is usually 12V but if your power supply delivers slightly more or less that might impact the speed of the fan. The ESP does not measure the voltage.
 2. Percentage of PWM (25.000Hz) at pin 4 of the fan. The controller in the fan reads out this ratio and regulates the motor speed accordingly. This usually does not need to be 12V. Most fans will regulate their speed correctly even if supplied with 3,3V at pin 4. As this is not standardized the circuit will deliver 12V.
 3. Measured RPM using the tacho signal from the fan (pin 3). The fans pull pin 3 to GND once every second. The ESP reacts to a rising flank and measures the time for one rotation. It usually calculates a sliding average of 16 measurements to denoise the timings. If the RPM is very low or even 0 there will be no timings avalilable. So if there was no tacho trigger for at least 2 seconds it sets RPM to 0. The firmware calculates the RPM from the average timings and wirites this value with two digits behind the comma.

As the ESP will not stop measuring it might produce big files. Be aware of that.

The firmware wirtes every measurement to the card. So there will be wrong measurements after changing the speed as the fan still has the momentum of the previous speed setting. Most fans need 3 to 4 seconds to accelerate or decelerate (they usually do not brake actively). Make sure to inspect how long the stabilization takes for your specific fan and discard the misleading measurements. You may average the meaurements after the stabilization. As the Firmware already averages internally you could also use just the last measument before the next speed change.
