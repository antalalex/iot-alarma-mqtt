# iot-alarma-mqtt

LINK WOKWI PROIECT: https://wokwi.com/projects/453168515648644097

Proiect IoT: sistem de alarmă bazat pe ESP32 și MQTT, cu detectare de mișcare și transmitere de evenimente către HiveMQ Cloud.

# Sistem de alarmă IoT bazat pe MQTT

Proiect realizat pentru disciplina **Sisteme cu microcontrolere și IoT industriale**.

## Descriere
Sistem de alarmă IoT implementat pe ESP32 care detectează mișcare folosind un senzor PIR.
La detectarea mișcării:
- se activează alarma locală (LED + buzzer)
- se trimit evenimente către brokerul MQTT HiveMQ Cloud

## Tehnologii utilizate
- ESP32
- Protocol MQTT
- HiveMQ Cloud
- Wokwi Simulator
- Arduino Framework

## Topicuri MQTT
- `iot/alarma/event`
- `iot/alarma/pir`
- `iot/alarma/status`
- `iot/alarma/cmd`

## Simulare
Proiectul este testat în Wokwi și monitorizat folosind HiveMQ Web Client.

