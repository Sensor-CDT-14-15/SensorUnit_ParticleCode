double temperature, voltage, light, pir2, noise, noisemax;
char publishString[40];
int pir;

void setup()
{
  pinMode(A7, INPUT);
  pinMode(A6, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  Serial.begin(9600);
 // attachInterrupt(A4, pirPublish, CHANGE);
}

void loop()
{
pirNoise();
Publish();
}


void publish() {
  voltage = analogRead(A7);
  voltage = (voltage * 3.3) / 4095;
  temperature = (2103 - voltage*1000)/10.9;
  Serial.println(temperature);
  light = analogRead(A6);
  sprintf(publishString,"%.1f, %.1f, %d, %.1f",temperature, light, pir, noisemax);
  Spark.publish("TL",publishString);
}


void pirNoise() {
  pir=0;
  noise =0;
  noisemax =0;
  int i =0;
   for (i=0; i < 500; i++){
      //for(i; i++; i < 30 ) {
      pir2 = analogRead(A4);
      noise = analogRead(A5);
      Serial.println(pir2);
      Serial.println(noise);
      if(pir2 > 3000) pir=1;
      if(noise > noisemax) noisemax = noise;
      delay(10);
      }
}
