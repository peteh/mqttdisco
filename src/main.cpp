#include <Arduino.h>
#include <MqttDevice.h>


MqttDevice mqttDevice("desk-12343", "Smart Desk", "Smart Desk Control OMT", "maker_pt");
MqttSelect mqttSelect(&mqttDevice, "height", "Desk Height");

void setup()
{
    mqttSelect.addOption("yolo");
}

void loop()
{

}