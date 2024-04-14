/*
This programs is for CNC Single Axis leth machine developed by Octagon Manufacturing Technology on Dt 21 March 2023 
This update include up and down motion of stepper moter using keypad buttons the function include are 
button_command() and set_zero()
author : Aditya Khode
*/
/*-----------------------------------------------------------------------*/

// Included all Libraries that are need

#include <LiquidCrystal_I2C.h>  // for 16x2 Display
#include <FlexyStepper.h>       // To control Stepper Moter
#include <Keypad.h>             // To take Input fro 4x4 keyboard
#include <math.h>               // To Use Math function power(a,b)
#include<arduino.h>

/*-----------------------------------------------------------------------*/

// Defined all the arduino pins

// Arduino uno 0 and 1 pins are not used
const int BUTTON_PIN = 2;           //Arduino pin no 2 for start the main drilling process
const int limitSwitchPin = 3;       //Arduino pin no 3 to initilise the position of Drill structure to the very upper position
const int ROW_NUM = 4;              //Arduino pin no 4,5,6,7 used for keypad Connect R1,R2,R3,R4 with 4,5,6,7 respectively
byte pin_rows[ROW_NUM] = {4,5,6,7};          
const int MOTOR_DIRECTION_PIN = 8;  //Arduino pin no 8 connected to tb6600 Stepper Driver dir+
const int MOTOR_STEP_PIN = 9;       //Arduino pin no 9 connected to tb6600 Stepper Driver pul+
const int relaymoter  = 10;         //Arduino pin no 10 connected to in1 of relay module
const int relaypumpmoter = 11;      //Arduino pin no 11 connected to in2 of relay module
//Arduino uno  12 are 13 pins aren't used
const int COLUMN_NUM = 4;           //Arduino pin no A0,A1,A2,A3 used for keypad Connect C1,C2,C3,C4 with A0,A1,A2,A3 respectively
byte pin_column[COLUMN_NUM] = {A0,A1,A2,A3};
//Arduino uno A4 used as SDA pin for LCD Display
//Arduino uno A5 used as SCL pin for LCD Display

/*--------------------------------------------------------------------------*/

// Defined all the constants and variable required in program 

float pos = 0 , dist = 0 , drill_distance = 0 ; // Variables use in operation Function
float result = 0;
int index = -1 ;                                            // Purpose use for Display of array first iterate with 1 and then add element in array stack like concept
int i , speed = 0 ;                                                     // Used in for loops
long inputInt ;                                             // Saves only pure number excluding point
char arr[10] ;                                              // Saves all the element from keypad from 0 to 9 amd also '.' only used for displaying purpose on LCD
char keys[ROW_NUM][COLUMN_NUM] = 
  {
    {'1','2','3', 'A'},
    {'4','5','6', 'B'},
    {'7','8','9', 'C'},
    {'.','0','#', 'D'}
  } ;                                                       // maped keys
String inputString = "";                                    //Saves only pure characters from 0 to 9 After every update the char is added up (concatenation addition of char values such as 1234)
float garbage;                                               // can store initilizer value where we do not required dist value

/*-----------------------------------------------------------------------------*/

// Created object for Keypad
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

//Created object for I2C Dispaly
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Created object for stepper moter 
FlexyStepper stepper;

/*___________________________________________________________________________________________________________________*/

// Function to take input from user Display it on LCD and return float value at the end

