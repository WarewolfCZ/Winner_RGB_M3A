EESchema Schematic File Version 4
LIBS:Winner_RGB_M3A_MOD-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Microchip_ATmega:ATmega8-16PU U5
U 1 1 5E059316
P 6350 2850
F 0 "U5" V 6600 4300 50  0000 L CNN
F 1 "ATmega8-16PU" V 6500 4300 50  0000 L CNN
F 2 "Package_DIP:DIP-28_W7.62mm" H 6350 2850 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/atmel-2486-8-bit-avr-microcontroller-atmega8_l_datasheet.pdf" H 6350 2850 50  0001 C CNN
	1    6350 2850
	0    1    1    0   
$EndComp
Wire Wire Line
	4950 2950 4750 2950
Wire Wire Line
	4950 2850 4750 2850
Wire Wire Line
	4750 2850 4750 2950
Wire Wire Line
	7750 2850 8100 2850
Wire Wire Line
	7750 2950 8100 2950
Wire Wire Line
	8100 2950 8100 2850
Connection ~ 8100 2850
Wire Wire Line
	7150 3450 7150 3750
$Comp
L Relay_SolidState:MOC3062M U2
U 1 1 5E093040
P 6550 5100
F 0 "U2" V 6504 5288 50  0000 L CNN
F 1 "MOC3062M" V 6950 4850 50  0000 L CNN
F 2 "Package_DIP:DIP-6_W7.62mm" H 6350 4900 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/MO/MOC3061M.pdf" H 6515 5100 50  0001 L CNN
	1    6550 5100
	0    1    1    0   
$EndComp
$Comp
L Relay_SolidState:MOC3062M U3
U 1 1 5E0A480A
P 6950 5100
F 0 "U3" V 6904 5288 50  0000 L CNN
F 1 "MOC3062M" V 7350 4950 50  0000 L CNN
F 2 "Package_DIP:DIP-6_W7.62mm" H 6750 4900 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/MO/MOC3061M.pdf" H 6915 5100 50  0001 L CNN
	1    6950 5100
	0    1    1    0   
$EndComp
$Comp
L Relay_SolidState:MOC3062M U4
U 1 1 5E0A6236
P 7400 5100
F 0 "U4" V 7354 5288 50  0000 L CNN
F 1 "MOC3062M" V 7800 4950 50  0000 L CNN
F 2 "Package_DIP:DIP-6_W7.62mm" H 7200 4900 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/MO/MOC3061M.pdf" H 7365 5100 50  0001 L CNN
	1    7400 5100
	0    1    1    0   
$EndComp
Wire Wire Line
	6650 4000 6650 4800
Wire Wire Line
	7050 4050 7050 4800
Wire Wire Line
	7500 4100 7500 4800
Wire Wire Line
	6000 4600 6000 4800
Wire Wire Line
	6000 4600 6450 4600
Wire Wire Line
	6450 4600 6450 4800
Wire Wire Line
	6450 4600 6850 4600
Wire Wire Line
	6850 4600 6850 4800
Connection ~ 6450 4600
Wire Wire Line
	6850 4600 7300 4600
Wire Wire Line
	7300 4600 7300 4800
Connection ~ 6850 4600
Wire Wire Line
	5450 3450 5450 3600
Wire Wire Line
	5350 3450 5350 3500
$Comp
L Connector_Generic:Conn_01x10 J1
U 1 1 5E0EA28B
P 4800 4500
F 0 "J1" V 4925 4446 50  0000 C CNN
F 1 "Conn_01x10" V 5016 4446 50  0000 C CNN
F 2 "Connector_JST:JST_PH_S10B-PH-K_1x10_P2.00mm_Horizontal" H 4800 4500 50  0001 C CNN
F 3 "~" H 4800 4500 50  0001 C CNN
	1    4800 4500
	0    1    1    0   
$EndComp
Wire Wire Line
	4750 2850 4300 2850
Wire Wire Line
	4300 2850 4300 3600
Connection ~ 4750 2850
Wire Wire Line
	4400 4300 4400 1800
Wire Wire Line
	8100 1800 8100 2050
$Comp
L Device:R R1
U 1 1 5E0F294A
P 4700 4000
F 0 "R1" H 4550 4000 50  0000 L CNN
F 1 "330k" H 4500 3850 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4630 4000 50  0001 C CNN
F 3 "~" H 4700 4000 50  0001 C CNN
	1    4700 4000
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 5E0F37DF
P 4500 3800
F 0 "C1" V 4450 3500 50  0000 C CNN
F 1 "4.7nF" V 4550 3450 50  0000 C CNN
F 2 "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 4538 3650 50  0001 C CNN
F 3 "~" H 4500 3800 50  0001 C CNN
	1    4500 3800
	0    1    1    0   
$EndComp
Wire Wire Line
	4700 3850 4700 3800
Wire Wire Line
	4700 4300 4700 4150
Wire Wire Line
	5350 3500 4700 3500
