#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
  pin_t pin_out;        // OUT pin - analog output to ESP32
  timer_t timer;
  float phase;
  int heartRate;        // Current heart rate from slider
  uint32_t heartRateAttr; // Attribute reference for slider
} chip_state_t;

static void chip_timer_callback(void *data) {
  chip_state_t *state = (chip_state_t*)data;
  
  // Read current heart rate from slider control
  state->heartRate = (int)attr_read(state->heartRateAttr);
  
  // Ensure valid range
  if (state->heartRate < 40) state->heartRate = 40;
  if (state->heartRate > 200) state->heartRate = 200;
  
  // Generate heartbeat waveform
  float period = 60.0 / state->heartRate;  // Period in seconds
  float position = fmod(state->phase, period);
  
  // Calculate output voltage (0.0 - 3.3V range for ESP32 ADC)
  // Base ~0.4V (500/4095*3.3), peak ~2.4V (3000/4095*3.3)
  float baseVoltage = 0.4;
  float peakVoltage = 2.4;
  float outputVoltage;
  
  // Pulse takes 12% of period (sharp peak like real sensor)
  if (position < (period * 0.12)) {
    float progress = position / (period * 0.12);
    if (progress < 0.25) {
      // Fast rise (25% of pulse)
      outputVoltage = baseVoltage + ((peakVoltage - baseVoltage) * (progress / 0.25));
    } else {
      // Slower fall (75% of pulse)
      outputVoltage = peakVoltage - ((peakVoltage - baseVoltage) * ((progress - 0.25) / 0.75));
    }
  } else {
    // Baseline with tiny noise
    outputVoltage = baseVoltage + ((rand() % 30) / 4095.0 * 3.3);
  }
  
  // Write voltage to OUT pin (ESP32 will read this via analogRead)
  pin_dac_write(state->pin_out, outputVoltage);
  
  // Increment phase (simulates time)
  state->phase += 0.005;
}

void chip_init(void) {
  chip_state_t *state = (chip_state_t*)malloc(sizeof(chip_state_t));
  
  // Initialize OUT pin (connected to ESP32 GPIO 35)
  state->pin_out = pin_init("OUT", ANALOG);
  
  // Initialize attribute for slider control
  state->heartRateAttr = attr_init("heartRate", 75); // Default 75 BPM
  
  // Read initial value
  state->heartRate = (int)attr_read(state->heartRateAttr);
  state->phase = 0;
  
  printf("Pulse Sensor initialized. Heart Rate: %d BPM\n", state->heartRate);
  
  // Create timer - update every 5ms for smooth waveform
  timer_config_t config = {
    .callback = chip_timer_callback,
    .user_data = state,
  };
  state->timer = timer_init(&config);
  timer_start(state->timer, 5000, true); // 5ms period
}