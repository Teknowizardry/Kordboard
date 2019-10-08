#include <Keyboard.h>
#include <Mouse.h>

/*
 * KordBoard v1.0
 Code for Arduino Pro Micro
Adapted by Matt Taylor, Teknowizardry
Original Programmed by Brian McEvoy. 24 Hour Engineer
24hourengineer.com
Program is distributable for personal use.
*/
#define OSX 0
#define OTHER_OS 1
int platform = OSX;

//These strings can be made to appear with special macro chords.
//Fill in your details if you wish to use them, or any other text you use often.
String fullName = "Full Name";
String email = "me@email.com";
String username = "username";
String pass_osx = "pass_osx";
String pass_other = "pass_other";
String usernum = "usernum";
String pass_num = "pass_num";


int platformToggle = 2; //pin for the jumper to tell the keyboard what OS it's on; connected to ground=OSX
// Inputs. Buttons may be addressed by name but the program expects all buttons after the pinky
// to be numbered sequentially.
int pinkyButton = 3;
int ringButton = 4;
int middleButton =5;
int indexButton = 6;
int nearTButton = 7;
int centerTButton = 8;
int farTButton = 9;

// Program integers
int prefixChord = 0;            // 1 = shift (F). 2 = numlock (N). 3 = special (CN). 4 = function keys
int chordValue = 0;
int randomNumber01;
int randomNumber02;
int randomNumber03;
int randomNumber04;
int debounceDelay = 20;
int mouseDelay = 40;
int mouseValues[] = {0,0,0};
int startingMouseValues[] = {0,0,0}; 
int mouseDivisionValues[] = {-15,-3,2}; //{-15,-3,1.5}; //z,y,x //sensitivity
int mouseOffset[] = {0,-55,10}; //-1024->1024
//X=A2 (vertical), Y=A1 (horizontal), Z=A0

int sign1 = 0;
int sign2 = 0;

// Booleans
boolean buttons[7];     // Pinky is [0] and far thumb is [6]
boolean latchingButtons[7];
boolean acquiringPresses = LOW;
boolean calculateKey = LOW;
boolean stickyCapsLock = LOW;
boolean stickyNumlock = LOW;
boolean stickySpecialLock = LOW;
boolean altChords = HIGH; //set to HIGH for alternates (key chart I provide)

void setup(){
  Serial1.begin(9600);
  Serial.begin(9600);
  Serial.println("Up and runnning");
  Keyboard.begin();
  Mouse.begin();
  
  randomSeed(analogRead(0));
  
  pinMode(platformToggle, INPUT_PULLUP);
  pinMode(pinkyButton, INPUT_PULLUP);
  pinMode(ringButton, INPUT_PULLUP);
  pinMode(middleButton, INPUT_PULLUP);
  pinMode(indexButton, INPUT_PULLUP);
  pinMode(nearTButton, INPUT_PULLUP);
  pinMode(centerTButton, INPUT_PULLUP);
  pinMode(farTButton, INPUT_PULLUP);

  if (!digitalRead(platformToggle)) {
    platform = OSX;
  } else {
    platform = OTHER_OS;
  }
}

void loop(){
  acquiringPresses = checkButtonArray();
  
  if (acquiringPresses && onlyFarThumbPressed(farTButton)){
    doMouseSTUFF();
  }
  
  if (acquiringPresses){
    delay(debounceDelay);                           // Instead of a true software debounce this will wait a moment until the first button press has settled.
    typingChord();                      // Wait and see which keys are touched. When they are all released print the correct letter.
    updateShiftKeys();          // Change the prefixChord value if any of the 'locks' are set. Example, Num Lock or Caps Lock.
    sendKeyPress();                     // Using the buttons pressed during the typingChord function determine how to handle the chord.
    delay(debounceDelay);                           // The other half of the software "debounce"
    for (int i = 0; i < 7; i++){    // Once a keypress has been sent the booleans should be reset.
      latchingButtons[i] = LOW;
    }
    chordValue = 0;
  }
}

void doMouseSTUFF(){
  for (int i = 0; i < 3; i++){
    startingMouseValues[i] = analogRead(i);
  }
  delay(debounceDelay);
  while (onlyFarThumbPressed(farTButton)){
    delay(mouseDelay);
    for (int i = 0; i < 3; i++){
      int reading = analogRead(i);
      mouseValues[i] = reading - startingMouseValues[i] + mouseOffset[i];
    }
    for (int i = 0; i < 3; i++){
      mouseValues[i] = mouseValues[i] / mouseDivisionValues[i];
    }

    Mouse.move((mouseValues[1]), (mouseValues[2]), (mouseValues[0]));
    Serial1.write(0xFD);
    Serial1.write((byte)0x00);
    Serial1.write((byte)0x03);
    Serial1.write((byte)0x00);            // Buttons
    Serial1.write((byte)mouseValues[1]);      // X axis
    Serial1.write((byte)mouseValues[2]);     // Y axis
    Serial1.write((byte)0x00);
    Serial1.write((byte)0x00);
    Serial1.write((byte)0x00);
  }
}