Wire Wire Line
	4700 3500 4700 3800
Connection ~ 4700 3800
Wire Wire Line
	4350 3800 4300 3800
Connection ~ 4300 3800
Wire Wire Line
	4300 3800 4300 4300
Wire Wire Line
	4650 3800 4700 3800
$Comp
L Device:C C2
U 1 1 5E0FB1DE
P 4600 3600
F 0 "C2" V 4348 3600 50  0000 C CNN
F 1 "4.7nF" V 4439 3600 50  0000 C CNN
F 2 "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 4638 3450 50  0001 C CNN
F 3 "~" H 4600 3600 50  0001 C CNN
	1    4600 3600
	0    1    1    0   
$EndComp
Connection ~ 4300 3600
Wire Wire Line
	4300 3600 4300 3800
$Comp
L Device:R R2
U 1 1 5E0FC8D3
P 4800 3800
F 0 "R2" H 4870 3846 50  0000 L CNN
F 1 "330k" H 4870 3755 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4730 3800 50  0001 C CNN
F 3 "~" H 4800 3800 50  0001 C CNN
	1    4800 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 3950 4800 4300
Wire Wire Line
	4800 3650 4800 3600
Wire Wire Line
	4450 3600 4300 3600
Wire Wire Line
	4800 3600 4750 3600
Wire Wire Line
	5450 3600 4800 3600
Connection ~ 4800 3600
Text Notes 5250 3700 0    50   ~ 0
Cold
Text Notes 5050 3500 0    50   ~ 0
Warm
Wire Wire Line
	7150 3750 6200 3750
Wire Wire Line
	6200 3750 6200 4800
Wire Wire Line
	7250 4000 6650 4000
Wire Wire Line
	7350 4050 7050 4050
Wire Wire Line
	7450 4100 7500 4100
Wire Wire Line
	4300 2850 3400 2850
Wire Wire Line
	3400 2850 3400 4850
Wire Wire Line
	3400 4850 5750 4850
Wire Wire Line
	5750 4850 5750 4600
Wire Wire Line
	5750 4600 6000 4600
Connection ~ 4300 2850
Connection ~ 6000 4600
Wire Wire Line
	7450 3450 7450 4100
Wire Wire Line
	7350 3450 7350 4050
Wire Wire Line
	7250 3450 7250 4000
Wire Wire Line
	6850 2250 6850 1800
Connection ~ 6850 1800
$Comp
L Device:R R3
U 1 1 5E120273
P 7800 2050
F 0 "R3" V 7593 2050 50  0000 C CNN
F 1 "10k" V 7684 2050 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 7730 2050 50  0001 C CNN
F 3 "~" H 7800 2050 50  0001 C CNN
	1    7800 2050
	0    1    1    0   
$EndComp
Wire Wire Line
	4400 1800 6850 1800
$Comp
L Switch:SW_Push SW1
U 1 1 5E11EECC
P 7200 1600
F 0 "SW1" H 7200 1885 50  0000 C CNN
F 1 "SW_Push" H 7200 1794 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm_H5mm" H 7200 1800 50  0001 C CNN
F 3 "~" H 7200 1800 50  0001 C CNN
	1    7200 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 2250 7450 2050
Wire Wire Line
	7450 1600 7400 1600
Wire Wire Line
	7000 1600 4750 1600
Wire Wire Line
	4750 1600 4750 2850
Wire Wire Line
	7450 2050 7650 2050
Connection ~ 7450 2050
Wire Wire Line
	7950 2050 8100 2050
Connection ~ 8100 2050
Wire Wire Line
	8100 2050 8100 2850
Wire Wire Line
	7450 1600 7450 2050
Wire Wire Line
	6850 1800 8100 1800
Text Label 3400 2850 0    50   ~ 0
GND
Text Label 8100 2500 0    50   ~ 0
5V
$Comp
L Relay_SolidState:MOC3062M U1
U 1 1 5E0A736C
P 6100 5100
F 0 "U1" V 6054 5288 50  0000 L CNN
F 1 "MOC3062M" V 6500 4850 50  0000 L CNN
F 2 "Package_DIP:DIP-6_W7.62mm" H 5900 4900 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/MO/MOC3061M.pdf" H 6065 5100 50  0001 L CNN
	1    6100 5100
	0    1    1    0   
$EndComp
Wire Wire Line
	5200 4300 5200 4150
Wire Wire Line
	5200 4150 6100 4150
Wire Wire Line
	6100 4150 6100 4800
Wire Wire Line
	5100 4300 5100 4100
Wire Wire Line
	5100 4100 6550 4100
Wire Wire Line
	6550 4100 6550 4800
Wire Wire Line
	5000 4300 5000 4050
Wire Wire Line
	5000 4050 6950 4050
Wire Wire Line
	6950 4050 6950 4800
Wire Wire Line
	4900 4300 4900 4200
Wire Wire Line
	4900 4200 7400 4200
Wire Wire Line
	7400 4200 7400 4800
$EndSCHEMATC