float num_value()                                        // name of the function is num_value
  {
    while(true)                                          // Takes the input until user doesn't press 'A' 
      {
          char key = keypad.getKey();                   // gets the char from keybord                                            <-----|
          delay(100);                                    //                                                                             |
          if (key !=NO_KEY)                             // Checks if the User has entered the value else again goes to previous line---|
            {
            /*----------------------------------------------------------------------*/

              if ((key >= '0' && key <= '9')||key =='.')  //checks if the input key is from 0 to 9 or 'A' or 'B' if from 0 to 9 or '.' executes fires if blocks
                {
                  // Intates the index from -1 to 0 ans becomes arr[0] ans saves the key for next key it wil be arr[1] and so on..

                  index =index+1;                      
                  arr[index]=key; 

                  // Displays all the array till last element enter i.e is index +1 on LCD it also display '.' and still it is not a float number it is just string
                  
                  lcd.setCursor(0,1);                  
                  for(int k = 0;k<index+1;k++)
                    {
                      lcd.print(arr[k]);
                      delay(10); 
                    }

                  // Concanation of char keys each time after display excluding the '.'

                  if( key != '.' )
                    {
                      inputString += key;                 
                    } 
                }
            /*---------------------------------------------------------------*/
                // If key is B the function is recalled and input process is again started
              else if (key == 'B') 
                {
                  inputString = "";
                  index = -1;
                  lcd.clear();
                  delay(100); 
                  return num_value();
                }
            /*----------------------------------------------------------------*/
               // If key is A the inputstring string is converted ti integer variable inputint

              else if (key == 'A')
                {
                  if (inputString.length() > 0)                 //length is greater than 0
                    {
                      int count = 0 , check = 0 , len = 0;      // initilizer
                      len = index+1;                            // saves the length arr i.e if arr value is "10.56" then len is 5
                /*-------------------------------------*/
                // below code scans if arr contains piont (.) or not     
                      for(i = 0; i<len;i++)
                        {
                          if(arr[i]=='.')
                            {
                              check =1;                             // if it contains point then it changes the value of check from 0 to 1 and stops checking
                              delay(10); 
                              break;
                            }
                          else
                            {
                              count = count +1;                    //  iterates the count from 0 until just before it gets point(.)
                            }
                        }
                /*---------------------------------------*/

                // if arr contains the point (.) it executs the if part else else part
                      if (check == 1)
                        {
                // we will divide the pure integer number with 10 to the power something but something is our let say div_factor
                // in prev example "10.56" if we divide "1056" with (10 to the power 2) that is 100 we will get 10.56 
                // So here div_factor will be 2 therefore here len = 5 and count = 2 for the example if we scan arr  so div_factor is len - count -1 i.e is 5-2-1 = 2
                // here we have substracted 1 because count iteration just before (.) and in len there is one iteration for (.)  also 
                          int div_factor = len - count - 1 ;
                      
                          inputInt = inputString.toInt();      // Converts string to interger ex "1056" to 1056
                          result = float(inputInt)/(pow(10, div_factor));   // divide using math.h function power 10 to the power div_factor
                          inputString = "";                     //resets the inputString
                          index = -1;                          // resets the index
                          delay(100); 
                          return result;
                        }
                // if no point is there then no need to divide by anything 
                       else
                        {
                          inputInt = inputString.toInt();      // converts string to integer
                          index = -1;                          // resets the index
                          inputString = "";                    //resets the inputString
                          result = (float) inputInt;
                          delay(100); 
                          return result;
                        }
                    }
                }    
           }   
      }
  }

/*___________________________________________________________________________________________________________*/

// gets the distance initially it has to travel to reach near job
float before_drill()
  {
    lcd.setCursor(0,0);                            // sets cursor to 0,0 and clears the display
    lcd.clear();                                   
    lcd.print("Home Position");
    delay(100); 
    Set_Zero();
    delay(100); 
    stepper.setSpeedInStepsPerSecond(20000);
    stepper.setAccelerationInStepsPerSecondPerSecond(10000);
    while (digitalRead(limitSwitchPin) == HIGH) 
      {
        stepper.moveRelativeInSteps(-200);
      }
      dist = (-1)*((stepper.getCurrentPositionInMillimeters()*62)/800);       //returns how much distance it has gone up
    stepper.setCurrentPositionInSteps(0);
    delay(100); 
    return dist;
  }

/*_____________________________________________________________________________________________________________*/

// gets the feed for drilling (speed for drilling)
float speeds()
  {
    lcd.setCursor(0,0);                            // sets cursor to 0,0 and clears the display
    lcd.clear();                                   
    lcd.print("Enter Speed");                                // display the input operation
    delay(100); 
    speed = num_value();
    delay(100); 
    return speed;
  }

/*______________________________________________________________________________________________________________*/

// gets the value how much you want to drill
float pos_value()
  {
    lcd.setCursor(0,0);                            // sets cursor to 0,0 and clears the display
    lcd.clear();                                   
    lcd.print("Drilling Depth");
    delay(100); 
    pos = num_value();
    delay(100); 
    return pos;
  }

/*______________________________________________________________________________________________________________*/

void operation(float pos,float dist,float speed)          // main drill operation takes distance as pos take downward distance in dist and feed in speed
{
  // Starts the Drill bit moter and coolent pump by activating relays
  digitalWrite(relaymoter,HIGH);
  digitalWrite(relaypumpmoter,HIGH);
  /*----------------------------------------------------------------------------*/
  // goes near the job i.e given downward dist(home position)
  stepper.setSpeedInMillimetersPerSecond(33999);
  stepper.setAccelerationInMillimetersPerSecondPerSecond(3399);
  dist = (400*dist)/62;
  stepper.moveToPositionInMillimeters(dist);
  stepper.setCurrentPositionInSteps(0.0);

 // goes and drill the job with given with pos
  stepper.setSpeedInMillimetersPerSecond(speed);
  stepper.setAccelerationInMillimetersPerSecondPerSecond(150.0);
  drill_distance = (400*pos)/62;
  stepper.moveToPositionInMillimeters(drill_distance);
  delay(100);

  // goes upwards
  stepper.setSpeedInMillimetersPerSecond(33999);
  stepper.setAccelerationInMillimetersPerSecondPerSecond(3399);
  stepper.moveToPositionInMillimeters(-0.0);  
  stepper.moveToPositionInMillimeters(-dist); 

  // Switch off the drilling bit and Coolent Pump
  digitalWrite(relaymoter,LOW);
  digitalWrite(relaypumpmoter,LOW);
  delay(10);

}

