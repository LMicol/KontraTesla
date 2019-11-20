// Contra tesla
// 
// Código para o arduino UNO R3 
// 
// Recebe comandos, manipula os motores e fornece dados de sensores via serial.
// 
// 
// m,vel_esq,vel_dir       : seta velocidade dos motores do lado esquerdo e do lado direito, respectivamente:
//                             sentido horário
//                                 0~127 -> faixa
//                                 0     -> min
//                                 127   -> max
//                             sentido antihorario
//                                 128~255 -> faixa
//                                 128     -> min
//                                 255     -> max
// v,ang_hor,ang_ver       : seta angulo de rotacao do servo motor horizontal e vertical, respectivamente:
//                             horizontal
//                                 0~180 -> faixa
//                                 0     -> direita
//                                 180   -> esquerda
//                             vertical
//                                 0~180 -> faixa
//                                 0     -> cima
//                                 180   -> baixo
// 
// a                       : retorna dados do acelerometro e giroscopio:
// a,x_ace,y_ace,z_ace,x_giro,y_giro,z_giro
//                             onde:
//                             x_ace  -> 
//                             y_ace  -> 
//                             z_ace  -> 
//                             x_giro -> 
//                             y_giro -> 
//                             z_giro ->
// 
// i                       : retorna os dados dos IR na ordem ESQ,DIR,TRA
// i,esquerdo,direito,traseiro
//                             1 -> sem colisão
//                             0 -> com colisão
// 
// u                       : retorna distancia do ultrasonico
// u,distancia
// t                       : retorna a temperatura
// t,temperatura
// s                       : para os motores
// 
// Esquema de ligação:
// Ligação detalhada em contra-tesla.fzz
// 
//  0  não usada
//  1  não usada
//  2  ponte h en1
//  3  ponte h ena (pwm)
//  4  ponte h en2
//  5  ponte h enb (pwm)
//  6  sensor de temperatura
//  7  ponte h en3
//  8  ponte h en4
//  9  sensor colisão traseiro
// 10  sensor colisão esquerdo
// 11  sensor ultrasônico echo
// 12  sensor colisão direito
// 13  sensor ultrasônico trig
// 
// A0  servo motor horizontal
// A1  servo motor vertical
// A2  não usada
// A3  não usada
// A4  mpu6050 sda
// A5  mpu6050 scl
// 
// Caracteristicas:
//  - Carro para quando sensores IR detectarem colisão;
//  - Dados da MPU são enviados para a serial constantemente;
//  - Temperatura é enviada regularmente.
// 
// Material utilizado:
//  - 1 Arduino UNO R3
//  - 1 Ponte H
//  - 1 Módulo HCSR04
//  - 4 baterias 18650
//  - 2 suportes para baterias 18650
//  - 2 Servo motores
//  - 3 IR
//  - 1 temp
//  - 


// Bibliotecas utilizadas
#include <L298N.h>
#include <Wire.h>
#include <Servo.h>
#include <Ultrasonic.h>

// Portas sensores infrared de colisão
#define IR_ESQ 10
#define IR_DIR 12
#define IR_TRA 9

// Desabilita a parada do carro quando sensores IR detectarem colisão
// 0 ativada
// 1 desativada
// Ativada por default
const int DESABILITA_PARADA_IR_COLISAO = 0;

// Desabilita a parada do carro quando distancia do ultrasonico for menor que
// Desativada por default
// 0 ativada
// 1 desativada
const int DESABILITA_PARADA_ULTRASONICO_COLISAO = 1;
const int DISTANCIA_PARADA_ULTRASONICO_COLISAO  = 4;

// Porta sensor de temperatura
#define TEMP 6

// Portas para o sensor ultrasônico
#define ULTRA_TRIG 13
#define ULTRA_ECHO 11

// Portas par de motores lado ESQUERDO
// ENA deve ser uma porta PWM
#define ENA 3
#define IN1 2
#define IN2 4

// Portas par de motores lado DIREITO
// ENB deve ser uma porta PWM
#define ENB 5
#define IN3 7
#define IN4 8

// Instância os dois pares de motores
L298N motorEsq(ENA,IN1,IN2);
L298N motorDir(ENB,IN3,IN4);

#define VEL_MIN_HORA 0
#define VEL_MAX_HORA 127
#define VEL_MIN_ANTI 128
#define VEL_MAX_ANTI 255
#define MAP_MIN 0
#define MAP_MAX 255

// Portas para os servos
// Utilizadas portas analogicas como digitais
#define SERVO_HOR A0
#define SERVO_VER A1

// Instância servos horizontal e vertical, respectivamente
Servo servoHor;
Servo servoVer;

// Variáveis para a MPU6050
// Endereco I2C do MPU6050
const int mpui2cAddress=0x68;
int16_t acelerometroX,acelerometroY,acelerometroZ;
int16_t giroscopioX,giroscopioY,giroscopioZ;
int16_t temperatureMpu;

