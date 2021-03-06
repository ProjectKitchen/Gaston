
#define LED_PIN 9

// pins for pumps
#define P1_PIN  4
#define P2_PIN  5
#define P3_PIN  6
#define P4_PIN  7

// pins for buttons
#define B1_PIN 10
#define B2_PIN 16
#define B3_PIN 14

#define IR_PIN  15
#define IR_TIMEOUT 100
#define IR_THRESHOLD 15
#define STABLE_IR_TIME 4

#define TIME_FACTOR 1000

uint8_t p1_time,p2_time,p3_time,p4_time;
char message[100];

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(P1_PIN, OUTPUT);
  pinMode(P2_PIN, OUTPUT);
  pinMode(P3_PIN, OUTPUT);
  pinMode(P4_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(B1_PIN, INPUT_PULLUP);
  pinMode(B2_PIN, INPUT_PULLUP);
  pinMode(B3_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  delay(3000);
  digitalWrite(LED_PIN,HIGH);
  Serial.println("Hi");

}

void loop() {
  uint32_t actcode = get_IR_code(&p1_time, &p2_time, &p3_time, &p4_time);
  sprintf(message,"Received pump timings: %d, %d, %d, %d",p1_time,p2_time,p3_time,p4_time);  
  Serial.println(message);
  dispense();
}

uint8_t debouncedRead(uint8_t pin) {
  uint8_t s,i=0;
  s=digitalRead(pin);
  while (i<20) {
    if (s != digitalRead(pin)) {
      i=0; s=digitalRead(pin);
    } else i++;
    delayMicroseconds(100);
  }  
  return(s);
}

uint32_t get_IR_code(uint8_t * p1, uint8_t * p2, uint8_t * p3, uint8_t * p4) {
  uint32_t timestamp, acttime;
  uint32_t code;
  uint8_t actpos,b1,b2,b3;
  uint8_t s,actstate,code_valid;

  do {
    Serial.println("Waiting for IR-Code");  
    while (debouncedRead(IR_PIN)==HIGH)   // wait for IR-Signal
    {
      if (digitalRead(B1_PIN) == LOW) 
      {  digitalWrite(P1_PIN,HIGH); while (digitalRead(B1_PIN) == LOW); digitalWrite(P1_PIN,LOW);  }
      if (digitalRead(B2_PIN) == LOW) 
      {  digitalWrite(P2_PIN,HIGH); while (digitalRead(B2_PIN) == LOW); digitalWrite(P2_PIN,LOW);  }
      if (digitalRead(B3_PIN) == LOW) 
      {  digitalWrite(P3_PIN,HIGH); while (digitalRead(B3_PIN) == LOW); digitalWrite(P3_PIN,LOW);  }
    }
   
    timestamp=millis();
    actstate=LOW;
    code=0;
    code_valid=0;
    actpos=0;
/*
    while ((millis()-timestamp) < IR_TIMEOUT) {
      s=digitalRead(IR_PIN);
      if (s!=actstate) {
         acttime=millis();
         if (acttime-timestamp >= STABLE_IR_TIME) {
           actstate=s;
           // Serial.println(acttime-timestamp);
           if ((acttime-timestamp) > IR_THRESHOLD) 
              code|=((uint32_t)1<<actpos);
           actpos++;
           digitalWrite(LED_PIN,actstate);
           timestamp=millis();
         }
      }
    }
*/
    uint8_t newstate_stable=0;
    while ((millis()-timestamp) < IR_TIMEOUT) {

      if (digitalRead(IR_PIN) != actstate) newstate_stable++;
      else newstate_stable=0;
      delayMicroseconds(10); 

      if (newstate_stable > 20) {
        acttime=millis();
        // Serial.println(acttime-timestamp);
        if ((acttime-timestamp) > IR_THRESHOLD) 
           code|=((uint32_t)1<<actpos);
        actpos++;
        digitalWrite(LED_PIN,actstate);
        if (actstate == HIGH) actstate=LOW; else actstate=HIGH;
        newstate_stable=0;
        timestamp=millis();
      }
    }

    
    digitalWrite(LED_PIN,LOW);
    if (actpos > 2) { 
      Serial.print("checking code 0x");
      Serial.print(code,HEX);
      if ((code >> 24) == 0xA5) {
        b1= (code >> 16 ) & 0xff;
        b2= (code >> 8 ) & 0xff;
        b3=  code & 0xff;
        if ((b1 ^ b2) == b3) {
          Serial.println("--> valid!");
          code_valid=1;
        }
      }
    }

    if (!code_valid) {
         Serial.println("--> invalid code ... retry in 1500 ms");
         delay (1500);
    }
  } while (!code_valid); 

  *p1 = (code >> 8) & 0x0f;
  *p2 = (code >> 12) & 0x0f;
  *p3 = (code >> 16) & 0x0f;
  *p4 = (code >> 20) & 0x0f;

  return(code);
}


void dispense()
{
  Serial.println("Start dispensing ...");
  if (p1_time) {   
    Serial.print("switching P1 on for "); Serial.print(p1_time * TIME_FACTOR); Serial.println(" milliseconds.");
    digitalWrite(P1_PIN, HIGH); delay ((uint16_t)p1_time * TIME_FACTOR); digitalWrite(P1_PIN, LOW);
  }
  if (p2_time) {   
    Serial.print("switching P2 on for "); Serial.print(p2_time * TIME_FACTOR); Serial.println(" milliseconds.");
    digitalWrite(P2_PIN, HIGH); delay ((uint16_t)p2_time * TIME_FACTOR); digitalWrite(P2_PIN, LOW);
  }
  if (p3_time) {   
    Serial.print("switching P3 on for "); Serial.print(p3_time * TIME_FACTOR); Serial.println(" milliseconds.");
    digitalWrite(P3_PIN, HIGH); delay ((uint16_t)p3_time * TIME_FACTOR); digitalWrite(P3_PIN, LOW);
  }
  if (p4_time) {   
    Serial.print("switching P4 on for "); Serial.print(p4_time * TIME_FACTOR); Serial.println(" milliseconds.");
    digitalWrite(P4_PIN, HIGH); delay ((uint16_t)p4_time * TIME_FACTOR); digitalWrite(P4_PIN, LOW);
  }
  Serial.println("Dispensing done.");
}

  
