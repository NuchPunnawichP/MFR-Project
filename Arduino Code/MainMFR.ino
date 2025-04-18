// Stepper Motor Connections
#define STEP_PIN 2
#define DIR_PIN 3

const int stepsPerMove = 750; // M8....calculating [reciprocating function - DEMO]
const int stepPerMoveForAdjust = 500; // M8....calculating [for adjsting]
const int stepDelay = 50; // move step (ms)

// Command
String command = "";

// Encoder Connections
const byte encoderPinA = 5;
const byte encoderPinB = 6;

// Encoder Variables
volatile long encoderCount = 0;
const int ppr = 1024;
float angle = 0.0;



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
  Serial.print("Type \"left\" for adjusting the initial position to the left.\n");
  Serial.print("Type \"right\" for adjusting the initial position to the right.\n");
  Serial.print("Type \"zero\" for setting the initial position.\n");
}

void loop()
{
  // updating the encoder position
  read_encoder();

  // vibration demo program
  main_program();
}



// FUNCTION

// moving stepper motor
void moveSteps(int steps)
{
  for (int i = 0 ; i < steps ; i++)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
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

      if (command == "left")
      {
        // Move Left if place the compututer to the right side of linear stage
        Serial.println("Ok Left");
        digitalWrite(DIR_PIN, HIGH);
        moveSteps(stepPerMoveForAdjust);
      }
      else if(command == "right")
      {
        // Move Right if place the compututer to the right side of linear stage
        Serial.println("Ok Right");
        digitalWrite(DIR_PIN, LOW);
        moveSteps(stepPerMoveForAdjust);
      }
      else if(command == "zero")
      {
        Serial.println("Here the starting point");
      }
      else if(command == "start")
      {
        // updating the encoder position
        read_encoder();

        Serial.print("! Let's go !");

        while(true)
        {
          // 1 - cos(theta) function
          
        }
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
  long count = encoderCount;
  interrupts();

  float angle = (count % ppr) * (360.0 / ppr); // convert to angle

  unsigned long currentMillis = millis();

  // Serial Plotter
  Serial.print(angle);
  Serial.print("\t");
  Serial.print(currentMillis);
  Serial.print("\t");
  Serial.println("0");


  //delay(20); // can adjust
}