// Para controle da serial
#define SPEED 115200
#define MAX_LEN 80
char inputSentence[MAX_LEN + 1];
const char lineEnding = '\n';
const char stringEnding = '\0';
int inputIndex = 0;
bool newInput = false;

// Para tratar os dados separados por virgula na serial
const byte MAX_TOKENS = 3;
const char* delimiter = ",";
char* tokens[MAX_TOKENS + 1];
enum indexName {id,value1,value2};

const char idTemperature = 't';
const char idIr = 'i';
const char idUltrasonic = 'u';
const char idMotors = 'm';
const char idServo = 'v';
const char idMpu = 'a';
const char isStop = 's';

// Variáveis para os dados dos sensores infrared
int ir_esqState = 1;
int ir_dirState = 1;
int ir_traState = 1;

// Para o sensor ultrasonico
Ultrasonic ultrasonic(ULTRA_TRIG,ULTRA_ECHO);
int distance;

// Variável para a temperatura do sensor de temperatura
float temperatureSensor = 0;

// Intervalo para imprimir na serial a temperatura
long previousMillis = 0;
long interval = 1000;

void setup()
{
    stopMotors();

    pinMode(SERVO_HOR,OUTPUT);
    pinMode(SERVO_VER,OUTPUT);

    servoHor.attach(SERVO_HOR);
    servoVer.attach(SERVO_VER);

    pinMode(IR_ESQ,INPUT);
    pinMode(IR_DIR,INPUT);
    pinMode(IR_TRA,INPUT);
    pinMode(TEMP,  INPUT);
    pinMode(ULTRA_TRIG,OUTPUT);
    pinMode(ULTRA_ECHO,INPUT);

    Wire.begin();
    Wire.beginTransmission(mpui2cAddress);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    Serial.begin(SPEED);
}

// Coleta dados dos sensores de colisão
void ir()
{
    // Lê os sensores
    ir_esqState = digitalRead(IR_ESQ);
    ir_dirState = digitalRead(IR_DIR);
    ir_traState = digitalRead(IR_TRA);

    // Inverte a saída do sensor traseiro, seu funcionamento é invertido em relação aos demais
    if(ir_traState == 1)
    {
        ir_traState = 0;
    }
    else if(ir_traState == 0)
    {
        ir_traState = 1;
    }
}

// Imprime na serial os dados dos sensores infrared
// Saída: idIr,esquerdo,direito,traseiro
void ir_print()
{
    // Lê os sensores
    ir();

    // Imprime na serial
    Serial.print(idIr);
    Serial.print(delimiter);
    Serial.print(ir_esqState);
    Serial.print(delimiter);
    Serial.print(ir_dirState);
    Serial.print(delimiter);
    Serial.println(ir_traState);
}

// Coleta e imprime na serial a temperatura, em graus celsius, do sensor de temperatura
// Saída: idTemperature,temperatura
void temperatura()
{
    // Lê o sensor
    temperatureSensor = digitalRead(TEMP);

    // Imprime na serial
    Serial.print(idTemperature);
    Serial.print(delimiter);
    Serial.println(temperatureSensor);
}

// Coleta a distancia do sensor ultrasonico
void ultra()
{
    // Lê o sensor
    distance = ultrasonic.read();
}

// Imprime na serial a distancia do sensor ultrasonico
// Saída: idUltrasonic,distancia
void ultra_print()
{
    // Lê o sensor
    ultra();

    // Imprime na serial
    Serial.print(idUltrasonic);
    Serial.print(delimiter);
    Serial.println(distance);
}

// Retorna os dados do acelerometro e do giroscopio no formato:
// Saída: idMpu,x_ace,y_ace,z_ace,x_giro,y_giro,z_giro
// Obs: não é utilizada a temperatura desse sensor
void mpu6050()
{
    Wire.beginTransmission(mpui2cAddress);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(mpui2cAddress,14,true);
    acelerometroX=Wire.read()<<8|Wire.read();
    acelerometroY=Wire.read()<<8|Wire.read();
    acelerometroZ=Wire.read()<<8|Wire.read();
    temperatureMpu=Wire.read()<<8|Wire.read();
    giroscopioX=Wire.read()<<8|Wire.read();
    giroscopioY=Wire.read()<<8|Wire.read();
    giroscopioZ=Wire.read()<<8|Wire.read();

    // Imprime na serial
    Serial.print(idMpu);
    Serial.print(delimiter);
    Serial.print(acelerometroX);
    Serial.print(delimiter);
    Serial.print(acelerometroY);
    Serial.print(delimiter);
    Serial.print(acelerometroZ);
    Serial.print(delimiter);
    Serial.print(giroscopioX);
    Serial.print(delimiter);
    Serial.print(giroscopioY);
    Serial.print(delimiter);
    Serial.println(giroscopioZ);

    // Imprime a temperatura do sensor embutido na mpu6050
    //Serial.print(idTemperature);
    //Serial.print(delimiter);
    //Serial.println(temperatureMpu/340.00+36.53);
}

