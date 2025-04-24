// Stepper Motor Connections
#define STEP_PIN 2
#define DIR_PIN 3

// Direct Port Access
#define STEP_PORT PORTE
#define STEP_DDR DDRE
#define STEP_BIT 4 // direct port
#define DIR_PORT PORTE
#define DIR_DDR DDRE
#define DIR_BIT 5 // direct port

const int stepsPerMove = 500; // M8....[reciprocating function --> 1-cos() function]
const int stepPerMoveForAdjust = 200; // M8....[for adjsting]
const int stepDelay = 250; // move step (µs) --- 100-300 should be good for direct port access

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

const int sA[5] = {100, 75, 50, 20, 10};
const int sW[5] = {100, 75, 50};

const int max_step[25] = {386, 348, 277, 178, 61, 61, 178, 277, 348, 386, 386, 348, 277, 178, 61, 61, 178, 277, 348, 386};
const int max_speed[25] = {100, 76, 54, 37, 26, 26, 37, 54, 76, 100, 100, 76, 54, 37, 26, 26, 37, 54, 76, 100};

char Orientation[21] = {'r', 'r', 'r', 'r', 'r', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'l', 'r', 'r', 'r', 'r', 'r'};

// Non-blocking stepper variables
unsigned long lastStepTime = 0;

int currentStep = 0;
int currentStepsTotal = 0;
int currentStepDelay = 0;
int currentPatternIndex = 0;

bool isStepHigh = false;
bool isRunning = false;
bool isVibrating = false;

// New variables for saw tooth pattern
bool isSawTooth = false;
bool sawToothDirection = true; // true = right, false = left
int sawPatternIndex = 0; // Track position in saw tooth pattern

// Direction change pause variables
bool isPausing = false;
unsigned long pauseStartTime = 0;
const unsigned long directionChangePause = 50; // 0.05 seconds (50ms) pause at direction change

// Saw tooth pattern arrays (10 steps in each direction)
const int sawStepPattern[20] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
const int sawSpeedPattern[20] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
const char sawOrientationPattern[20] = {'r', 'l', 'r', 'l', 'r', 'l', 'r', 'l', 'r', 'l', 'r', 'l', 'r', 'l', 'r', 'l', 'r', 'l', 'r', 'l'};

// Encoder reading timing
unsigned long lastEncoderReadTime = 0;
const int encoderReadInterval = 20; // Read every 20 ms
bool ch = false; // for checking encoder while vibrating

// MAIN FUNCTION
void setup()
{
  Serial.begin(9600);

  // Stepper Motor - Direct Port Configuration
  STEP_DDR |= (1 << STEP_BIT);  // Set STEP pin as output
  DIR_DDR |= (1 << DIR_BIT);    // Set DIR pin as output
  
  // Make STEP to be LOW at initially
  STEP_PORT &= ~(1 << STEP_BIT);

  // Encoder
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), handleEncoderB, CHANGE);

  // How to use control this vibration device
  Serial.print("WELCOME TO VIBRATION DEMO DEVICE\n");
  Serial.print("Type \"A\" with the number of [0, 1, 2, 3, 4] for %Amplitude at [100%, 75%, 50%, 20%, 10%] with maximum at 50 mm\n");
  Serial.print("Type \"W\" with the number of [0, 1, 2] for %Speed at [100%, 75%, 50%]\n");
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
  
  // Handle non-blocking pause
  handlePause();
  
  // Read encoder
  if(ch)
    readEncoderRegularly();
}

// FUNCTION

// Non-blocking steps
void startStepSequence(int steps, int stepDelayValue)
{
  currentStep = 0;
  currentStepsTotal = steps;
  currentStepDelay = stepDelayValue;
  isStepHigh = false;
  isRunning = true;
  lastStepTime = micros();
}

// Function to handle stepper timing - optimized with direct port
void handleStepper()
{
  if (!isRunning || isPausing)
    return;
  
  unsigned long currentMicros = micros();
  
  // Check if it's time for next step action
  if (currentMicros - lastStepTime >= currentStepDelay)
  {
    lastStepTime = currentMicros;
    
    if (isStepHigh)
    {
      // Step pin LOW using direct port
      STEP_PORT &= ~(1 << STEP_BIT);
      isStepHigh = false;
    }
    else
    {
      // Step pin HIGH using direct port
      STEP_PORT |= (1 << STEP_BIT);
      isStepHigh = true;
      currentStep++;
      
      // Check if movement is complete
      if (currentStep >= currentStepsTotal)
      {
        isRunning = false;
        
        // If in vibration mode, continue to next segment
        if (isVibrating)
        {
          setupNextVibrationSegment();
        }
        else if (isSawTooth)
        {
          // Start pause if we're changing direction
          if (sawPatternIndex == 9 || sawPatternIndex == 19)
          {
            startPause();
          }
          else
          {
            // Continue saw tooth pattern by immediately setting up next segment
            setupNextSawSegment();
          }
        }
      }
    }
  }
}

