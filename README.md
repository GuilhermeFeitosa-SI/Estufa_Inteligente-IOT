# Estufa Inteligente IoT

## Problema que o projeto resolve

Este projeto apresenta um protótipo de sistema IoT para monitoramento e controle de variáveis ambientais em estufas agrícolas. O sistema realiza a leitura de parâmetros como temperatura e umidade, possibilitando o acompanhamento das condições do ambiente e o acionamento automatizado de dispositivos, visando demonstrar uma solução para auxiliar no controle das condições de cultivo.

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

## Como executar o projeto

Antes de iniciar, é necessário ter o **Node.js** instalado na máquina.

Após instalar o Node.js, abra o terminal na pasta do projeto e execute o comando:

```bash
node server.js
