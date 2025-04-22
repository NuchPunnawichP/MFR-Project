// Stepper Motor Connections
#define STEP_PIN 2
#define DIR_PIN 3

const int stepsPerMove = 500; // M8....[reciprocating function --> 1-cos() function]
const int stepPerMoveForAdjust = 200; // M8....[for adjsting]
const int stepDelay = 750; // move step (ms) --- 10-30 should be great

// Command
String command = "";

// Encoder Connections
const byte encoderPinA = 20;
const byte encoderPinB = 21;

// Encoder Variables
volatile long encoderCount = 0;
const int ppr = 1024;
float angle = 0.0;
long count = 0;

// sin or cos function
// finite position and speed. Declare by step and stepDelay;
float A = 100.0;
float W = 100.0;

const int sA[5] = {100, 75, 50, 20};
const int sW[5] = {100, 75, 50};

const int max_step[25] = {386, 348, 277, 178, 61, 61, 178, 277, 348, 386, 386, 348, 277, 178, 61, 61, 178, 277, 348, 386};
const int max_speed[25] = {100, 76, 54, 37, 26, 26, 37, 54, 76, 100, 100, 76, 54, 37, 26, 26, 37, 54, 76, 100};

char Orientation[21] = {'r', 'r', 'r', 'r', 'r', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'r', 'r', 'r', 'r', 'r'};

// Non-blocking stepper variables
unsigned long lastStepTime = 0;

int currentStepIndex = 0;
int currentStep = 0;
int currentStepsTotal = 0;
int currentStepDelay = 0;

bool isStepHigh = false;
bool isRunning = false;

int vibrationIndex = 0;

// Encoder reading timing
unsigned long lastEncoderReadTime = 0;
const int encoderReadInterval = 20; // Read every 20ms

// MAIN FUNCTION

void setup()
{
  Serial.begin(9600);

  // Stepper Motor
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  // Encoder
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), handleEncoderB, CHANGE);

  // How to use control this vibration device
  Serial.print("WELCOME TO VIBRATION DEMO DEVICE\n");
  Serial.print("Type \"A\" with the number of [0, 1, 2] for %Amplitude at [100%, 75%, 50%] with maximum at 50 mm\n");
  Serial.print("Type \"W\" with the number of [0, 1, 2, 3] for %Speed at [100%, 75%, 50%, 20%]\n");
  Serial.print("Type \"l\" for adjusting the initial position to the left.\n");
  Serial.print("Type \"r\" for adjusting the initial position to the right.\n");
  Serial.print("Type \"zero\" for setting the initial position.\n");
  Serial.print("Type \"start\" for start the Vibration Demo Device\n");
  Serial.print("Type \"stop\" to stop the vibration mode\n");
}

void loop()
{
  // main for vibration device
  main_program();
  
  // Non-blocking stepper motor control
  handleStepper();
  
  // Read encoder
  readEncoderRegularly();
}



// FUNCTION

// Non-blocking moveSteps
void startStepSequence(int steps, int fraction)
{
  currentStep = 0;
  currentStepsTotal = steps;
  currentStepDelay = int(stepDelay / (100 / fraction));
  isStepHigh = false;
  isRunning = true;
  lastStepTime = micros();
}

// Function to handle stepper timing
void handleStepper()
{
  if (!isRunning) return;
  
  unsigned long currentMicros = micros();
  
  // Check if it's time for next step action
  if (currentMicros - lastStepTime >= currentStepDelay)
  {
    lastStepTime = currentMicros;
    
    if (isStepHigh)
    {
      // Step pin LOW
      digitalWrite(STEP_PIN, LOW);
      isStepHigh = false;
    }
    else
    {
      // Step pin HIGH and count step
      digitalWrite(STEP_PIN, HIGH);
      isStepHigh = true;
      currentStep++;
      
      // Check if movement is complete
      if (currentStep >= currentStepsTotal)
      {
        if (vibrationIndex > 0)
        {
          // Continue to next segment in vibration sequence
          setupNextVibrationSegment();
        }
        else
        {
          // End of single movement
          isRunning = false;
        }
      }
    }
  }
}

