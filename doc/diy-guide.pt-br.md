# Rota DIY!

Eu sei man, não deu pra comprar um Protopanda montado ou você só quer construir o seu próprio. Eu entendo. Como eles disseram no filme de 2005 "Robôs":
"Oh querido, sinto muito que você perdeu a entrega. Mas tudo bem, fazer o bebê é a parte divertida."

Vamos fazer um então >:3

# Importante

Este projeto ainda está em desenvolvimento e pouquíssimas pessoas seguiram a rota DIY até agora. É possível, dá pra fazer tranquilo! Mas não será tão bom quanto um Protopanda montado com a PCB fornecida.

Esse guia vai exigir que você saiba um pouco de solda e o básico de como usar um multímetro (medir voltagem, resistência e continuidade). Existem guias na internet ensinando isso, então isso não será ensinado aqui.

Algumas peças podem ser substituídas por outras, como os resistores, não precisam ser especificamente 3k e 10k, você pode usar tipo 30k e 100k. Eu testei até com 5k/15k. Se você souber o que é um divisor resistivo, a proporção deve ser cerca de 1/3.
O MCU não pode ser substituído. PRECISA SER `ESP32 N16R8`

Você está lidando com ESP32, este microcontrolador usa 3.3v, então se você acidentalmente conectar ou deixar uma ponte de solda entre o 9v ou 5v e qualquer um dos GPIOs, diga adeus ao seu ESP. xablau.

Alguns conversores buck têm problemas com capacitores extras, então o capacitor de 1000~4700uF necessário mais adiante no guia pode ser desnecessário.

# Peças necessárias

1) Um dos conversores buck 5v 3A: [opção 1](https://aliexpress.com/item/1005005505907937.html) / [opção 2](https://pt.aliexpress.com/item/1005011601387749.html) / [opção 3](https://pt.aliexpress.com/item/1005006009759175.html)
2) Gatilho PD: [opção 1](https://aliexpress.com/item/1005007889747084.html) / [opção 2](https://pt.aliexpress.com/item/1005012106478427.html)
3) [Módulo SD Card](https://pt.aliexpress.com/item/1005009221606482.html)
4) [Tela OLED](https://aliexpress.com/item/1005006141235306.html)
5) [Placa de desenvolvimento ESP32 N16R8](https://aliexpress.com/item/1005009906920237.html)
6) [Buzzer 5 ou 3.3v](https://aliexpress.com/item/1005006201550296.html)
7) [Conector para o HUB75](https://aliexpress.com/item/1005007851512814.html) (Pegue o de 16 pinos)
8) [Fita de LED WS2812b](https://pt.aliexpress.com/item/1005007989431712.html)
9) [Fan 40x10mm 5v](https://pt.aliexpress.com/item/1005009148488355.html)
10) Algumas barras de pino fêmea e macho
11) Resistores (1k, 3k e 10k)
12) Um capacitor eletrolítico entre 1000uF a 4700uF de pelo menos 6.3v (pode ser dispensável)
13) Qualquer botão push
14) Uma placa perfurada (perf board)
15) Alguns fios
16) Um cartão SD (tente pegar o menor que encontrar, tipo 2gb~8gb)
17) [2x painéis HUB75 P2.5](https://pt.aliexpress.com/item/1005006224809039.html)
18) Cabo de alimentação HUB75 (geralmente vem com o painel HUB75 quando você compra)
19) 2x cabos de dados HUB75 (geralmente vem com o painel HUB75 quando você compra)

Opcionais para uso externo:

