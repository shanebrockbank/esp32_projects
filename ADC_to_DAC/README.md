# 📡 ESP32 ADC-to-DAC Passthrough with Voltage Scaling

This project demonstrates the use of the ESP32's built-in **Analog-to-Digital Converter (ADC)** and **Digital-to-Analog Converter (DAC)** peripherals to read an analog voltage (e.g., from a photoresistor) and output a corresponding analog voltage.

The goal is to provide a simple, well-documented implementation that highlights basic embedded systems techniques—sampling, averaging, analog input scaling, and DAC output clamping—suitable for basic firmware engineering.

---

## 🔧 Features

- **ADC Reading**: Samples analog voltage from GPIO36 (ADC1_CHANNEL_0).
- **Noise Reduction**: Implements a simple moving average filter to reduce noise.
- **Voltage Scaling**: Converts the averaged ADC value to a voltage (0–3.3V).
- **DAC Mapping**: Maps the voltage to an 8-bit DAC value (0–255) with clamping.
- **Analog Output**: Outputs the value on GPIO25 (DAC_CHANNEL_1).
- **Debug Print**: Outputs raw, averaged, and scaled data via UART.

---

## 📦 Hardware Setup

| Pin    | Function    | Description                                |
|--------|-------------|--------------------------------------------|
| GPIO36 | ADC Input   | Connect to analog sensor (e.g. LDR + voltage divider) |
| GPIO25 | DAC Output  | Outputs voltage proportional to ADC input  |

**Note**: Input voltage must not exceed 3.3V. Use a voltage divider if needed.

---

## 📄 How It Works

1. **ADC Sampling**: The ESP32 samples analog input at a 12-bit resolution.
2. **Moving Average**: Smooths the signal using a circular buffer of 4 samples.
3. **Voltage Calculation**: Scales ADC value to real voltage using the known max of 4095 and Vref = 3.3V.
4. **DAC Output Mapping**: Converts the voltage to a DAC-friendly range (0–255), applies a manual offset, and clamps the result.
5. **Continuous Loop**: Repeats the read-average-scale-output process every 100ms.

---

## 📈 Future Enhancements

Here are some potential additions to build on this project:

- **Dynamic Scaling**: Adjust the DAC scaling automatically based on observed ADC range for better resolution in varying lighting conditions.
- **Logging to CSV**: Record sensor data via UART to a PC and log in CSV format for later analysis in Python or Excel.
- **Real-Time Plotting**: Use Python + Matplotlib to plot ADC readings in real time through a serial connection.
- **Threshold Triggers**: Add conditional logic to toggle an LED or buzzer when voltage goes above or below a certain threshold.
- **Multiple Sensors**: Expand to read multiple ADC channels and output to different DACs or log data selectively.
