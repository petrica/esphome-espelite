### Data Folder

The file sensor-signal.sr contains a single temperature and humidity reading from an SI7021 sensor. The signal follows this protocol:

1. Host initiates by sending:
   - 500μs logical LOW 
   - 40μs logical HIGH

2. Sensor responds with:
   - 40μs logical LOW
   - 76μs logical HIGH

3. Data transfer (40 bits total):
   Each bit consists of:
   - 42μs logical LOW
   - For bit 0: 24μs logical HIGH
   - For bit 1: 72μs logical HIGH
