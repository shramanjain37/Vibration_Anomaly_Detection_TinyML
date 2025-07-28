#include <Wire.h>
#include <MPU6050_light.h>
#include <LiquidCrystal_I2C.h>

// TensorFlow Lite Micro
#include <TensorFlowLite.h>
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"

// Model
#include "fan_low_model.h"

// Utility functions (MAD, MSE)
extern "C" {
  #include "utils.h"
}

// Constants
#define NUM_SAMPLES 200
#define NUM_AXES 3
#define MAD_SCALE 1.4826 //constant in MAD
#define MSE_THRESHOLD 4e-4

// Sensor & Display
MPU6050 mpu(Wire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// TensorFlow Lite
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
tflite::MicroMutableOpResolver<3> resolver;

constexpr int kTensorArenaSize = 2 * 1024; //block of memory to store tensors (i/o),layers and buffers
uint8_t tensor_arena[kTensorArenaSize];

// Data buffers
float features[NUM_SAMPLES][NUM_AXES]; // raw sensor data
float measurements[NUM_SAMPLES]; // buffer to compute MAD
float mad[NUM_AXES]; // actual MAD values
float recon[NUM_AXES]; // reconstructed MAD output from model

void setup() {
  Serial.begin(115200);
 
  while (!Serial);
  delay(500);
  Serial.println("Serial OK");

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Init...");

  // MPU6050
  Wire.begin();
  byte status = mpu.begin();
  if (status != 0) {
    Serial.print("MPU init failed: "); Serial.println(status);
    lcd.print("MPU Error");
    while (1);
  }
  mpu.calcOffsets(true, true); // automatic calibration of sensor on start

  // Model
  model = tflite::GetModel(fan_low_model); //load model
  if (model->version() != TFLITE_SCHEMA_VERSION) { //model should match library version
    Serial.println("Model schema mismatch");
    lcd.print("Model Error");
    while (1);
  }

  resolver.AddFullyConnected(); // trained model with dense() 
  resolver.AddReshape(); // used reshape in model
 
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter; // interpreter is brain of model. Allocates memory, accepts input, excutes mode, stores output

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("Tensor allocation failed");
    lcd.print("Tensor Error");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

}

void loop() {

  Serial.println("Collecting samples...");

  // Sample accelerometer readings
  for (int i = 0; i < NUM_SAMPLES; i++) {
    mpu.update();
    features[i][0] = mpu.getAccX();
    features[i][1] = mpu.getAccY();
    features[i][2] = mpu.getAccZ();
    delay(5); // ~200Hz sampling rate
  }

  // Compute MAD for each axis
  for (int axis = 0; axis < NUM_AXES; axis++) {
    for (int i = 0; i < NUM_SAMPLES; i++) {
      measurements[i] = features[i][axis];
    }
    mad[axis] = MAD_SCALE * calc_mad(measurements, NUM_SAMPLES);
  }

  Serial.print("MAD: ");
  for (int i = 0; i < NUM_AXES; i++) {
    Serial.print(mad[i], 6); Serial.print(" ");
    input->data.f[i] = mad[i]; // Feed into model input
  }
  Serial.println();

  // Run inference (call model on input MAD vector)
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Inference failed!");
    lcd.print("Infer Error");
    return;
  }

  // Get reconstructed MAD from model
  for (int i = 0; i < NUM_AXES; i++) {
    recon[i] = output->data.f[i];
  }

  // Compute MSE between input and output (high mse is anomaly)
  float mse = calc_mse(mad, recon, NUM_AXES);
  Serial.print("MSE: ");
  Serial.println(mse, 7);

  // Show result
  lcd.clear();
  if (mse > MSE_THRESHOLD) {
    lcd.print("ANOMALY");
    Serial.println("Anomaly detected!");
    
  } else {
    lcd.print("NORMAL");
    
  }

  delay(2000);
}