// Setting up for the next vibration segment
void setupNextVibrationSegment()
{
  vibrationIndex++;
  if (vibrationIndex >= 20) vibrationIndex = 0;
  
  // Calculate steps and speed for this segment
  int stepMoving = int(max_step[vibrationIndex] / (100 / A));
  int speedMoving = int(max_speed[vibrationIndex] / (100 / W));
  
  // Set direction
  if (Orientation[vibrationIndex] == 'r')
    digitalWrite(DIR_PIN, LOW); // RIGHT
  else if (Orientation[vibrationIndex] == 'l')
    digitalWrite(DIR_PIN, HIGH); // LEFT
    
  // Reset step counter and set new parameters
  currentStep = 0;
  currentStepsTotal = stepMoving;
  currentStepDelay = int(stepDelay / (100 / speedMoving));
}

// encoder reading while generating excitation
void readEncoderRegularly()
{
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastEncoderReadTime >= encoderReadInterval)
  {
    lastEncoderReadTime = currentMillis;
    read_encoder();
  }
}

void main_program()
{
  while (Serial.available())
  {
    char c = Serial.read();

    if (c == '\n')
    {
      command.trim();

      if(command == "A0" || command == "A1" || command == "A2" || command == "A3")
      {
        int mode = command[1] - '0';
        A = sA[mode];
        
        // Debugging
        Serial.print("Mode Amplitude\n");
        Serial.println(mode);
        Serial.println(A);
      }
      else if(command == "W0" || command == "W1" || command == "W2")
      {
        int mode = command[1] - '0';
        W = sW[mode];

        // Debugging
        Serial.print("Mode W\n");
        Serial.println(mode);
        Serial.println(W);
      }
      else if(command == "ch")
      {
        int tim = 0;
        while(tim < 10)
        {
          read_encoder();
          delay(20);
          tim++;
        }
      }
      else if (command == "l")
      {
        // Move Left if place the compututer to the right side of linear stage
        Serial.println("Ok Left");
        digitalWrite(DIR_PIN, HIGH);
        startStepSequence(stepPerMoveForAdjust, 100);
      }
      else if(command == "r")
      {
        // Move Right if place the compututer to the right side of linear stage
        Serial.println("Ok Right");
        digitalWrite(DIR_PIN, LOW);
        startStepSequence(stepPerMoveForAdjust, 100);
      }
      else if(command == "zero")
      {
        Serial.print("Here the starting point\n");

        count = 0.0;
        Serial.print("Starting angle: ");
        Serial.println(angle);
      }
      else if(command == "start")
      {
        // Start vibration sequence
        Serial.println("! Let's go !");
        
        // Initialize vibration mode
        vibrationIndex = -1; // Just for the initial
        setupNextVibrationSegment();
        isRunning = true;
      }
      else if(command == "stop")
      {
        // Stop any movement
        Serial.println("Stopping movement");
        isRunning = false;
        vibrationIndex = 0;
      }
      else
      {
        Serial.println("Try Again !!");
      }

      command = "";
    }
    else
    {
      command += c;
    }
  }
}



// reading encoder
void handleEncoderA()
{
  bool A = digitalRead(encoderPinA);
  bool B = digitalRead(encoderPinB);

  if (A == B)
  {
    encoderCount++;
  }
  else
  {
    encoderCount--;
  }
}

void handleEncoderB()
{
  bool A = digitalRead(encoderPinA);
  bool B = digitalRead(encoderPinB);

  if (A != B)
  {
    encoderCount++;
  }
  else
  {
    encoderCount--;
  }
}

void read_encoder()
{
  noInterrupts();
  count = encoderCount;
  interrupts();

  angle = (count % ppr) * (360.0 / ppr); // convert to angle

  unsigned long currentMillis = millis();

  // Serial Plotter
  Serial.print(angle);
  Serial.print("\t");
  Serial.print(currentMillis);
  Serial.print("\t");
  Serial.println("0");
}