* [Sensor de toque para boop (recomendado)](https://aliexpress.com/item/1005006246380749.html)
* [Receptor IR VS1838B se usar controle IR](https://pt.aliexpress.com/item/1005009595736688.html)

# Ferramentas

* Solda
* Ferro de solda
* Alicate de corte
* Um suporte para a PCB. Alguns chamam de "mão tripla" (terceira mão)
* Multímetro
* Alguns fios

Sim, você vai soldar coisas aqui. Eu não recomendo usar cabos jumper. Eles até funcionam, mas soltam facilmente e isso VAI acontecer muito e você vai ficar louco por que isso vai rolar no meio da sua fursuit parade.

# Convenções de nomenclatura

Existem algumas convenções de nomenclatura que podem ser confusas, então para ajudar você pode usar este guia durante a montagem.

* VCC - Significa 'Voltagem no Coletor Comum' (Voltage at the Common Collector). Geralmente onde você vai fornecer energia ou onde a energia está sendo gerada. No esquema isso geralmente significa 9V, 5v ou 3.3v dependendo de onde está apontado. **Em termos simples é o terminal positivo.**
* GND - Significa terra (ground). **Em termos simples é o terminal negativo.** É normal que seja compartilhado entre os elementos.
* SLC/SCK - Clock serial (Serial Clock), o pino de clock do barramento I2C. Às vezes é escrito como SLC, outras vezes como SCK. Eles são iguais.
* SDA - Dados seriais (Serial Data), o pino de dados do barramento I2C.
* GPIO / IO - Quando disser IO5, GPIO5 significa a mesma coisa.

# Montagem

Minha ideia com este guia é fornecer um passo a passo até você ter algo finalizado. Em cada passo vamos verificar se fizemos as coisas corretamente antes de prosseguir.
Pode parecer complicado no início, mas não se preocupe, meu amigo!

## Reunindo as peças

Com todas as ferramentas e peças, vamos lá!

![](./diy-assembly1.png)

Aqui está o que vamos construir.

![Diagrama](./diy-schematic.png "Esquema eletrônico")

> IMPORTANTE: No esquemático, tem `IO9`, `IO10`, `IO33` (inner number)... Note que o `GPIO9/IO9` está no pino 15 do esquemático (número de fora). Neste guia, quando dizemos "conecte algo ao IO15", isso significa conectar ao GPIO15 (que está na posição 8 do número de fora), e não ao pino 15 (que é o IO9). **USE OS NÚMEROS DENTRO DO QUADRADO, NÃO OS NÚMEROS FORA DELE.**

## Alimentação

> IMPORTANTE: Qualquer fio usado na trilha de 5V ou 9V deve ser mais grosso. Use algo como AWG 20 ou maior.
> Se você soldar o capacitor e o 5V parar de aparecer, significa que o módulo tem uma proteção contra sobrecorrente defeituosa, você pode ficar sem ele. Então se isso acontecer, apenas remova o capacitor!

Primeiro, certifique-se de usar alguma ferramenta abrasiva, como palha de aço ou lixa fina para limpar o lado do cobre da placa perfurada. O cobre oxida muito, muito rápido, e essa camada de óxido impede a solda de grudar na superfície da placa.

![](./diy-assembly3.png)

Com isso, vamos construir esta parte nesta seção:

![](./diy-assembly4.png)

Antes de tudo, vamos posicionar os componentes na PCB de forma que fique compacto.

![](./diy-assembly5.png)

A ideia é usar aqueles barramentos fêmea para segurar o módulo ESP32S3 de forma que possamos removê-lo se necessário. Também com o duplo propósito de levantá-lo um pouco para podermos colocar coisas embaixo!

Então coloque os barramentos fêmea nos pinos do ESP e coloque-o na PCB deixando pelo menos uma fileira de espaço da borda.

![](./diy-assembly6.png)

Agora solde **todos** os pinos e remova o módulo ESP dos pinos.

![](./diy-assembly7.png)

Agora é hora de conectar os módulos. Se você está começando, este diagrama pode ajudar a indicar onde conectar o quê.

![](./diy-assembly8.png)

Resumindo, há duas etiquetas no módulo gatilho PD, VCC e GND. VCC significa positivo 9V e GND significa negativo ou terra. Embaixo do conversor DC há etiquetas indicando a polaridade e onde é entrada/saída.

Certifique-se de não estar soldando GND ao IN+ nem VCC ao IN-. Coloque também dois barramentos fêmea de 2 pinos no gatilho PD e outros dois barramentos fêmea de 2 pinos na saída do conversor DC assim:

![](./diy-assembly9.png)

Coloque-os na placa assim. Empurre-os para baixo o máximo que puderem.

![](./diy-assembly10.png)

E, claro, solde os pinos do outro lado.

Agora conecte em uma bateria externa (power bank) que tenha carregamento rápido/Power Delivery, pegue um voltímetro e coloque as pontas de prova (**TOME CUIDADO PARA NÃO CURTO-CIRCUITÁ-LAS**).
Se mostrar 5V, você pode precisar ajustar aqueles interruptores.

![](./diy-assembly12.png)

Ajuste até mostrar 8.70v~9.10v.

![](./diy-assembly11.png)

Agora verifique a saída do conversor DC. Deve mostrar entre 4.9~5.2v.

![](./diy-assembly13.png)

Feito isso, vamos soldar o capacitor. Primeiro, observe cuidadosamente o capacitor, geralmente há uma faixa branca. Isso indica onde está o polo negativo. Coloque na placa com a polaridade correta!!!! **Isso é importante. NÃO COLOQUE O CAPACITOR AO CONTRÁRIO**

> Se você soldar o capacitor e o 5V parar de aparecer, significa que o módulo tem uma proteção contra sobrecorrente defeituosa, você pode ficar sem ele. Então se isso acontecer, apenas remova o capacitor!

![](./diy-assembly14.png)

Do outro lado você pode dobrar os terminais para funcionarem como fio. Basta cortar o excesso quando terminar.

![](./diy-assembly15.png)

Também é aceitável fazer ponte de solda nos dois pinos de cada lado do conversor DC como na foto acima.
Coloque o ESP32 nos barramentos fêmea e verifique se aqueles pinos destacados correspondem à foto. Você precisará conectá-los adequadamente. Você só precisa de um GND, mas é uma boa ideia conectar ambos se tiver paciência.

![](./diy-assembly16.png)

Tome cuidado ao virar a placa, porque olhando agora, o 5V é o segundo pino no topo. Mas olhando por baixo, isso inverte, é o segundo pino na parte inferior.

![](./diy-assembly17.png)

![](./diy-assembly18.png)

Agora, colocando o ESP32 e ligando, ele deve acender!

![](./diy-assembly19.png)

Feito!

## I2C

Para esta e todas as outras etapas, primeiro você precisa gravar o firmware. [Use este guia para isso.](./flashing-guide.md)

Depois de gravar o firmware, vamos conectá-lo à tela OLED.

![](./diy-assembly20.png)

Para este guia específico, usarei uma tela diferente, uma maior para facilitar tirar fotos.
Lembra quando eu disse "Sem cabos jumper!!". Sim, eu os usarei, mas eles serão soldados. A única razão pela qual estou usando agora é porque posso conectar/desconectá-los da tela OLED. Todo este hardware do guia será reaproveitado depois.

Apenas certifique-se de que SLC/SCK está indo no pino IO9 e SDA no IO8.
GND está em qualquer pino GND e VCC está no pino 3.3v do ESP. Verifique duas e três vezes se você conectou o fio no pino correto!

Use alguns fios para unir a PCB e a tela. Essa tela ficará dentro da moldura frontal do protogen, então você precisará que ela tenha esses fios de extensão.

![](./diy-assembly21.png)

Se tudo estiver correto, quando você ligar, deve mostrar uma imagem na tela:

![](./diy-assembly22.png)

Então você verá um erro aparecendo dizendo: "Sem cartão SD"

Se você não vir nada, verifique se conectou tudo corretamente. Verifique com o multímetro se há 3.3v chegando. Se necessário, use o testador de continuidade no multímetro para confirmar se o pino está correto.

## Opcionais da placa

Agora instalaremos o botão interno, o buzzer e o divisor resistivo!
Observe que todos esses podem ser ignorados.

* Se você não quiser o buzzer, edite o `config_defaults.hpp` comentando esta linha: `USE_BUZZER`
* Se você não quiser o botão interno para ativar o wifi na inicialização, edite o `config_defaults.hpp` comentando esta linha: `ENABLE_EDIT_MODE`
* Se você não quiser o divisor resistivo (recomendado se você não for usar o gatilho PD, mas uma bateria), edite o `config_defaults.hpp` comentando esta linha: `USE_PIN_BATTERY_IN`

![](./diy-assembly23.png)

Vou colocar o botão e o buzzer próximos um do outro naquele canto, mas honestamente, se você acha que há um lugar melhor, coloque-os lá.

![](./diy-assembly24.png)

É bastante coisa, mas vá devagar, parte por parte.
Para esta parte você pode usar qualquer fio. Pode usar fios finos, sem problema.

### Botão interno

Primeiro, verifique como um botão push funciona. Se eles têm 4 pinos, significa que dois são comuns como nesta imagem:

![](./diy-assembly26.png)

Com isso em mente, é assim que você o conecta:

![](./diy-assembly25.png)

* O resistor de 1k está conectado ao pino 3.3V.
* O outro pino do resistor está conectado ao pino do botão E ao pino IO39 no ESP32
* O outro pino do botão está conectado ao GND

### Divisor resistivo

![](./diy-assembly27.png)

* O resistor de 10k é conectado entre o pino IO3 e 9V (aquele conector de pino de 9v que colocamos no gatilho PD)
* O resistor de 3k é conectado entre GND e IO3

### Buzzer

![](./diy-assembly28.png)

* O buzzer é conectado entre o pino IO40 e GND.
Verifique a marcação de um + no buzzer. É lá que vai o IO40.

## Cartão SD

Alguns módulos de cartão SD podem ser maiores ou menores... Estou usando um grande no tutorial, mas pegue um que caiba para você!

Escolhi colocar o módulo do cartão SD ali porque esse era o formato da minha placa. Não precisa colocar especificamente ali.

![](./diy-assembly29.png)

É assim que você deve conectá-lo:

![](./diy-assembly30.png)

E quando concluído:

![](./diy-assembly31.png)

Agora, depois de verificar duas vezes, coloque o cartão SD (certifique-se de colocar todos os arquivos necessários no cartão SD como diz no [guia de gravação](./flashing-guide.md#o-que-vai-no-cartão-sd)).
Ao ligar, ele deve detectar o cartão. Se não detectar, verifique se você fez a [parte de configuração do cartão SD para o modo DIY](./flashing-guide.md#configuração).
Ou seja trocar o `PANDA_SD_MODE` para 1, e reduzir o clock para 40mhz

> Cartões SD maiores que 32GB não funcionam! Eles também devem estar formatados como FAT32

Se tudo correr bem, o Protopanda deve inicializar corretamente e todos os procedimentos devem mostrar isto:

![](./diy-assembly32.png)

Neste ponto você pode conectar o controle remoto e brincar como quiser!

> Se você estiver usando o mesmo módulo que estou usando nas fotos, conecte o VCC ao 5V em vez do 3.3v. Este módulo tem um regulador e precisa de 5v para operar.

## Tela HUB75

Esta é a parte mais complexa, porque você precisa soldar cerca de 13 fios nos GPIOs. Há muito espaço para erro, então faça DEVAGAR e verifique três vezes se está conectando corretamente.

![](./diy-assembly33.png)

![](./diy-assembly34.png)

Uma boa ideia é começar com os fios pretos (GND) para ter uma referência visual.

![](./diy-assembly35.png)

Então, depois de soldar dolorosamente todos os fios, deve parecer ocupado assim.

![](./diy-assembly36.png)

Lembre-se de que você ainda pode colocar alguns fios do outro lado da placa.

![](./diy-assembly37.png)

Antes de testar, vamos garantir que todos os fios estejam conectados! Coloque seu multímetro no modo de continuidade e teste CADA PINO.

![](./diy-assembly38.png)

> Importante: Os pinos estão sendo vistos de cima para baixo. Se você ver pelo outro lado da placa ou se olhar o conector de frente, esquerda e direita trocam de lugar.

Agora que você verificou que cada pino está conectado corretamente, vamos conectar a alimentação dos painéis.
Pegue o cabo de alimentação que veio com os painéis e corte as pontas.

![](./diy-assembly39.png)

Agora eles devem ir no 5V e no GND.

![](./diy-assembly40.png)

![](./diy-assembly41.png)

Agora, certifique-se de que está conectado ao 5V, meça a saída e deve ser 5v. Se estiver correto, desconecte tudo e conecte o cabo do painel e a alimentação.

![](./diy-assembly42.png)

Ligue e você deverá vê-lo funcionando!!

![](./diy-assembly43.png)