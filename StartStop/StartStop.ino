byte ACCPin = 13; //Выход на транзистор переферии ACC
byte IGN2Pin = 12; //Выход на транзистор переферии IGN2
byte EnginePin = 11; //Выход на транзистор зажигания 
byte StarterPin = 10; //Выход на транзистор стартера 
byte ButtonPin = 9; //Вход + с кнопки <----------------------------- Стягивающий резистор на "+" 
byte ImmoPin = 8; //Вход с иммо <----------------------------- Подтягивающий резистор на "+" 
byte StopPin = 7; //Вход с тормоза <----------------------------- Стягивающий резистор на "-" 
byte TahoPin = 6; //Вход с тахометра <----------------------------- Стягивающий резистор на "-" 
byte WordsPin = 5; //Выход на глазок 1  
byte YellowGlazokPin = 4; //Выход на глазок 2  
byte GreenGlazokPin = 3; //Выход на глазок 3
byte LockPin = 2; //Выход на транзистор блокировку
byte statusEngine = 0; 
//byte rotate = 0; 
unsigned int rotate=0; 
boolean stateACC = false;
boolean stateIGN2 = false;
unsigned long lastPress = 0; 
unsigned long lastImmo = 0; 
unsigned long splitTime = 0; 
unsigned long time =0; 
byte stat = 0; 
void setup(){ 
pinMode(EnginePin, OUTPUT); 
pinMode(StarterPin, OUTPUT); 
pinMode(ACCPin, OUTPUT);
pinMode(IGN2Pin, OUTPUT);
pinMode(WordsPin, OUTPUT); 
pinMode(YellowGlazokPin, OUTPUT); 
pinMode(GreenGlazokPin, OUTPUT); 
//time = 0; 
lastImmo=millis(); 
lastPress=millis(); 
Serial.begin(9600); 
Serial.println("Arduino is started"); 
} 

// 0 - Все выключенно 
// 1 - Включено только зажигание 
// 2 - Крутиться стартер, ждем оборотов 
// 3 - Двигатель работает, все включено 
// 4 - Неудачная попытка запуска, включено только зажигание 
// 5 - Двигатель заглушен, АСС пока еще работает 
void loop(){ 
if (stat != statusEngine) {stat = statusEngine; Serial.println(stat);} 
unsigned long newTime = millis() - lastPress; 
unsigned int button = pressButton(); 
//if (button != 0) Serial.println(button); 
lastImmoTest(); 
switch (statusEngine) { 
//----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----  
case 0: 
digitalWrite(GreenGlazokPin, LOW); 
if (stateACC) digitalWrite(YellowGlazokPin, HIGH); else digitalWrite(YellowGlazokPin, LOW); 
if (millis() - lastImmo > 10000) digitalWrite(WordsPin, HIGH); else digitalWrite(WordsPin, LOW); 
if (button == 2000 && stateACC) { 
StopAll(); 
} 
else if (button == 1700 && !stateACC) StartACC(); 
else if (button != 0 && millis() - lastImmo > 2000 && newTime > 500) { 
StartEngine(); 
statusEngine=1; 
} 
break; 
//----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----  
case 1: 
if (digitalRead(ImmoPin) == LOW) StopAll(); 
digitalWrite(WordsPin, HIGH);  
if (stateACC) digitalWrite(YellowGlazokPin, HIGH); else digitalWrite(YellowGlazokPin, LOW); 
if (millis() - lastImmo > 2000 && StopIsPress() && newTime < 900)  
{ 
digitalWrite(GreenGlazokPin, HIGH);  
digitalWrite(WordsPin, HIGH); 
} 
 else 
{ 
digitalWrite(GreenGlazokPin,!digitalRead(GreenGlazokPin)); 
delay(100); 
} 
  
if (button == 1500) { 
StopAll(); 
} 
else if (button != 0 && newTime > 500) { 
if (!stateACC)  
StopAll(); 
else 
{StopEngine(); 
 statusEngine=5; 
 } 
 } 
 else if (newTime > 10000 && newTime < 10100 && StopIsPress()) { 
 StartStarter(); 
 statusEngine=2; 
 } 
 break; 
  
  
 //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- 
 case 2: 
 digitalWrite(WordsPin, HIGH);  
 digitalWrite(GreenGlazokPin, HIGH);  
 if (stateACC) digitalWrite(YellowGlazokPin, HIGH); 
 if (digitalRead(TahoPin) == HIGH) { 
 rotate++; 
 while (digitalRead(TahoPin) == HIGH) if (digitalRead(ImmoPin) == LOW) {StopAll(); break;} 
 } 
 if (rotate>=2) { 
 unsigned int rpm = (rotate*60000*10000/2)/(micros()-splitTime); 
 Serial.print("rpm: ");Serial.println(rpm); 
 rotate=0; 
 splitTime=micros(); 
 if (rpm>2500) { //<-------ВЕРНУТЬ СЮДА ТЫЩУ!!!! <------- 
 StopStarter(); 
 StartACC();
 StartIGN2(); 
 statusEngine=3; 
 Serial.println("Engine is WORK!!"); 
 break; 
 } 
 } 
 if (button != 0 && newTime > 500) {  
 StopAll(); 
 } 
 else if (newTime > 5000 || !StopIsPress()){ 
 StopStarter(); 
 statusEngine=4; 
 } 
 break; 
 //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- 
 case 3: 
 digitalWrite(GreenGlazokPin, HIGH); 
 digitalWrite(YellowGlazokPin, LOW); 
 if (newTime > 5000) digitalWrite(WordsPin, LOW); else digitalWrite(WordsPin, HIGH); 
 if (button == 1500) { 
 StopAll(); 
 } 
 else if (button != 0 && newTime > 500) {  
 StopEngine(); 
 statusEngine=5; 
 } 
 break; 
  
 //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- 
  
 case 4: 
  
 digitalWrite(WordsPin, HIGH); 
 digitalWrite(GreenGlazokPin, !digitalRead(GreenGlazokPin)); delay(50); 
 digitalWrite(YellowGlazokPin, LOW); 
 if (button == 1500 || newTime > 10000) { 
 StopAll(); 
 } 
 else if (button != 0 && newTime > 500 && StopIsPress()) { 
 StartStarter(); 
 statusEngine=2; 
 } 
 else if (button != 0 && newTime > 500 && !StopIsPress()) { 
 StopAll(); 
 } 
 break; 
 //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- 
 case 5: 
 digitalWrite(GreenGlazokPin, LOW); 
 if (stateACC) digitalWrite(YellowGlazokPin, HIGH); else digitalWrite(YellowGlazokPin, LOW); 
 if (newTime > 5000)  
 { 
 if (millis() - lastImmo > 5000) digitalWrite(WordsPin, HIGH); else digitalWrite(WordsPin, LOW); 
 } 
 else  
 digitalWrite(WordsPin, HIGH); 
 if (button == 1500) { 
 StopAll(); 
 } 
 else if (millis() - lastImmo < 2000) StopAll(); 
 else if (button != 0 && newTime > 500) { 
 StartEngine(); 
 statusEngine=1; 
 } 
 lastImmoTest(); 
 break; 
 //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- 
 } 
 } 