// Para os motores do lado ESQUERDO e DIREITO, respectivamente
void stopMotors()
{
    motorEsq.stop();
    motorDir.stop();
}

void loop()
{
    // Obtem os dados dos sensores de colisao SEM imprimir na serial
    ir();

    // Se colisao for detectada, para os motores e imprime na serial os dados dos sensores.
    if((ir_esqState == 0 || ir_dirState == 0 || ir_traState == 0) && (DESABILITA_PARADA_IR_COLISAO == 0))
    {
        stopMotors();
        ir_print();
    }

    // Obtem distancia com o ultrasônico SEM imprimir na serial
    ultra();

    // Se perto demais, para os motores e imprime na serial.
    if((distance < DISTANCIA_PARADA_ULTRASONICO_COLISAO) && (DESABILITA_PARADA_ULTRASONICO_COLISAO == 0))
    {
        stopMotors();
        ultra_print();
    }

    // Imprime na serial os dados do acelerometro e do giroscopio, freneticamente :)
    mpu6050();

    unsigned long currentMillis = millis();

    // Imprime na serial a temperatura a cada 'interval'
    if(currentMillis - previousMillis > interval)
    {
        previousMillis = currentMillis;

        temperatura();
    }

    // Trata os comandos recebidos
    if(newInput)
    {
        // Quebra a entrada usando delimiter
        int tokenIndex = 0;
        tokens[tokenIndex] = strtok(inputSentence,delimiter);

        while((tokenIndex < MAX_TOKENS - 1) && tokens[tokenIndex])
        {
            tokenIndex++;
            tokens[tokenIndex] = strtok(NULL,delimiter);
        }

        // Converte os parametros para int
        int val1 = atoi(tokens[value1]);
        int val2 = atoi(tokens[value2]);

        // Trata os motores
        if(*tokens[id] == idMotors)
        {
            // Tarefa: nao repetir codigo, preparar para motores independetes utilizando mais pontes h.
            stopMotors();

            // Motores esquerdos rodam no sentido antihorario
            // Senão, no sentido horario
            if(val1 <= VEL_MAX_HORA)
            {
                // Mapeamento para [MAP_MIN,MAP_MAX]
                int val11map = map(val1,VEL_MIN_HORA,VEL_MAX_HORA,MAP_MIN,MAP_MAX);

                // Configura a velocidade
                motorEsq.setSpeed(val11map);

                // Move para frente
                motorEsq.backward();
            }
            else
            {
                // Mapeamento para [MAP_MIN,MAP_MAX]
                int val12map = map(val1,VEL_MIN_ANTI,VEL_MAX_ANTI,MAP_MIN,MAP_MAX);

                // Configura a velocidade
                motorEsq.setSpeed(val12map);

                // Move para trás
                motorEsq.forward();
            }

            // Idem mas, para os motores do lado direito
            if(val2 <= VEL_MAX_HORA)
            {
                int val21map = map(val2,VEL_MIN_HORA,VEL_MAX_HORA,MAP_MIN,MAP_MAX);
                motorDir.setSpeed(val21map);
                motorDir.backward();
            }
            else
            {
                int val22map = map(val2,VEL_MIN_ANTI,VEL_MAX_ANTI,MAP_MIN,MAP_MAX);
                motorDir.setSpeed(val22map);
                motorDir.forward();
            }
        }
        else if(*tokens[id] == idServo)
        {
            // Movimenta os servos para as posições informadas
            servoHor.write(val1);
            servoVer.write(val2);
        }
        else if(*tokens[id] == idMpu)
        {
            // Retorna os dados do acelerometro e giroscopio
            mpu6050();
        }
        else if(*tokens[id] == isStop)
        {
            // Para os motores
            stopMotors();
        }
        else if(*tokens[id] == idIr)
        {
            // Retorna os dados dos sensores de colisão
            ir_print();
        }
        else if(*tokens[id] == idUltrasonic)
        {
            // Retorna a distância obtida do sensor ultrasônico
            ultra_print();
        }
        else if(*tokens[id] == idTemperature)
        {
            // Retorna a temperatura
            temperatura();
        }

        // Limpa string
        inputSentence[0] = stringEnding;
        newInput = false;
        inputIndex = 0;
    }
}

void serialEvent()
{
    // Coleta bytes de entrada enquanto estiver chegando
    while(Serial.available())
    {
        // Obtem proximo byte:
        char inChar = (char)Serial.read();

        // Se for nova linha, seta flag. Então loop principal trata o comando recebido
        if(inChar == lineEnding)
        {
            newInput = true;
        }
        else
        {
            if(inputIndex < MAX_LEN)
            {
                inputSentence[inputIndex++] = inChar;
                inputSentence[inputIndex] = stringEnding;
            }
        }
    }
}
