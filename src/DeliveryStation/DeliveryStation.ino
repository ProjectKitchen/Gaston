
#define LED_PIN 9
#define P1_PIN  4
#define P2_PIN  5
#define P3_PIN  6
#define P4_PIN  7

#define IR_PIN  15
#define IR_TIMEOUT 100
#define IR_THRESHOLD 15
#define STABLE_IR_TIME 4


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(P1_PIN, OUTPUT);
  pinMode(P2_PIN, OUTPUT);
  pinMode(P3_PIN, OUTPUT);
  pinMode(P4_PIN, OUTPUT);
  digitalWrite(LED_PIN,HIGH);
  
  pinMode(IR_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  delay(3000);
  Serial.println("Hi");

}

uint8_t p1_time,p2_time,p3_time,p4_time;
char message[100];

void loop() {
  Serial.println("Waiting for IR-Code");  
  uint32_t actcode = get_IR_code(&p1_time, &p2_time, &p3_time, &p4_time);
  sprintf(message,"Received pump timings: %d, %d, %d, %d",p1_time,p2_time,p3_time,p4_time);  
  Serial.println(message);
}

uint8_t debouncedRead(uint8_t pin) {
  uint8_t s,i=0;
  s=digitalRead(pin);
  while (i<40) {
    if (s != digitalRead(pin)) {
      i=0; s=digitalRead(pin);
    } else i++;
  }  
  return(s);
}

uint32_t get_IR_code(uint8_t * p1, uint8_t * p2, uint8_t * p3, uint8_t * p4) {
  uint32_t timestamp, acttime;
  uint32_t code;
  uint8_t actpos,b1,b2,b3;
  uint8_t s,actstate,code_valid;

  do {

    while (debouncedRead(IR_PIN)==HIGH);   // wait for IR-Signal
    timestamp=millis();
    actstate=LOW;
    code=0;
    code_valid=0;
    actpos=0;

    while ((millis()-timestamp) < IR_TIMEOUT) {
      s=digitalRead(IR_PIN);
      if (s!=actstate) {
         acttime=millis();
         if (acttime-timestamp >= STABLE_IR_TIME) {
           actstate=s;
           Serial.println(acttime-timestamp);
           if ((acttime-timestamp) > IR_THRESHOLD) 
              code|=((uint32_t)1<<actpos);
           actpos++;
           digitalWrite(LED_PIN,actstate);
           timestamp=millis();
         }
      }
    }
    digitalWrite(LED_PIN,HIGH);

    Serial.print("checking code 0x");
    Serial.println(code,HEX);
    if ((code >> 24) == 0xA5) {
      b1= (code >> 16 ) & 0xff;
      b2= (code >> 8 ) & 0xff;
      b3=  code & 0xff;
      if ((b1 ^ b2) == b3) code_valid=1;
    }

  } while (!code_valid); 

  *p1 = (code >> 8) & 0x0f;
  *p2 = (code >> 12) & 0x0f;
  *p3 = (code >> 16) & 0x0f;
  *p4 = (code >> 20) & 0x0f;

  return(code);
}