/*_____________________________________________________________________________________________________________*/

// Drill Initilizer

void initilizer()
  {
    stepper.setSpeedInStepsPerSecond(20000);
    stepper.setAccelerationInStepsPerSecondPerSecond(10000);
    while (digitalRead(limitSwitchPin) == HIGH) 
      {
        stepper.moveRelativeInSteps(-200);
      }
    stepper.setCurrentPositionInSteps(0);
    delay(100); 
  }

/*________________________________________________________________________________________________________________*/

// Welcome program

void welcome()
  {
    lcd.init(); 
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Octagon Manufactring");  
    lcd.setCursor(2, 1);  
    lcd.print("leth machine"); 
    delay(3000); 
    lcd.clear(); 
  }

/*__________________________________________________________________________________________________________________*/


char button_command()                     // This function gets Which Character is entered
  {
    Serial.println("Enter Key");
    while (true)
      {
        char key = keypad.getKey();  
        delay(100);  
        if(key !=NO_KEY)
          {
            Serial.println(key);
            return key;
          }
      }
  }

void Set_Zero()
  {
 /* if(button_command()=='C')               // If C is entered than it starts to Set zero function
    {
      Serial.println("Set home Position");
    }
  */
    while(true)
      {
        
        char command = button_command();
          {
            if(command=='C')                                     // If C is entered than moter statrs to go down
              {
                Serial.println(" c");
                digitalWrite(MOTOR_DIRECTION_PIN,LOW); // Enables the motor to move in a particular direction
                for(int x = 0; x < 100; x++) 
                {
                  digitalWrite(MOTOR_STEP_PIN,HIGH); 
                  delayMicroseconds(2000); 
                  digitalWrite(MOTOR_STEP_PIN,LOW); 
                  delayMicroseconds(806.45); 
                }
              }
            else if (command =='A')                                    // If A is entered than Set_Zero Function stops
              {
                Serial.println(" A Set pos Zero");
                delay(100); 
                break;
              }
            else if(command=='D')                                    // If D is entered than moter statrs to go up
              {
                Serial.println(" D");
                digitalWrite(MOTOR_DIRECTION_PIN,HIGH); //Changes the direction of rotation
                for(int x = 0; x < 100; x++) 
                {
                    digitalWrite(MOTOR_STEP_PIN,HIGH);
                    delayMicroseconds(2000);
                    digitalWrite(MOTOR_STEP_PIN,LOW);
                    delayMicroseconds(806.45);
                }
              }
          }
      }
  }
/*__________________________________________________________________________________________________________________*/
int main()
  {
    /*-----------------------------------------------------------------------------------------------------*/
    // setup pins input and output
      Serial.begin(9600);
      inputString.reserve(10); // maximum number of digit for a number is 10, change if needed
      pinMode(BUTTON_PIN, INPUT_PULLUP);
      pinMode(limitSwitchPin, INPUT_PULLUP);
      stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
      stepper.setStepsPerMillimeter(25 * 1);    // 1x microstepping
      pinMode(relaymoter,OUTPUT);
      pinMode(relaypumpmoter,OUTPUT);

    /*-----------------------------------------------------------------------------------------------------*/

    // Welcomes
    Serial.println("Welcomes");
     welcome();

    /*------------------------------------------------------------------------------------------------------*/

    // initilizing the drill
    Serial.println("Settingup drill");
    lcd.setCursor(0, 0); 
    lcd.print("Setting Up Drill");
    delay(2000);
    initilizer();
    lcd.clear();
    lcd.print("Completed");
    delay(2000);
    lcd.clear();

    /*------------------------------------------------------------------------------------------------------*/
    
    // taking Home Distance
    Serial.println("Home pos");
    delay(100); 
    dist = before_drill() ;
    lcd.clear();

    /*------------------------------------------------------------------------------------------------------*/
    // taking feed for drill
    Serial.println("feed");
    delay(100); 
    speed = int(speeds());
    lcd.clear();

    /*------------------------------------------------------------------------------------------------------*/


    // taking how much drill is to be done
    Serial.println("drill depth");
    delay(100); 
    pos = pos_value();
    lcd.clear();

    lcd.setCursor( 0, 0 );                  // Displays the enterd values
    lcd.print("Home feed Depth");
    lcd.setCursor(0,1);
    lcd.print(dist);
    lcd.setCursor(6,1);
    lcd.print(speed);
    lcd.setCursor(10,1);
    lcd.print(pos);

  

/*_________________________________________________________________________________________________________________________________*/


while(true)
  {
    while(digitalRead(BUTTON_PIN) == HIGH)
      {
        //Serial.println("no Signal");
        delay(10);
      }
    Serial.println("Signal");
    delay(100); 
    operation(pos,dist,speed);  //performs operation
    delay(100); 
    initilizer();     // uncomment this only if limit Switch is working fine
    
  }
  }
