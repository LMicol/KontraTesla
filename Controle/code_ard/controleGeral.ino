// Measure Angle with a MPU-6050(GY-521) Written by Ahmet Burkay KIRNIK

/* Volante */

#include<Wire.h>

const int MPU_addr=0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

/**/

bool controller = false;          // true = volante; false = joystick

/* Joystick */

// Variaveis do Joystick
int outputValueX = 0;
int outputValueY = 0;
int buttonState = 0;

const int analogInPinX = 1;       // A1
const int analogInPinY = 2;       // A2
const int digitalJoystick = 2;    // D2

/**/

void setup() {
  /* Joystick */
  pinMode(buttonState, INPUT);
  /**/
  /* Volante */
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
}

void loop() {
  // Lê o estado do botão e muda o dispositivo controlador, se necessário
  buttonState = digitalRead(digitalJoystick);
  if (!buttonState) {
    controller = !controller;
    while (!digitalRead(digitalJoystick)) {} // enquanto não soltar o botão, trava a leitura, pois a troca não foi efetuada
    delay(50);
  }
  // Usa o volante como dispositivo controlador
  if (controller) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);
    AcX=Wire.read()<<8|Wire.read();
    AcY=Wire.read()<<8|Wire.read();
    AcZ=Wire.read()<<8|Wire.read();
    int xAng = map(AcX,minVal,maxVal,-90,90);
    int yAng = map(AcY,minVal,maxVal,-90,90);
    int zAng = map(AcZ,minVal,maxVal,-90,90);

    x = RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
    y = RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
    z = RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

    // [0,360] -> [-180,180]
    if (y > 180) {
      y = -360 + y;
    }
    if (x > 180) {
      x = -360 + x;
    }
    if (z > 180) {
      z = -360 + z;
    }

/*
    if (y >= 45 && y <= 135) {
      Serial.print(z);
      Serial.print(",");
      Serial.println(y);
    } else {
      Serial.print(x);
      Serial.print(",");
      Serial.println(y);
    }
*/

    // caso algum dos ângulos de controle esteja fora do intervalo definido para mapeamento, os valores são zerados
    if (y < 0 || y > 135) {
      // ignora a saída e para o carro
      x = 0;
      y = 90;
      z = 0;
    } else if (y >= 45 || y <= 135) {
      if (z < -60 || z > 60) {
        x = 0;
        y = 90;
        z = 0;
      }
    } else { // (y >= 0 && y < 45)
      if (x < -60 || x > 60) {
        x = 0;
        y = 90;
        z = 0;
      }
    }
/*
    if (y >= 45 && y <= 135) {
      Serial.print(z);
      Serial.print(",");
      Serial.println(y);
    } else {
      Serial.print(x);
      Serial.print(",");
      Serial.println(y);
    }
*/

    // x: [-60, 60] -> [-128, 127]; y: [0, 180] -> [127, -128]
    int xc;
    if (y >= 45 && y <= 135) {
      xc = map(z, -60, 60, -128, 127);
    } else {
      xc = map(x, -60, 60, -128, 127);
    }
    int yc = map(y, 0, 180, 127, -128);

    if(xc < 4 && xc > -4){
      xc = 0;
    }
    if(yc < 4 && yc > -4){
      yc = 0;
    }

    Serial.print(xc);
    Serial.print(",");
    Serial.println(yc);
  }
  // Usa o joystick como dispositivo controlador
  else {
    outputValueX = analogRead(analogInPinX);
    outputValueY = analogRead(analogInPinY);

    int xj = (outputValueX - 512) / 4;
    int yj = (outputValueY - 512) / 4;

    // eliminar o erro existente nos valores mostrado pelo joystick
    if(xj < 4 && xj > -4){
      xj = 0;
    }
    if(yj < 4 && yj > -4){
      yj = 0;
    }

    Serial.print(xj);
    Serial.print(",");
    Serial.println(yj);
  }
}
