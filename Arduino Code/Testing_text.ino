String command = "";

void setup()
{
  Serial.begin(9600);

  Serial.print("Here the command:");
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
        Serial.println("ok left");
      }
      else if(command == "right")
      {
        Serial.println("ok right");
      }
      else if(command == "zero")
      {
        Serial.println("ready");
      }
      else if(command == "start")
      {
        Serial.println("let's go");
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
