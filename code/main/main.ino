#include <ModbusRtu.h>
#include <DHT_U.h>
#include <DHT.h>
#include <Arduino.h>

#define MAX_ADC_READING 1023
#define ADC_REF_VOLTAGE 5.0
#define REF_RESISTANCE 10000
#define LUX_CALC_SCALAR 125235178.3654270
#define LUX_CALC_EXPONENT -1.604568157
#define ID 1

Modbus slave(ID, 0, 0);
DHT dht(8, DHT11);
int8_t state = 0;
uint16_t data[5];
const int led_pin = 7;
const int motor_pin = 9;
const int heat_pin = 10;
const int ldr_pin = A0;
//const int pump_pin = 11;
int ldrRawData;
float resistorVoltage, ldrVoltage;
float ldrResistance;
float ldrLux;
unsigned long ms_from_start = 0;
unsigned long ms_option1 = 0;
unsigned long option1_interval = 100;
unsigned long ms_option2 = 0;
unsigned long option2_interval = 200;

void setup(){
    //Serial.begin(9600);
    slave.begin(9600);
    pinMode(motor_pin, OUTPUT);
    pinMode(heat_pin, OUTPUT);
    pinMode(led_pin, OUTPUT);
    //pinMode(pump_pin, OUTPUT);
    dht.begin();
}

void loop(){
    ms_from_start = millis();
    state = slave.poll(data, 5);
    option1();
    option2();
}

void option1(){
    if (ms_from_start - ms_option1 > option1_interval){
        ms_option1 = ms_from_start;
        float suhu = dht.readTemperature();
        data[1] = suhu;
        analogWrite(motor_pin, data[3]);
        analogWrite(heat_pin, data[4]);
    }
}

void option2(){
    if (ms_from_start - ms_option2 > option2_interval){
        ms_option2 = ms_from_start;
        ldrRawData = analogRead(ldr_pin);
        resistorVoltage = (float)ldrRawData / MAX_ADC_READING * ADC_REF_VOLTAGE;
        ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;
        ldrResistance = ldrVoltage / resistorVoltage * REF_RESISTANCE;
        ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
        data[2] = ldrLux;
        if (ldrLux <= 70){
            digitalWrite(led_pin, HIGH);
            bitWrite(data[0], 0, 1);
        }
        else{
            digitalWrite(led_pin, LOW);
            bitWrite(data[0], 0, 0);
        }
    }
}