// Start a non-blocking pause
void startPause()
{
  isPausing = true;
  pauseStartTime = millis();
}

// Handle non-blocking pause
void handlePause()
{
  if (isPausing)
  {
    unsigned long currentMillis = millis();
    
    if (currentMillis - pauseStartTime >= directionChangePause)
    {
      // Pause complete, continue motion
      isPausing = false;
      
      if (isSawTooth)
      {
        setupNextSawSegment();
      }
    }
  }
}

// Move to next vibration segment
void setupNextVibrationSegment()
{
  // Move to next pattern index
  currentPatternIndex++;
  if (currentPatternIndex >= 20)
    currentPatternIndex = 0;
  
  // Calculate steps and speed for this segment
  int stepMoving = int(max_step[currentPatternIndex] / (100 / A));
  int speedMoving = int(max_speed[currentPatternIndex] / (100 / W));
  
  // Set direction based on orientation - using direct port access
  if (Orientation[currentPatternIndex] == 'r')
    DIR_PORT &= ~(1 << DIR_BIT); // RIGHT - LOW
  else if (Orientation[currentPatternIndex] == 'l')
    DIR_PORT |= (1 << DIR_BIT);  // LEFT - HIGH
  
  // Calculate step delay based on speed
  int actualStepDelay = int(stepDelay * (100 / speedMoving));
  
  // Start the next segment
  startStepSequence(stepMoving, actualStepDelay);
}

// Setup next saw tooth segment with sine-like motion pattern
void setupNextSawSegment()
{
  if (isSawTooth)
  {
    // Move to next pattern index
    sawPatternIndex++;
    if (sawPatternIndex >= 20)
      sawPatternIndex = 0;
    
    // Calculate steps and speed for this segment based on sine pattern
    int stepMoving = int(sawStepPattern[sawPatternIndex] / (100 / A));
    int speedMoving = int(sawSpeedPattern[sawPatternIndex] / (100 / W));
    
    // Set direction based on orientation pattern - using direct port access
    if (sawOrientationPattern[sawPatternIndex] == 'r')
      DIR_PORT &= ~(1 << DIR_BIT); // RIGHT - LOW
    else if (sawOrientationPattern[sawPatternIndex] == 'l')
      DIR_PORT |= (1 << DIR_BIT);  // LEFT - HIGH
    
    // Calculate step delay based on speed
    int actualStepDelay = int(stepDelay * (100 / speedMoving));
    
    // Start the next segment
    startStepSequence(stepMoving, actualStepDelay);
  }
}

// Start vibration sequence
void startVibration()
{
  isVibrating = true;
  isSawTooth = false;
  isPausing = false;
  currentPatternIndex = 19; // Set to last index, it will increment to 0 in setupNextVibrationSegment
  setupNextVibrationSegment();
}

// Start saw tooth sequence
void startSawTooth()
{
  isSawTooth = true;
  isVibrating = false;
  isPausing = false;
  sawPatternIndex = 19; // Start at end of pattern
  
  // Initialize first movement
  setupNextSawSegment();
}

// Stop vibration sequence
void stopVibration()
{
  isVibrating = false;
  isSawTooth = false;
  isPausing = false;
  isRunning = false;
}

// Encoder reading while generating excitation
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

      if(command == "A0" || command == "A1" || command == "A2" || command == "A3" || command == "A4")
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
        // Move Left if place the computer to the right side of linear stage
        Serial.println("Ok Left");
        DIR_PORT |= (1 << DIR_BIT);  // LEFT - HIGH
        startStepSequence(stepPerMoveForAdjust, stepDelay);
      }
      else if(command == "r")
      {
        // Move Right if place the computer to the right side of linear stage
        Serial.println("Ok Right");
        DIR_PORT &= ~(1 << DIR_BIT); // RIGHT - LOW
        startStepSequence(stepPerMoveForAdjust, stepDelay);
      }
      else if(command == "zero")
      {
        Serial.print("Here the starting point\n");

        count = 0.0;
        Serial.print("Starting angle: ");
        Serial.println(angle);
      }
      else if(command == "sin")
      {
        // Start vibration in sine sequence
        Serial.println("! SINE Let's go !");
        ch = true;

        startVibration();
      }
      else if(command == "saw")
      {
        // Start vibration in saw tooth sequence with sine-like motion
        Serial.println("! SAW TOOTH Let's go !");
        ch = true; // Enable encoder reading
        
        startSawTooth();
      }
      else if(command == "stop")
      {
        // Stop any movement
        Serial.println("Stopping movement");
        ch = false;

        stopVibration();
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

// Reading encoder
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

  angle = (count % ppr) * (360.0 / ppr / 4); // convert to angle

  unsigned long currentMillis = millis();

  // Serial Plotter
  Serial.print(angle);
  Serial.print("\t");
  Serial.print(currentMillis);
  Serial.print("\t");
  Serial.println("0");
}