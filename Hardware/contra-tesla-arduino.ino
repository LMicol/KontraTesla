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
L298N motorESQ(ENA,IN1,IN2);
L298N motorDIR(ENB,IN3,IN4);

// Portas para os servos
// Utilizadas portas analogicas como digitais
#define servo_HOR A0
#define servo_VER A1

// Instância servos horizontal e vertical, respectivamente
Servo servoHOR;
Servo servoVER;

// Variáveis para a MPU
const int MPU=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

// Para controle da serial
#define SPEED 115200
#define MAX_LEN 80
char inputSentence[MAX_LEN + 1];
const char lineEnding = '\n';
int inputIndex;
bool newInput = false;

// Para tratar os dados separados por virgula recebidos na serial
const byte MAX_TOKENS = 3;
const char* delimiter = ",";
char* tokens[MAX_TOKENS + 1];
enum indexName {id,value1,value2};

// Variáveis para os dados dos sensores infrared
int IR_ESQState = 1;
int IR_DIRState = 1;
int IR_TRAState = 1;

// Para o sensor ultrasonico
Ultrasonic ultrasonic(ULTRA_TRIG,ULTRA_ECHO);
int distance;

// Variável para a temperatura do sensor de temperatura
float tempo = 0;

// Intervalo para imprimir na serial a temperatura
long previousMillis = 0;
long interval = 1000;

void setup()
{
    stopMotors();

    pinMode(servo_HOR,OUTPUT);
    pinMode(servo_VER,OUTPUT);

    servoHOR.attach(servo_HOR);
    servoVER.attach(servo_VER);

    pinMode(IR_ESQ,INPUT);
    pinMode(IR_DIR,INPUT);
    pinMode(IR_TRA,INPUT);
    pinMode(TEMP,  INPUT);
    pinMode(ULTRA_TRIG,OUTPUT);
    pinMode(ULTRA_ECHO,INPUT);

    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    Serial.begin(SPEED);
}

// Coleta dados dos sensores de colisão
void ir()
{
    // Lê os sensores
    IR_ESQState = digitalRead(IR_ESQ);
    IR_DIRState = digitalRead(IR_DIR);
    IR_TRAState = digitalRead(IR_TRA);

    // Inverte a saída do sensor traseiro, seu funcionamento é invertido em relação aos demais
    if(IR_TRAState)
    {
        IR_TRAState = 0;
    }
    else
    {
        IR_TRAState = 1;
    }
}

// Imprime na serial os dados dos sensores infrared
// Saída: i,esquerdo,direito,traseiro
void ir_print()
{
    // Chama a função que lê os sensores infrared
    ir();

    // Imprime na serial os dados
    Serial.print("i,"); Serial.print(IR_ESQState);
    Serial.print(",");  Serial.print(IR_DIRState);
    Serial.print(",");  Serial.println(IR_TRAState);
}

// Coleta e imprime na serial a temperatura, em graus celsius, do sensor de temperatura
// Saída: t,temperatura
void temperatura()
{
    // Lê o sensor
    tempo = digitalRead(TEMP);

    // Imprime na serial
    Serial.print("t,"); Serial.println(tempo);
}

// Coleta e imprime na serial a distancia do sensor ultrasonico
// Saída: u,distancia
void ultra()
{
    // Lê o sensor
    distance = ultrasonic.read();

    // Imprime na serial
    Serial.print("u,"); Serial.println(distance);
}

