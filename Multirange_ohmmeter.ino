//TODO 400
#include <LiquidCrystal.h>
#define NUM_REF_RESISTORS 8
#define NUM_SELECT_PINS   3
#define MAX_ANALOG_VALUE 973        
#define SWITCH_RESISTANCE 49// FOR CALIBRATION
float rRef[NUM_REF_RESISTORS] = {85, 195, 850, 1200, 18000, 92000, 900000, 9000000};//x0 x1 x2 x3 x4 x5 x6 x7 FOR CALIBRATION


const byte rSelPins[NUM_SELECT_PINS] = {3, 2, 1};//A  B  C
const byte enableMux = 0; // 1 = no connection, 0 = one of eight signals connected
int screenWidth, screenHeight;

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);                                                                          

void setup()
{
  pinMode(enableMux, OUTPUT);
  digitalWrite(enableMux, HIGH);      
  
  for (int i = 0; i < NUM_SELECT_PINS; i++)
  {
    pinMode(rSelPins[i], OUTPUT);     
    digitalWrite(rSelPins[i], HIGH);  
  }
 // Initialize the LCD display
  lcd.begin(16, 2);

}
// This function scales the resistor value

char ScaleToMetricUnits(float *prVal, char fStr[])
{
  char unit;
  if (*prVal < 1000)
  {
    unit = ' ';
  }
  else if (*prVal >= 1000 && *prVal < 1000000)
  {
    *prVal /= 1000;
    unit = 'K';
  }
  else if (*prVal >= 1000000 && *prVal < 1000000000)
  {
    *prVal /= 1000000;
    unit = 'M';
  }
  else
  {
    *prVal /= 1000000000;
    unit = 'G';
  }
  for (int k=2, s=10; k >= 0; k--, s*=10)
  {
    if ((int)(*prVal) / s == 0)
    {
      dtostrf(*prVal, 4, k, fStr); 
      break;
    }
  }
  return unit;
}

void DisplayResultsOnLCDScreen(char unit, char fStr[])
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ArduinOhmmeter");
  lcd.setCursor(0, 1);
  if (unit != 0)
  {
    lcd.print(fStr);
    lcd.print(" ");
    lcd.print(unit);
  }
  else
  {
    lcd.print("- - -");
  }
}
void loop()
{
  int cOut;
  float delta, deltaBest1 = MAX_ANALOG_VALUE, deltaBest2 = MAX_ANALOG_VALUE;
  float rBest1 = -1, rBest2 = -1, rR, rX;
  char unit = 0, fStr[16];
  for (byte count = 0; count < NUM_REF_RESISTORS; count++)
  {
    digitalWrite(rSelPins[0], count & 1); // C
    digitalWrite(rSelPins[1], count & 2); // B
    digitalWrite(rSelPins[2], count & 4); // A
    
    digitalWrite(enableMux, LOW);       
    delay(count + 800);                   
    cOut = analogRead(A0);              
    digitalWrite(enableMux, HIGH);      
    delay(NUM_REF_RESISTORS - count);   
    //valid digitized values
    if (cOut < MAX_ANALOG_VALUE)
    {
      rR = rRef[count] + SWITCH_RESISTANCE; 
      rX = (rR * cOut) / (MAX_ANALOG_VALUE - cOut);
      delta = (MAX_ANALOG_VALUE / 2.0 - cOut);
      if (fabs(delta) < fabs(deltaBest1))
      {
        deltaBest2 = deltaBest1;
        rBest2 = rBest1;
        deltaBest1 = delta;
        rBest1 = rX;
      }
      else if (fabs(deltaBest2) > fabs(delta))
      {
        deltaBest2 = delta;
        rBest2 = rX;
      }
    }
  }
  
  if (rBest1 >= 0 && rBest2 >= 0)
  {
    if (deltaBest1 * deltaBest2 < 0)
    {
      rX = rBest1 - deltaBest1 * (rBest2 - rBest1) / (deltaBest2 - deltaBest1); 
    }
    else
    {
      rX = rBest1;  
    }
    unit = ScaleToMetricUnits(&rX, fStr);
  }
  DisplayResultsOnLCDScreen(unit, fStr);
  delay(250);
}
