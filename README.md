
# Projeto Final de Sistemas Embarcados

# Controle de um Sistema de Elevadores


# REQUISITOS FUNCIONAIS:

1.	Os elevadores, quando parados em um andar, ficarão com a porta aberta.
2.	Os elevadores só se movimentam se estiverem com as portas totalmente fechadas.
3.	As portas dos elevadores somente se abrem se os elevadores estiverem totalmente parados e na posição correta de um andar com uma margem de erro de +- 10 mm.
4.	Se um botão interno do elevador for pressionado, deverá ser acendido para indicar que foi acionado, e após a chegada ao andar, deverá ser apagado.
5.	Ao ser pressionado um botão externo em um andar, tanto para cima como para baixo, o elevador que irá atender é o correspondente ao lugar(esquerda, central ou direita) que o botão foi pressionado.
6.	As chamadas dos elevadores serão atendidas por um sistema de prioridades estáticas e dinâmicas, conforme as condições situacionais.
7.	O tempo em que as portas deverão ficar abertas em cada andar requisitado deve ser de no mínimo 5 segundos.
8.	Os elevadores, quando sem nenhuma requisição para atender, deverão voltar ao andar térreo, para lá aguardar uma nova requisição.





# REQUISITOS NÃO FUNCIONAIS:

1.	O controlador deverá ser implementado utilizando o kit EK-TM4C1294XL.
2.	O controlador deverá ser implementado em linguagem C utilizando o RTOS RTX5.
3.	O controlador deverá utilizar programção concorrente com o RTOS RTX5.
4.	A comunicação do controlador com o simulador deverá ser efetuada através da UART, por meio de interrupções. 
5.	Os elevadores deverão possuir, cada um, uma fila de chamadas com 32 espaços para armazenar a ordem que os botões internos e externos foram pressionados.
6.	Cada fila de chamadas deverá atribuir uma prioridade estática e outra dinâmica a cada requisição feita por acionamento de botões.
7.	Quanto maior o valor da prioridade, que é a soma da prioridade estática com a dinâmica, maior será sua prioridade.
8.	A prioridade estática dos botões internos é de 16 e a dos botões externos é de 8.
9.	A prioridade dinâmica começa em 0 e é incrementada em mais 1 a cada parada do elevador.
10. A prioridade dinâmica também é incrementada em +1, caso a distancia do andar de requisição seja menor ou igual a 4 andares que o andar atual do elevador.
11. A prioridade dinâmica também é incrementada em +2, caso a distancia do andar de requisição seja menor ou igual a 3 andares que o andar atual do elevador.
12.	A prioridade dinâmica também é incrementada em +3, caso a distancia do andar de requisição seja menor ou igual a 2 andares que o andar atual do elevador.
13. A prioridade dinâmica também é incrementada em +4, caso a distancia do andar de requisição seja menor ou igual a 1 andares que o andar atual do elevador.
14.	O acionamento de parada no andar indicado deverá ocorrer assim que o controlador receber a interrupção indicando que o elevador selecionado chegou ao andar de destino.
15.	Se o andar onde o elevador parou pela requisição de maior prioridade também era o andar de uma outra requisição de menor prioridade que solicitava o mesmo andar, então, ambas serão consideradas atendidas. 