boolean onlyFarThumbPressed(int functionMaxButton){
  for (int i = functionMaxButton - 1; i > (functionMaxButton - 7); i--){
    if(!digitalRead(i)){
      return LOW;
    }
  }
  if (!digitalRead(functionMaxButton)){
    return HIGH;
  }else{
    return LOW;
  }
}
/*boolean onlyFarThumbPressed(int function_fButton){
  for (int i = function_fButton +1; i < (function_fButton + 7); i++){
    if(!digitalRead(i)){
      return LOW;
    }
  }
  if (!digitalRead(function_fButton)){
    return HIGH;
  }else{
    return LOW;
  }
}*/

void updateShiftKeys(){
  if (stickyCapsLock){
    prefixChord = 1;
  }
  if (stickyNumlock){
    prefixChord = 2;
  }
  if (stickySpecialLock){
    prefixChord = 3;
  }
}

boolean checkButtonArray(){
  // Update the buttons[] array with each scan. Set the acquiringPresses bit HIGH if any switch is pressed.
  for (int i = 0; i < 7; i++){
    boolean buttonState = !digitalRead(pinkyButton + i);
    /*int conversion = pinkyButton;
    if (i==0) {
      conversion = pinkyButton; 
    } else if (i==1) {
      conversion = ringButton;
    } else if (i==2) {
      conversion = middleButton;
    } else if (i==3) {
      conversion = indexButton;
    } else if (i==4) {
      conversion = nearTButton;
    } else if (i==5) {
      conversion = centerTButton;
    } else if (i==6){
      conversion = farTButton;
    }
    boolean buttonState = !digitalRead(conversion);*/
    if (buttonState){
      buttons[i] = HIGH;
    }else{
      buttons[i] = LOW;
    }
  }
  for (int i = 0; i < 7; i++){
    if (buttons[i]){
      return HIGH;
    }
  }
  return LOW;
}

void typingChord(){
  while (acquiringPresses){
    for (int i = 0; i < 7; i++){
      if (buttons[i] == HIGH){
        latchingButtons[i] = HIGH;
      }
    }
    acquiringPresses = checkButtonArray();
  }
}

void sendKeyPress(){
  for (int i = 0; i < 7; i++){
    if (latchingButtons[i] == HIGH){
      chordValue = chordValue + customPower(2, i);
    }
  }
  Serial1.write(keySwitch(chordValue));
}

int customPower(int functionBase, int functionExponent){
  int functionResult = 1;
  for (int i = 0; i < functionExponent; i++){
    functionResult = functionResult * functionBase;
  }
  return functionResult;
}