void StartEngine(){ 
digitalWrite(EnginePin, HIGH); 
Serial.println("Engine is ON"); 
lastPress=millis(); 
 } 
 void StopEngine(){ 
 digitalWrite(EnginePin, LOW);
digitalWrite(IGN2Pin, LOW); 
 Serial.println("Engine is OFF"); 
 lastPress=millis(); 
 } 
 void StopAll(){ 
 digitalWrite(EnginePin, LOW); 
 digitalWrite(StarterPin, LOW); 
 digitalWrite(ACCPin, LOW);
digitalWrite(IGN2Pin, LOW); 
 Serial.println("All OFF");  
 while (!digitalRead(ButtonPin)){} 
 stateACC=false; 
 lastPress=millis(); 
 statusEngine=0; 
 lastPress=millis();  
 } 
  
 void StartStarter(){ 
 digitalWrite(StarterPin, HIGH); 
 Serial.println("Starter START!"); 
 splitTime=micros(); 
 lastPress=millis(); 
 } 
 void StopStarter(){ 
 digitalWrite(StarterPin, LOW); 
 Serial.println("Starter if OFF!"); 
 lastPress=millis(); 
 } 
  
 void StartACC(){ 
 digitalWrite(ACCPin, HIGH); 
 Serial.println("Starter ACC!"); 
 stateACC=true; 
 lastPress=millis(); 
 }

 void StartIGN2(){ 
 digitalWrite(IGN2Pin, HIGH); 
 Serial.println("Starter IGN2!"); 
 stateIGN2=true; 
 lastPress=millis(); 
 } 
 void lastImmoTest(){ 
 if (digitalRead(ImmoPin) == LOW) { //<----------------------------- Поменял на LOW 
 lastImmo=millis(); 
 Serial.println("Immo+"); 
 } 
 } 
  
 unsigned int pressButton(){ 
 unsigned long newTime = millis(); 
 boolean currentPosition = digitalRead(ButtonPin); 
 if (currentPosition == HIGH){  
 delay(50); 
 currentPosition=digitalRead(ButtonPin);  
 } 
 while (!currentPosition) { 
 currentPosition=digitalRead(ButtonPin); 
 if (millis() - newTime > 1000) return 1500; 
 } 
 newTime=millis() - newTime; 
 if (newTime < 60) newTime = 0; 
 return newTime;  
 } 
 boolean StopIsPress(){ 
 return digitalRead(StopPin);  
 }
