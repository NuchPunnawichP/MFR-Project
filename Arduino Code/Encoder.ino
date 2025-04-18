// Pin connections
const byte encoderPinA = 2;
const byte encoderPinB = 3;

// Variables
volatile long encoderCount = 0;
const int ppr = 1024;
float angle = 0.0;

void setup()
{
  Serial.begin(9600);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), handleEncoderB, CHANGE);
}


void loop()
{
  noInterrupts();
  long count = encoderCount;
  interrupts();

  float angle = (count % ppr) * (360.0 / ppr);

  // Serial Plotter
  unsigned long currentMillis = millis();
  Serial.print(angle);
  Serial.print("\t");
  Serial.print(currentMillis);
  Serial.print("\t");
  Serial.println("0");


  delay(20);
}

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
