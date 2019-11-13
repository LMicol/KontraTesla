# KontraTesla - Controle

Repositório para os códigos de controle para o trabalho desenvolvido na disciplina DLSC808 da UFSM no segundo semestre de 2019.

## Códigos principais

* [code_ard/controleGeral.ino](code_ard/controleGeral.ino): Verifica qual o dispositivo deve ser lido (a troca de dispositivos é feita através do botão do joystick - pressionar e soltar), obtém os valores e mapeia para dois valores, um referente a frente-trás e outro esquerda-direita, que serão escritos na serial para tratamento posterior pelo código em [controle.py](controle.py).
* [controle.py](controle.py): Lê a serial e, caso tenha recebido valores dos dispositivos, faz o processamento utilizando o cálculo de [controle-teste.py](controle-teste.py) para publicar os valores via MQTT.

## Códigos auxiliares

* [controle-teste.py](controle-teste.py): Recebe duas entradas (x e y), entre -128 e 127, para processar e mostrar a saída que será passada aos motores.
* [line2.py](line2.py): Visualização do posicionamento do volante, através de linhas, utilizando a biblioteca pygame.
* [pub.py](pub.py): Código simples para atuar como *publisher* usando MQTT.
