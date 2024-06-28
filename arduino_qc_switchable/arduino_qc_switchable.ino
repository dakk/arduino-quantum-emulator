#include <math.h>

const int NUM_QUBITS = 7;
const int NUM_STATES = 1 << NUM_QUBITS; // 2^NUM_QUBITS
const int LED_PINS[NUM_QUBITS] = {5, 6, 7, 8, 9, 10, 11}; // Digital pins for LEDs
const int BUTTON_COMPUTE_PIN = 12;
const int BUTTON_LOOP_PIN = 13;

int lastButtonState = LOW;

// Complex number struct
struct Complex {
  float real;
  float imag;
  
  Complex(float r = 0, float i = 0) : real(r), imag(i) {}
  
  Complex operator*(const Complex& other) const {
    return Complex(real * other.real - imag * other.imag,
                   real * other.imag + imag * other.real);
  }
  
  Complex& operator+=(const Complex& other) {
    real += other.real;
    imag += other.imag;
    return *this;
  }
  
  Complex operator-(const Complex& other) const {
    return Complex(real - other.real, imag - other.imag);
  }
  
  Complex operator+(const Complex& other) const {
    return Complex(real + other.real, imag + other.imag);
  }
};

// Quantum state vector
Complex stateVector[NUM_STATES];



void setup() {
  Serial.begin(9600); 
  Serial.println("Arduino Quantum Computer");
  randomSeed(analogRead(0));
  
  for (int i = 0; i < NUM_QUBITS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  
  pinMode(BUTTON_COMPUTE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LOOP_PIN, INPUT_PULLUP);
}

void swap(Complex& a, Complex& b) {
  Complex temp = a;
  a = b;
  b = temp;
}


// Initialize the state vector to |00000>
void initializeState() {
  for (int i = 0; i < NUM_STATES; i++) {
    stateVector[i] = Complex();
  }
  stateVector[0] = Complex(1, 0);
}


// Apply CNOT gate with control and target qubits
void CX(int control, int target) {
  for (int i = 0; i < NUM_STATES; i++) {
    if ((i & (1 << control)) && !(i & (1 << target))) {
      swap(stateVector[i], stateVector[i ^ (1 << target)]);
    }
  }
}


// Apply Hadamard gate to a specific qubit
void H(int qubit) {
  const float SQRT_2 = 1.0 / sqrt(2);
  for (int i = 0; i < NUM_STATES; i++) {
    if (i & (1 << qubit)) {
      Complex temp = stateVector[i];
      stateVector[i] = stateVector[i ^ (1 << qubit)] * SQRT_2 - temp * SQRT_2;
      stateVector[i ^ (1 << qubit)] = temp * SQRT_2 + stateVector[i ^ (1 << qubit)] * SQRT_2;
    }
  }
}

void X(int qubit) {
  for (int i = 0; i < NUM_STATES; i++) {
    if (i & (1 << qubit)) {
      swap(stateVector[i], stateVector[i ^ (1 << qubit)]);
    }
  }
}


void Y(int qubit) {
  const Complex I(0, 1);
  for (int i = 0; i < NUM_STATES; i++) {
    if (i & (1 << qubit)) {
      Complex temp = stateVector[i];
      stateVector[i] = stateVector[i ^ (1 << qubit)] * I;
      stateVector[i ^ (1 << qubit)] = temp * Complex(0, -1);
    }
  }
}


void Z(int qubit) {
  for (int i = 0; i < NUM_STATES; i++) {
    if (i & (1 << qubit)) {
      stateVector[i] = stateVector[i] * Complex(-1, 0);
    }
  }
}


// Measure the quantum state and collapse it
void measure() {
  float probabilities[NUM_STATES];
  float cumulativeProb = 0;
  
  for (int i = 0; i < NUM_STATES; i++) {
    probabilities[i] = stateVector[i].real * stateVector[i].real + 
                       stateVector[i].imag * stateVector[i].imag;
    cumulativeProb += probabilities[i];
  }
  
  float randomValue = random(1000) / 1000.0 * cumulativeProb;
  int measured_state = 0;
  
  for (int i = 0; i < NUM_STATES; i++) {
    if (randomValue <= probabilities[i]) {
      measured_state = i;
      break;
    }
    randomValue -= probabilities[i];
  }
  
  // Collapse the state vector
  for (int i = 0; i < NUM_STATES; i++) {
    stateVector[i] = Complex();
  }
  stateVector[measured_state] = Complex(1, 0);
  
  // Output result to LEDs
  for (int i = 0; i < NUM_QUBITS; i++) {    
    digitalWrite(LED_PINS[i], (measured_state & (1 << i)) ? HIGH : LOW);
    Serial.print((measured_state & (1 << i))?1:0);
  }
  Serial.print("\n");
}

void compute() {  
  initializeState();

  // Write here your quantum circuit!
  H(0);
  H(1);
  CX(0, 2);
  H(3);
  CX(1, 4);
  CX(1, 5);
  CX(0, 6);
  
  measure();
}


void loop_2() {
  compute();
  delay(1000);
}

  
void loop() {
  // Read the button state
  int reading = digitalRead(BUTTON_LOOP_PIN);

  if (reading == LOW) {  
    reading = digitalRead(BUTTON_COMPUTE_PIN);
  
    if (reading == HIGH && lastButtonState == LOW) {
      lastButtonState = HIGH;
      compute();        
    }
  
    lastButtonState = reading;
  } else {
    compute();
    delay(1000);    
  }
}
