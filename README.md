# MorseCodeDecoder
This application, written in C, is designed for the FRDM-KL05Z development platform with additional use of ALS-PT19 light sensor, HD44780U LCD controller with PCF8574 I/O expander. The scheme of how to connect everything properly is shown below. 
 
![image](https://user-images.githubusercontent.com/69008729/119542801-3a8a1400-bd90-11eb-8d81-9ab4f45a137d.png)

# Project assumptions
Duration of Morse code elements:
- dot 0.1-1.2 s
- dash 1.3-4.0 s
- space between two signals dedicated to describe one sign (letter/digit) 0.1-1.2 s
- space between two signs 1.3-3.4 s
- space between two words 3.5-11.9 s

# How it works
Every 100 ms PWM delivers an interrupt which cause in calculation the average of the light sensor measurements. Then, this average is compared with reference value (which is established during system startup), when it is greater then the "up" counter is increased (value of this counter will be used to determine either dot or dash is obtained), when it is less than reference value, the "down" counter is increased (value of this counter will be used to determine how long the light sensor isn't obtaining any significant signal). When there is no signal for 12 s after calibration/last obtained signal, user is informed about this situation by LCD screen. When the "up" counter value is greater than 40, it means that the measured light intensity is greater than the reference value for more than 4 s, which can be caused by change of the work environment. It is necessary to restart the board to calibrate it again.
