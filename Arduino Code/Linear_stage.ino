// Stepper Motor
#define STEP_PIN 2
#define DIR_PIN 3

const int stepsPerMove = 750; // M8....calculating [reciprocating function - DEMO]
const int stepPerMoveForAdjust = 500; // M8....calculating [for adjsting]
const int stepDelay = 50; // move step (ms)

// Command
String command = "";

void setup()
{
  Serial.begin(9600);
  Serial.println("Welcome");

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
}

void loop()
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
        Serial.print("! Let's go !");

        while(true)
        {
          // Move forward
          digitalWrite(DIR_PIN, HIGH);
          moveSteps(stepsPerMove);
          //delay(50);

          // Move backward
          digitalWrite(DIR_PIN, LOW);
          moveSteps(stepsPerMove);
          //delay(50);
        }
      }
      else if(command == "stop")
      {
        Serial.println("HUH");
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

  /*
  // Move left
  digitalWrite(DIR_PIN, HIGH);
  moveSteps(stepsPerMove);
  delay(500);

  // Move right
  digitalWrite(DIR_PIN, LOW);
  moveSteps(stepsPerMove);
  delay(500);
  */
}

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