int keySwitch(int functionChordValue){
  switch (functionChordValue){
    case 0:
      prefixChord = 0;
      Keyboard.releaseAll();
      stickyNumlock = LOW;
      stickyCapsLock = LOW;
      stickySpecialLock = LOW;
      return 0;     // error
    case 1:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
              //alternate
Keyboard.print('o');
          return 111;          // 111 → o
     
          } else {
              Keyboard.print('w');
              return 119;          // 119 is equivalent to the letter 'w'
          }  
        case 1:
          prefixChord = 0;
          prefixChord = 0;
          if (altChords) {
             //alternate
      Keyboard.print('O');
          return 79;
          } else {
           Keyboard.print('W');
           return 87;
          } 
        case 2:
          prefixChord = 0;
          prefixChord = 0;
          Keyboard.print('5');
          return 53;
        case 3:
          prefixChord = 0;
          prefixChord = 0;
          Keyboard.print('%');
          return 37;
        case 4:
          prefixChord = 0;
          Keyboard.write(198);
          return 198;
      }
    case 2:
      delay(debounceDelay*3);
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('a');
          return 97;          // 97 → a
        } else {
    Keyboard.print('y');
          return 121;          // 121 is equivalent to the letter 'y'
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        
    Keyboard.print('A');
        
          return 65;
        } else {
    Keyboard.print('Y');
          return 89;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('4');
          return 52;
        case 3:
          prefixChord = 0;
          Keyboard.print('$');
          return 36;
        case 4:
          prefixChord = 0;
          Keyboard.write(197);
          return 197;
      }
    case 3:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('r');
          return 114;          // 114 → r
        } else {
    Keyboard.print('u');
          return 117;          // 117 is equivalent to the letter 'u'
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
         Keyboard.print('R');
          return 82;
        } else {
    Keyboard.print('U');
          return 85;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('"');
          Keyboard.print('"');
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.releaseAll();
          Serial1.print('"');
          Serial1.print('"');
          return 0x0B;
        case 3:
          prefixChord = 0;
          Keyboard.print('"');
          Keyboard.print('"');
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.releaseAll();
          Serial1.print('"');
          Serial1.print('"');
          return 0x0B;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 4:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('t');
          return 116;          // 116 → t
        } else {
    Keyboard.print('r');
          return 114;          // 114 → r
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        
    Keyboard.print('T');
          return 84;
        } else {
    Keyboard.print('R');
          return 82;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('3');
          return 45;
        case 3:
          prefixChord = 0;
          Keyboard.print('#');
          return 35;
        case 4:
          prefixChord = 0;
          Keyboard.write(196);
          return 196;
      }
    case 5:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('h');
          return 104;          // 104 → h
        } else {
    Keyboard.print(email);
          Serial1.print(email);
          return 0;
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('H');
          return 72;
        } else {
    Keyboard.print(username);
          Serial1.print(username);
          return 0;
      } 
        case 2:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print(email);
          Serial1.print(email);
          return 0;
        } else {
    Keyboard.print(pass_osx);
          Serial1.print(pass_osx);
          return 0;
      } 
        case 3:
          prefixChord = 0;
          if (altChords) {
        //alternate
            switch (platform) {
              case OSX:
                Keyboard.print(pass_osx);
                Serial1.print(pass_osx);
                return 0;
              default:
                Keyboard.print(pass_other);
                Serial1.print(pass_other);
                return 0;
            } 
        } else {
    Keyboard.print(fullName);
          Serial1.print(fullName);
          return 0;
      } 
        case 4:
          prefixChord = 0;
          randomNumber01 = random(1, 20);
          Keyboard.print(randomNumber01);
          Serial1.print(randomNumber01);
          return 0;
      }
    case 6:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('n');
          return 110;          // 110 → n
     
        } else {
    Keyboard.print('h');
          return 104;          // 104 → h
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('N');
          return 78;
        } else {
    Keyboard.print('H');
          return 72;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('0');
          Keyboard.print('0');
          Serial.print('0');
          return 48;
        case 3:
          prefixChord = 0;
          Keyboard.print('0');
          Keyboard.print('0');
          Serial.print('0');
          return 48;
        case 4:
          prefixChord = 0;
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(110); //xxx  Command N (New)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(110); //xxx  Ctrl N (New)
              Keyboard.releaseAll();
              return 0;
          }
      }
    case 7:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('l');
          return 108;          // 108 → l (lowercase L)
  
        } else {
    Keyboard.print('s');
          return 115;          // 115 → s
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
           Keyboard.print('L');
          return 76;
        } else {
    Keyboard.print('S');
          return 83;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('-');
          return 45;
        case 3:
          prefixChord = 0;
          Keyboard.print('_');
          return 95;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 8:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('e');
          return 101;          // 101 → e
        } else {
    Keyboard.print('i');
          return 105;          // 105 → i
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('E');
          return 69;
        } else {
    Keyboard.print('I');
          return 73;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('2');
          return 50;
        case 3:
          prefixChord = 0;
          Keyboard.print('@');
          return 64;
        case 4:
          prefixChord = 0;
          Keyboard.write(195);
          return 195;
      }
    case 9:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.write(KEY_TAB);
        Serial1.print('\t');
          return 0xB3;
        } else {
    Keyboard.print('b');
          return 98;          // 98 → b
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        //KEY_TAB  0xB3  179
        Keyboard.write(KEY_TAB);
        Serial1.print('\t');
          return 0xB3;
        } else {
    Keyboard.print('B');
          return 66;   
      } 
        case 2:
          prefixChord = 0;
          Keyboard.write(92);
          return 92;
        case 3:
          prefixChord = 0;
          Keyboard.print('|');
          return 124;
        case 4:
          prefixChord = 0;
          Keyboard.write(205);
          return 205;
      }
    case 10:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('s');
          return 115;          // 115 → s
      
        } else {
    Keyboard.print('k');
          return 107;          // 107 → k
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('S');
          return 83;
        } else {
    Keyboard.print('K');
          return 75;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('$');
          return 36;
        case 3:
          prefixChord = 0;
          Keyboard.print('$');
          return 36;
        case 4:
          prefixChord = 0;
          switch (platform){
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(119); //xxx  Command W (Close window)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(119); //xxx  Ctrl W (Close Window)
              Keyboard.releaseAll();
              return 0;
          }
      }
    case 11:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('u');
          return 117;          // 117 is equivalent to the letter 'u'

        } else {
    Keyboard.print('z');
          return 122;          // 122 → z
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('U');
          return 85;
        } else {
    Keyboard.print('Z');
          return 90;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('`');
          return 96;
        case 3:
          prefixChord = 0;
          Keyboard.print('~');
          return 126;
        case 4:
          prefixChord = 0;
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(117); //xxx  Command U (Upload in Arduino)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(117); //xxx  Ctrl U (?)
              Keyboard.releaseAll();
              return 0;
          }
      }
    case 12:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('i');
          return 105;          // 105 → i
      
        } else {
    Keyboard.print('d');
          return 100;          // 100 → d
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        
    Keyboard.print('I');
          return 73;
        } else {
    Keyboard.print('D');
          return 68;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('/');
          return 47;
        case 3:
          prefixChord = 0;
          Keyboard.print('?');
          return 63;
        case 4:
          prefixChord = 0;
          switch (platform){
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(116); //xxx  Command T (New Tab)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(116); //xxx  Ctrl T (New Tab)
              Keyboard.releaseAll();
              return 0;
          }
      }
    case 13:                  // This chord is open
    randomNumber01 = random(0, 255);
    randomNumber02 = random(0, 1000);
    randomNumber03 = random(0, 4000);
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('c');
          return 99;          // 99 → c

        } else {
    sign1 = random(0,4);
          sign2 = random(0,4);
          Keyboard.print(randomNumber01);
          Serial1.print(randomNumber01);
          if (sign1 == 0){
            Keyboard.print('/');
            Serial1.print('/');
          }
          if (sign1 == 1){
            Keyboard.print('*');
            Serial1.print('*');
          }
          if (sign1 == 2){
            Keyboard.print('-');
            Serial1.print('-');
          }
          if (sign1 == 3){
            Keyboard.print('+');
            Serial1.print('+');
          }
          Keyboard.print(randomNumber02);
          Serial1.print(randomNumber02);
          if (sign2 == 0){
            Keyboard.print('/');
            Serial1.print('/');
          }
          if (sign2 == 1){
            Keyboard.print('*');
            Serial1.print('*');
          }
          if (sign2 == 2){
            Keyboard.print('-');
            Serial1.print('-');
          }
          if (sign2 == 3){
            Keyboard.print('+');
            Serial1.print('+');
          }
          Keyboard.print(randomNumber03);
          Serial1.print(randomNumber03);
          delay(5);
          Keyboard.print('\n');
          return 10;
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('C');
          return 67;
        } else {
    Keyboard.print("0.");
          Serial1.print("0.");
          randomNumber04 = random(0, 10);
          Keyboard.print(randomNumber04);
          Serial1.print(randomNumber04);
          randomNumber04 = random(0, 10);
          Keyboard.print(randomNumber04);
          Serial1.print(randomNumber04);
          randomNumber04 = random(0, 10);
          Keyboard.print(randomNumber04);
          Serial1.print(randomNumber04);
          randomNumber04 = random(0, 10);
          Keyboard.print(randomNumber04);
          Serial1.print(randomNumber04);
          return 0;
      } 
        case 2:
          Keyboard.print(fullName);
          Serial1.print(fullName);
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          Keyboard.print(username);
          Serial1.print(username);
          return 0;
        case 4:
          prefixChord = 0;
          Keyboard.print(randomNumber01);
          Serial1.print(randomNumber01);
          return 0;
      }
    case 14:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('d');
          return 100;          // 100 → d

        } else {
    Keyboard.print('e');
          return 101;          // 101 → e
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('D');
          return 68;
        } else {
    Keyboard.print('E');
          return 69;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('=');
          return 61;
        case 3:
          prefixChord = 0;
          Keyboard.print('+');
          return 43;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 15:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('m');
          return 109;          // 109 → m

        } else {
    Keyboard.print('t');
          return 116;          // 116 → t
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('M');
          return 77;
        } else {
    Keyboard.print('T');
          return 84;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('0');
          Keyboard.print('0');
          Keyboard.print('0');
          Serial1.print('0');
          Serial1.print('0');
          return 48;
        case 3:
          prefixChord = 0;
          Keyboard.print('0');
          Keyboard.print('0');
          Keyboard.print('0');
          Serial1.print('0');
          Serial1.print('0');
          return 48;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 16:
      stickyCapsLock = LOW;
      stickyNumlock = LOW;
      switch (prefixChord){
        case 0:
          prefixChord = 2;
          return 0;       // Set the 'numlock' when only the near thumb key is pressed.
        case 1:
          prefixChord = 2;
          return 0;
        case 2:
          stickyNumlock = !stickyNumlock; // When this is pressed a second time toggle it.
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 2;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 17:
      stickyCapsLock = LOW;
      stickyNumlock = LOW;
      stickySpecialLock = LOW;
      switch (prefixChord){
        case 0:
          prefixChord = 4;
          return 0;
        case 1:
          prefixChord = 4;
          return 0;
        case 2:
          prefixChord = 4;
          return 0;
        case 3:
          prefixChord = 4;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 18:
      stickyCapsLock = LOW;
      stickyNumlock = LOW;
      stickySpecialLock = LOW;
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(27);
          return 0x1B;
        case 1:
          prefixChord = 0;
          Keyboard.write(27);
          return 0x1B;
        case 2:
          prefixChord = 0;
          Keyboard.write(27);
          return 0x1B;
        case 3:
          prefixChord = 0;
          Keyboard.write(27);
          return 0x1B;
        case 4:
          prefixChord = 0;
          Keyboard.write(27);
          return 0x1B;
      }
    case 19:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.print(';');
          return 59;
        case 1:
          prefixChord = 0;
          Keyboard.print(':');
          return 58;
        case 2:
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 20:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.print(',');
          return 44;
        case 1:
          prefixChord = 0;
          Keyboard.print('<');
          return 60;
        case 2:
          prefixChord = 0;
          switch (platform){
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(115); //xxx  Command S (Save)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(115); //xxx  Ctrl S (Save)
              Keyboard.releaseAll();
              return 0;
          }
          
        case 3:
          prefixChord = 0;
          switch (platform){
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(112); //xxx  Command P (Print)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(112); //xxx  Ctrl P (Print)
              Keyboard.releaseAll();
              return 0;
          }
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 21:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(32); //xxx  Quicksilver
          delay(debounceDelay*3);
          Keyboard.releaseAll();
          return 0;
        case 1:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(32); //xxx
          delay(debounceDelay*3);
          Keyboard.releaseAll();
          return 0;
        case 2:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(32); //xxx
          delay(debounceDelay*3);
          Keyboard.releaseAll();
          return 0;
        case 3:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_CTRL);
          Keyboard.press(32); //xxx
          Keyboard.releaseAll();
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 22:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.print('.');
          return 46;
        case 1:
          prefixChord = 0;
          Keyboard.print('>');
          return 62;
        case 2:
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 23:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ALT);
          return 0xE2;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ALT);
          return 0xE2;
        case 2:
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 24:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_GUI);
          return 0x83;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_GUI);
          return 0x83;
        case 2:
          prefixChord = 0;
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(99); //xxx  Command C (Copy)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(99); //xxx  Ctrl C (Copy)
              Keyboard.releaseAll();
              return 0;
          }
        case 3:
          prefixChord = 0;
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(118); //xxx  Command V (Paste)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(118); //xxx  Ctrl V (Paste)
              Keyboard.releaseAll();
              return 0;
          }
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 25:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.print("975000130");
          Serial1.print("975000130");
          return 0;
        case 1:
          Keyboard.print("MATTHEW504");
          Serial1.print("MATTHEW504");
          return 0;
        case 2:
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 26:
      switch (prefixChord){
        case 0:
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(KEY_TAB); //Switch to next app
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.press(KEY_F7);
              Keyboard.releaseAll();
              return 0;
          }
        case 1:
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(KEY_LEFT_SHIFT);
              Keyboard.press(KEY_TAB); //Switch to last app
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.press(KEY_F7);
              Keyboard.releaseAll();
              return 0;
          }
        case 2:
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 27:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_CTRL);
          return 0xE0;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_CTRL);
          return 0xE0;
        case 2:
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 28:
      switch (prefixChord){
        case 0:
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(32); //Open quicksilver
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.press(KEY_F2);
              Keyboard.releaseAll();
              return 0;
          }
        case 1:
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(32); //Open quicksilver
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.press(KEY_F2);
              Keyboard.releaseAll();
              return 0;
          }
        case 2:
          prefixChord = 0;
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(113); //xxx  Command Q (Quit)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(113); //xxx  Ctrl Q (Quit?)
              Keyboard.releaseAll();
              return 0;
          }
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 29:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx Spotlight Search
          delay(debounceDelay*3);
          Keyboard.releaseAll();
          return 0;
        case 1:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx
          delay(debounceDelay*3);
          Keyboard.releaseAll();
          return 0;
        case 2:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx
          delay(debounceDelay*3);
          Keyboard.releaseAll();
          return 0;
        case 3:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx
          Keyboard.releaseAll();
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 30:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(39);
          return 39;
        case 1:
          prefixChord = 0;
          Keyboard.write(34);
          return 34;
        case 2:
          prefixChord = 0;
          return 0;
        case 3:
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 31:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx SIRI
          delay(1000);
          Keyboard.releaseAll();
          return 0;
        case 1:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx
          delay(1000);
          Keyboard.releaseAll();
          return 0;
        case 2:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx
          delay(1000);
          Keyboard.releaseAll();
          return 0;
        case 3:
          prefixChord = 0;
          Keyboard.press(KEY_LEFT_GUI);
          Keyboard.press(32); //xxx
          delay(1000);
          Keyboard.releaseAll();
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 32:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.print(' ');
          return 32;          // 32  → 'space'
        case 1:
          prefixChord = 0;
          Keyboard.print(' ');
          return 32;
        case 2:
          prefixChord = 0;
          Keyboard.print('1');
          return 49;
        case 3:
          prefixChord = 0;
          Keyboard.print('!');
          return 33;
        case 4:
          prefixChord = 0;
          Keyboard.write(194);
          return 194;
      }
    case 33:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
         Keyboard.print('g');
          return 103;          // 103 → g

        } else {
    Keyboard.print('f');
          return 102;          // 102 → f
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('G');
          return 71;
        } else {
    Keyboard.print('F');
          return 70;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('9');
          return 57;
        case 3:
          prefixChord = 0;
          Keyboard.print('(');
          return 40;
        case 4:
          prefixChord = 0;
          Keyboard.write(202);
          return 202;
      }
    case 34:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
         Keyboard.print('w');
              return 119;          // 119 is equivalent to the letter 'w'
 
        } else {
    Keyboard.print('g');
          return 103;          // 103 → g
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
         Keyboard.print('W');
           return 87;
        } else {
    Keyboard.print('G');
          return 71;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('8');
          return 56;
        case 3:
          prefixChord = 0;
          Keyboard.print('*');
          return 42;
        case 4:
          prefixChord = 0;
          Keyboard.write(201);
          return 201;
      }
    case 35:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('k');
          return 107;          // 107 → k

        } else {
    Keyboard.print('v');
          return 118;          // 118 → v
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('K');
          return 75;
        } else {
    Keyboard.print('V');
          return 86;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print(']');
          return 91;
        case 3:
          prefixChord = 0;
          Keyboard.print('}');
          return 125;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 36:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('y');
          return 121;          // 121 is equivalent to the letter 'y'

        } else {
    Keyboard.print('c');
          return 99;          // 99 → c
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('Y');
          return 89;
        } else {
    Keyboard.print('C');
          return 67;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('7');
          return 55;
        case 3:
          prefixChord = 0;
          Keyboard.print('&');
          return 38;
        case 4:
          prefixChord = 0;
          Keyboard.write(200);
          return 200;
      }
    case 37:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('x');
          return 120;          // 120 → x

        } else {
    Keyboard.print(']');
          return 93;          // 93 → ]
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('X');
          return 88;
        } else {
    Keyboard.print('}');
          return 125;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print(']');
          return 93;
        case 3:
          prefixChord = 0;
          Keyboard.print('}');
          return 125;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 38:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('b');
          return 98;          // 98 → b

        } else {
    Keyboard.print('p');
          return 112;          // 112 → p
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('B');
          return 66;
        } else {
    Keyboard.print('P');
          return 80;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('%');
          return 37;
        case 3:
          prefixChord = 0;
          Keyboard.print('%');
          return 37;
        case 4:
          prefixChord = 0;
          randomNumber01 = random(1, 100);
          Keyboard.print(randomNumber01);
          Serial1.print(randomNumber01);
          return 0;
      }
    case 39:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('q');
          return 113;          // 113 → q


        } else {
    Keyboard.print('n');
          return 110;          // 110 → n
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('Q');
          return 81;
        } else {
    Keyboard.print('N');
          return 78;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('[');
          return 91;
        case 3:
          prefixChord = 0;
          Keyboard.print('{');
          return 123;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 40:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
         if (altChords) {
        //alternate
            Keyboard.print('f');
          return 102;          // 102 → f
        } else {
     Keyboard.print('l');
          return 108;          // 108 → l (lowercase L)
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('F');
          return 70;
        } else {
    Keyboard.print('L');
          return 76;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('6');
          return 54;
        case 3:
          prefixChord = 0;
          Keyboard.print('^');
          return 94;
        case 4:
          prefixChord = 0;
          Keyboard.write(199);
          return 199;
      }
    case 41:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print("sudo ");
        Serial1.print("sudo ");
          return 0;
        } else {
    Keyboard.print('x');
          return 120;          // 120 → x
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print("sudo ");
        Serial1.print("sudo ");
          return 0;
        } else {
    Keyboard.print('X');
          return 88;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('&');
          return 38;
        case 3:
          prefixChord = 0;
          Keyboard.print('&');
          return 38;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 42:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.print('v');
          return 118;          // 118 → v

        } else {
    Keyboard.print('j');
          return 106;          // 106 → j
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('V');
          return 86;
        } else {
    Keyboard.print('J');
          return 74;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('(');
          Keyboard.print(')');
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.releaseAll();
          Serial1.print('(');
          Serial1.print(')');
          return 0x0B;
        case 3:
          prefixChord = 0;
          Keyboard.print('(');
          Keyboard.print(')');
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.releaseAll();
          Serial1.print('(');
          Serial1.print(')');
          return 0x0B;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 43:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('z');
          return 122;          // 122 → z


        } else {
    Keyboard.print('q');
          return 113;          // 113 → q
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('Z');
          return 90;
        } else {
    Keyboard.print('Q');
          return 81;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('?');
          return 63;
        case 3:
          prefixChord = 0;
          Keyboard.print('?');
          return 63;
        case 4:
          prefixChord = 0;
          switch (platform) {
            case OSX:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(122); //xxx  Command Z (Undo)
              Keyboard.releaseAll();
              return 0;
            default:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(122); //xxx  Ctrl Z (Undo?)
              Keyboard.releaseAll();
              return 0;
          }
      }
    case 44:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
                    Keyboard.print('p');
          return 112;          // 112 → p
        } else {
    Keyboard.print('m');
          return 109;          // 109 → m
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate

    Keyboard.print('P');
          return 80;
        } else {
    Keyboard.print('M');
          return 77;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('*');
          return 42;
        case 3:
          prefixChord = 0;
          Keyboard.print('*');
          return 42;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 45:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.write(KEY_LEFT_GUI);
          return 0x83;
        } else {
    Keyboard.print('[');
          return 91;          // 91 → [
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
        Keyboard.write(KEY_LEFT_GUI);
          return 0x83;
        } else {
    Keyboard.print('{');
          return 123;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('[');
          return 91;
        case 3:
          prefixChord = 0;
          Keyboard.print('{');
          return 123;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 46:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('j');
          return 106;          // 106 → j

        } else {
    Keyboard.print('a');
          return 97;          // 97 → a
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
            Keyboard.print('J');
          return 74;
        } else {
    Keyboard.print('A');
          return 65;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('+');
          return 43;
        case 3:
          prefixChord = 0;
          Keyboard.print('+');
          return 43;
        case 4:
          prefixChord = 0;
          Keyboard.write(204);
          return 204;
      }
    case 47:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          if (altChords) {
        //alternate
                  Keyboard.print('0');
          return 48;
        } else {
    Keyboard.print('o');
          return 111;          // 111 → o
      } 
        case 1:
          prefixChord = 0;
          if (altChords) {
        //alternate
                  Keyboard.print('0');
          return 48;
        } else {
    Keyboard.print('O');
          return 79;
      } 
        case 2:
          prefixChord = 0;
          Keyboard.print('0');
          return 48;
        case 3:
          prefixChord = 0;
          Keyboard.print(')');
          return 29;
        case 4:
          prefixChord = 0;
          Keyboard.write(203);
          return 203;
      }
    case 48:
      stickyCapsLock = LOW;
      stickyNumlock = LOW;
      switch (prefixChord){
        case 0:
          prefixChord = 3;
          return 0;       // Set the sticky 'special characters' when only the near thumb key is pressed.
        case 1:
          prefixChord = 3;
          return 0;
        case 2:
          prefixChord = 3;
          return 0;
        case 3:
          stickySpecialLock = !stickySpecialLock;
          prefixChord = 0;
          return 0;
        case 4:
          prefixChord = 0;
          return 0;
      }
    case 64:
      stickyNumlock = LOW;
      stickySpecialLock = LOW;
      switch (prefixChord){
        case 0:
          prefixChord = 1;
          return 0;       // Set the sticky 'shift' when only the near thumb key is pressed.
        case 1:
          stickyCapsLock = !stickyCapsLock;
          prefixChord = 0;
          return 0;
        case 2:
          prefixChord = 1;
          return 0;
        case 3:
          prefixChord = 1;
          return 0;
        case 4:
          prefixChord = 1;
          return 0;
      }
    case 65:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.print('\n');
          return 10;          // 10 → enter
        case 1:
          prefixChord = 0;
          Keyboard.print('\n');
          return 10;          // 10 → enter
        case 2:
          prefixChord = 0;
          Keyboard.print('\n');
          return 10;          // 10 → enter
        case 3:
          prefixChord = 0;
          Keyboard.print('\n');
          return 10;          // 10 → enter
        case 4:
          prefixChord = 0;
          Keyboard.print('\n');
          return 10;          // 10 → enter
      }
    case 66:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Mouse.click(MOUSE_RIGHT);
          return 0;
        case 1:
          prefixChord = 0;
          Mouse.click(MOUSE_RIGHT);
          delay(2);
          Mouse.click(MOUSE_RIGHT);
          return 0;
        case 2:
          prefixChord = 0;
          Mouse.click(MOUSE_RIGHT);
          return 0;
        case 3:
          prefixChord = 0;
          Mouse.click(MOUSE_RIGHT);
          return 0;
        case 4:
          prefixChord = 0;
          Mouse.click(MOUSE_RIGHT);
          return 0;
      }
    case 67:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_RIGHT_ARROW);
          return 0x07;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_RIGHT_ARROW);
          return 0x07;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_RIGHT_ARROW);
          return 0x07;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_RIGHT_ARROW);
          return 0x07;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_RIGHT_ARROW);
          return 0x07;
      }
    case 68:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Mouse.click(MOUSE_MIDDLE);
          return 0;
        case 1:
          prefixChord = 0;
          Mouse.click(MOUSE_MIDDLE);
          delay(2);
          Mouse.click(MOUSE_MIDDLE);
          return 0;
        case 2:
          prefixChord = 0;
          Mouse.click(MOUSE_MIDDLE);
          return 0;
        case 3:
          prefixChord = 0;
          Mouse.click(MOUSE_MIDDLE);
          return 0;
        case 4:
          prefixChord = 0;
          Mouse.click(MOUSE_MIDDLE);
          return 0;
      }
    case 69:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_UP_ARROW);
          return 0x0E;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_UP_ARROW);
          return 0x0E;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_UP_ARROW);
          return 0x0E;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_UP_ARROW);
          return 0x0E;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_UP_ARROW);
          return 0x0E;
      }
    case 70:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_DELETE);
          return 0x04;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_DELETE);
          return 0x04;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_DELETE);
          return 0x04;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_DELETE);
          return 0x04;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_DELETE);
          return 0x04;
      }
    case 71:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_BACKSPACE);
          return 0x08;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_BACKSPACE);
          return 0x08;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_BACKSPACE);
          return 0x08;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_BACKSPACE);
          return 0x08;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_BACKSPACE);
          return 0x08;
      }
    case 72:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Mouse.click(MOUSE_LEFT);
          return 0;
        case 1:
          prefixChord = 0;
          Mouse.click(MOUSE_LEFT);
          delay(2);
          Mouse.click(MOUSE_LEFT);
          return 0;
        case 2:
          prefixChord = 0;
          Mouse.click(MOUSE_LEFT);
          return 0;
        case 3:
          prefixChord = 0;
          Mouse.click(MOUSE_LEFT);
          return 0;
        case 4:
          prefixChord = 0;
          Mouse.click(MOUSE_LEFT);
          return 0;
      }
    case 73:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ARROW);
          return 0x0B;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ARROW);
          return 0x0B;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ARROW);
          return 0x0B;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ARROW);
          return 0x0B;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ARROW);
          return 0x0B;
      }
    case 74:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ALT);
          return 0xE2;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ALT);
          return 0xE2;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ALT);
          return 0xE2;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ALT);
          return 0xE2;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_LEFT_ALT);
          return 0xE2;
      }
    case 75:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_DOWN);
          return 0x06;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_DOWN);
          return 0x06;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_DOWN);
          return 0x06;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_DOWN);
          return 0x06;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_DOWN);
          return 0x06;
      }
    case 76:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_UP);
          return 0x03;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_UP);
          return 0x03;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_UP);
          return 0x03;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_UP);
          return 0x03;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_PAGE_UP);
          return 0x03;
      }
    case 77:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_DOWN_ARROW);
          return 0x0C;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_DOWN_ARROW);
          return 0x0C;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_DOWN_ARROW);
          return 0x0C;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_DOWN_ARROW);
          return 0x0C;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_DOWN_ARROW);
          return 0x0C;
      }
    case 78:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_END);
          return 0x05;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_END);
          return 0x05;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_END);
          return 0x05;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_END);
          return 0x05;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_END);
          return 0x05;
      }
    case 79:
      switch (prefixChord){
        case 0:
          prefixChord = 0;
          Keyboard.write(KEY_HOME);
          return 0x02;
        case 1:
          prefixChord = 0;
          Keyboard.write(KEY_HOME);
          return 0x02;
        case 2:
          prefixChord = 0;
          Keyboard.write(KEY_HOME);
          return 0x02;
        case 3:
          prefixChord = 0;
          Keyboard.write(KEY_HOME);
          return 0x02;
        case 4:
          prefixChord = 0;
          Keyboard.write(KEY_HOME);
          return 0x02;
      }
    
    default:
      prefixChord = 0;
      Keyboard.releaseAll();
      stickyNumlock = LOW;
      stickyCapsLock = LOW;
      stickySpecialLock = LOW;
      return 0;     // error
  }
}
