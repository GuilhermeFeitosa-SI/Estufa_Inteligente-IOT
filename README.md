# Estufa Inteligente IoT

## Problema que o projeto resolve

Este projeto tem como objetivo reduzir perdas de safras em estufas agrícolas causadas por variações de temperatura e umidade. O sistema realiza o monitoramento das condições do ambiente e pode acionar dispositivos automaticamente para manter condições adequadas para o cultivo.

## Componentes utilizados até o momento

* ESP32
* Sensor DHT11
* Módulo Relé 1 Canal
* Cooler/Fan 12V
* Fonte 12V
* Bomba de água 12V
* Sensor de umidade do solo
* Jumpers


## O que ainda falta implementar

* Rede Wi-Fi
* MQTT (HiveMQ)

## Funcionamento do sistema

O sensor DHT11 realiza a leitura da temperatura e da umidade do ambiente. Os dados são processados pelo ESP32, que verifica se os valores estão dentro dos limites definidos. Caso a temperatura ultrapasse o valor configurado, o ESP32 aciona um módulo relé que liga o cooler para auxiliar na ventilação da estufa.

As informações também são enviadas por meio do protocolo MQTT, permitindo monitoramento remoto dos dados coletados.

