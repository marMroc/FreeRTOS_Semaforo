# FreeRTOS_Semaforo
Projeto prático de monitoramento de distância e identificação de gases inflamáveis, enviando estes dados para um monitor LCD

Resumo:

Este projeto teve como objetivo o desenvolvimento de um projeto prático com Arduíno e FreeRTOS com a finalidade de medir uma distância com um sensor ultrassônico, ler um sensor de gases inflamáveis e exibir as informações em um display LCD. 

Componentes:
- Arduíno Mega 2560
- Display LCD I2C 16X2
- Sensor Ultrassônico de distância HC-SR04
- Sensor de gases inflamáveis MQ-2

Circuito esquemático:


Bibliotecas: 
- FreeRTOS (by Richard Barry) versão 10.0.0-10
 Possível instalar no FreeRTOS IDE apenas realizando a busca pelo nome
- Sensor Ultrassônico (HC-SR04)
 Disponível no reposítório https://github.com/makerhero/Ultrasonic

Desafios:

Utilizar os conceitos de FreeRTOS para dividir as ações do sistema em tarefas da melhor forma possível.

Divisões das tarefas:

 As tarefas foram divididas da seguinte forma:

Tarefa 1: Ler a distância com o sensor ultrassônico a cada meio segundo e enviar as leituras para a tarefa 3, além de escrever no serial monitor a cada leitura feita.
Tarefa 2: Ler os gases inflamáveis presentes a cada segundo, além de realizar o envio destas leituas para a tarefa 3, escrevendo no serial monitor a cada leitura feita.
Tarefa 3: Exibir no display LCD as leituras enviadas pelas tarefas 1 e 2. 

Aprendizados:
- Fixação dos conceitos de sistemas operacionais em tempo real, como tarefas, semáforos, filas e gerenciamento de memória.
- Realizar o planejamento e análise de requisitos do sistema para evitar condições de corrida e deadlock.
- Sincronizar e realizar a comunicação entre as tarefas.
- Fixar conceitos de gerenciamento de memória.