// Retorna os dados do acelerometro e do giroscopio no formato:
// Saída: a,x_ace,y_ace,z_ace,x_giro,y_giro,z_giro
// Obs: não é utilizada a temperatura desse sensor
void mpu6050()
{
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU,14,true);
    AcX=Wire.read()<<8|Wire.read();
    AcY=Wire.read()<<8|Wire.read();
    AcZ=Wire.read()<<8|Wire.read();
    Tmp=Wire.read()<<8|Wire.read();
    GyX=Wire.read()<<8|Wire.read();
    GyY=Wire.read()<<8|Wire.read();
    GyZ=Wire.read()<<8|Wire.read();

    // Imprime na serial
    Serial.print("a,");Serial.print(AcX);
    Serial.print(","); Serial.print(AcY);
    Serial.print(","); Serial.print(AcZ);
    Serial.print(","); Serial.print(GyX);
    Serial.print(","); Serial.print(GyY);
    Serial.print(","); Serial.println(GyZ);

    // Descomentar para imprimir a temperatura do sensor embutido na mpu
    //Serial.print("t,"); Serial.println(Tmp/340.00+36.53);
}

// Para os motores
void stopMotors()
{
    // Para os motores do lado ESQUERDO e DIREITO, respectivamente
    motorESQ.stop();
    motorDIR.stop();
}

void loop()
{
    // Obtem os dados dos sensores de colisao SEM imprimir na serial
    ir();

    // Se colisao for detectada, para os motores e imprime na serial os dados dos sensores de colisão.
    if(IR_ESQState == 0 || IR_DIRState == 0 || IR_TRAState == 0)
    {
        // Para os motores
        stopMotors();
        ir_print();
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
        // Quebra a entrada usando o delimiter
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
        if(*tokens[id] == 'm')
        {
            // Tarefa: transformar em vetor de motores, pra nao repetir codigo e preparar para a situação de quatro motores independetes utilizando duas pontes h.
            //stopMotors();

            // Se parameto < 128 motores do lado esquerdo rodam no sentido antihorario
            // Senão, rodam no sentido horario
            if(val1 < 128)
            {
                // Mapeia o parametro para 0~255
                int val11map = map(val1,   0, 127, 0, 255);

                // Configura a velocidade
                motorESQ.setSpeed(val11map);

                // Move para frente
                motorESQ.backward();
            }
            else
            {
                // Mapeia o parametro para 0~255
                int val12map = map(val1, 128, 255, 0, 255);

                // Configura a velocidade
                motorESQ.setSpeed(val12map);

                // Move para trás
                motorESQ.forward();
            }

            // Igual ao if-else anterior mas, para os motores do lado direito
            if(val2 < 128)
            {
                int val21map = map(val2,   0, 127, 0, 255);
                motorDIR.setSpeed(val21map);
                motorDIR.backward();
            }
            else
            {
                int val22map = map(val2, 128, 255, 0, 255);
                motorDIR.setSpeed(val22map);
                motorDIR.forward();
            }
        }
        else if(*tokens[id] == 'v')
        {
            // Movimenta os servos para as posições informadas
            servoHOR.write(val1);
            servoVER.write(val2);
        }
        else if(*tokens[id] == 'a')
        {
            // Retorna os dados do acelerometro e giroscopio
            mpu6050();
        }
        else if(*tokens[id] == 's')
        {
            // Pára os motores
            stopMotors();
        }
        else if(*tokens[id] == 'i')
        {
            // Retorna os dados dos sensores de colisão
            ir_print();
        }
        else if(*tokens[id] == 'u')
        {
            // Retorna a distância obtida do sensor ultrasônico
            ultra();
        }
        else if(*tokens[id] == 't')
        {
            // Retorna a temperatura
            temperatura();
        }

        // limpa string
        inputSentence[0] = '\0';
        newInput = false;
        inputIndex = 0;
    }
}

void serialEvent()
{
    // coleta bytes de entrada enquanto estiver chegando
    while(Serial.available())
    {
        // obtem proximo byte:
        char inChar = (char)Serial.read();

        // se caracter for nova linha seta flag, entao loop principal pode tratar a instrução recebida
        if(inChar == lineEnding)
        {
            newInput = true;
        }
        else
        {
            if(inputIndex < MAX_LEN)
            {
                inputSentence[inputIndex++] = inChar;
                inputSentence[inputIndex] = '\0';
            }
        }
    }
}
