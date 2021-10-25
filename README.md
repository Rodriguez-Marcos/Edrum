# Bateria casera ArduDrum

## Objetivos:

En este proyecto se busca crear un sistema embebido con arduino que a travez del puerto USB o con un traductor de USB a MIDI se puedan leer estas señales y sintetizarlas con una pc o un sintetizador

## Materiales:

- Arduino UNO, Mega, Nano, o Leonardo
- PC con algun programa sintetizador midi (recomendado ableton live)
- Resistencias de 333k ohm
- Piezo electrico de 50mm
- Protoboard o placa personalizada para el circuito
- Cables para protoboard

## Como construir el proyecto:

Compilar el programa que se encuentra en ``/Arduino Code`` en su placa de Arduino y realizar el siguiente circuito en su protoboard:


<img src="img\scheme.jpg" alt="drawing" width="200"/>



Una vez realizado los pasos anteriores debe conectar su placa de Arduino a su PC, abrir el programa ableton live, por ultimo deberia linkear el programa con La placa arduino tal como se muestra en la sigueinte imagen:

<img src="img\ableton-external-instrument.png" alt="drawing" width="600"/>

el ultimo paso es cargarle sonidos a su bateria y ya podra disfrutar de su bateria casera hecha con un coste minimo :)