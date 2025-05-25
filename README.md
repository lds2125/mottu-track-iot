Guia de Uso


1. Simulação do Hardware (Wokwi)

.  Acesse o wokwi.
.  Carregue `arduino-esp32/mottu_track_esp32.ino` em `sketch.ino`.
.  Carregue `diagram.json` em `diagram.json`.
.  Inicie a simulação.
    * Verificação: LEDs azul e ciano acesos (Wi-Fi/MQTT conectado).

2. Dashboard de Monitoramento (Node-RED)

.  *Configure o nó `mqtt in` "Status Vagas Pátio A":
    * Servidor: `HiveMQ Public Broker` (`broker.hivemq.com:1883`, Client ID vazio).
    * Tópico: `mottu/filial_simulada/patio_A/#`.
.  Acesse o dashboard em `http://localhost:1880/ui`.

3. Interação

.  No simulador Wokwi, pressione e segure um botão de vaga.
    *Resultado: A vaga correspondente no dashboard mudará de VERDE (LIVRE) para VERMELHO (OCUPADA).
.  Solte o botão.
    *Resultado: A vaga retornará a VERDE.
