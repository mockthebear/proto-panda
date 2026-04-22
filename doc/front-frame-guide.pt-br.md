# Estrutura Frontal

Aqui você aprenderá como montar a estrutura frontal. Observe que algumas peças podem ser um pouco diferentes se você estiver fazendo uma Protopanda DIY em vez de uma montada com a PCB.

1) [Ferramentas](#ferramentas)
1) [Painéis](#painéis)
1) [Cabos HUB75](#cabos-hub75)
1) [Cabo de alimentação](#cabo-de-alimentação)
1) [Outros cabos](#outros-cabos)
1) [Tela interna](#tela-interna)
1) [Sensor Boop](#sensor-boop)
1) [Cooler](#cooler)
1) [Segundo painel](#segundo-painel)
1) [Suporte para o pelo](#suporte-para-o-pelo)
1) [Capa do conector](#capa-do-conector)
1) [Lente de Fresnel](#lente-de-fresnel)
1) [Cabo USB](#cabo-usb)
1) [Materiais](#materiais)


## Materiais

O primeiro e mais importante material é a estrutura frontal impressa para a matriz P2.5.

Então você precisará da eletrônica:

* 2x [Painéis de LED P2.5 HUB75](https://pt.aliexpress.com/item/1005005793147869.html)
* 2x Cabos HUB75 (geralmente acompanham o painel)
* 1x Cabo duplo para as matrizes HUB75 (geralmente acompanha o painel)
* 1x [Fan 4010 5v 40mmx10cm](https://pt.aliexpress.com/item/1005009148488355.html)
* 1x Cabo USB
* 1x [Tela oled 0.96 polegadas](https://pt.aliexpress.com/item/1005006262908701.html)
* Alguns cabos
* 4x Parafusos M2 6mm
* 4x Porcas M2
* 2x Parafusos M3 16mm~18mm
* 4~6x Parafusos M3 8mm
* 2x [Porca M3 auto-roscante](https://pt.aliexpress.com/item/1005008207473770.html)


![img](./front-frame-materials.png)

### Opcionais

* 2x [Terminal IDC 8x2](https://pt.aliexpress.com/item/1005007995752212.html)
* 2x Porcas M3
* 2x Parafusos M3 8MM
* 2x Parafusos M3 4MM
* [Lente de Fresnel](https://pt.aliexpress.com/item/1005009696060069.html)
* [KF2510](https://pt.aliexpress.com/item/1005011814149800.html?) Pegue o de 4 pinos.

### Ferramentas

* Chave de fenda
* Ferramentas para crimpagem e terminais de crimpagem (depende de como você vai fazer)

# Guia de montagem

## Painéis

Primeiro precisamos desmontar os painéis para remover o suporte original. Use uma chave de fenda fina.
Há um total de 6 parafusos, eles são bem pequenos, então tome cuidado e guarde-os.

![img](./front-frame-screw-1.png)

Depois de removidos, retire o suporte.

![img](./front-frame-screw-2.png)

Agora é hora de colocar um dos painéis no lado ESQUERDO da estrutura.

![img](./front-frame-fixing-panel-1.png)

Há uma orientação correta do painel. Observe que no painel há setas brancas. Alinhe-as apontando para frente como as setas vermelhas. Alinhe também o painel usando o furo apontado pela seta amarela.

Em seguida, siga estes passos:
1) Alinhe o painel
2) Coloque o parafuso no furo
3) Empurre o parafuso e alinhe o painel
4) Com uma chave de fenda pequena ou uma ponta de chave de fenda, aperte o parafuso. **NÃO APERTE MUITO, apenas o suficiente para fixar**
5) Faça isso nos 5 pontos destacados em vermelho.

![img](./front-frame-fixing-panel-2.png)

## Cabos HUB75

Agora é hora de colocar o primeiro cabo. Este cabo transferirá o sinal do primeiro painel para o segundo.

![img](./front-frame-cable-1.png)

Mas como você pode ver, está um pouco longo demais. Você pode deixar como está ou pode cortar o cabo. É o que vou ensinar agora. Mas preste muita atenção no que está fazendo; se você os conectar ao contrário, certamente danificará um ou ambos os painéis.

Se quiser deixar como está, pule para [Cabo de alimentação](#cabo-de-alimentação)

Como você pode ver, o cabo tem cerca de 19~20cm. Mas precisamos de pelo menos 11CM.

![img](./front-frame-cable-2.png)

Então, para isso, você precisará cortar o cabo em cerca de 13,5cm~14,5cm

![img](./front-frame-cable-3.png)

Em seguida, pegue o terminal IDC e coloque-o na orientação correta. **VERIFIQUE DUAS OU TRÊS VEZES A ORIENTAÇÃO COM BASE NO CABO ORIGINAL**

![img](./front-frame-cable-4.png)
![img](./front-frame-cable-5.png)

Se precisar de ajuda extra, pesquise no Google sobre como crimpar conectores IDC.

## Cabo de alimentação

Para alimentar o HUB75, você pode usar o cabo original. Mas ele é volumoso e extremamente longo. Então você pode cortá-lo.

![img](./front-frame-cable-6.png)

Em seguida, corte bem no ponto onde ele se junta com o outro cabo

![img](./front-frame-cable-7.png)

Depois disso, remova os terminais extras; precisamos apenas de um preto e um vermelho em cada um desses dois.

![img](./front-frame-cable-8.png)

Em seguida, descasque cerca de 1cm de cada fio, torça-os juntos, coloque um shrink em cada um e crimpe um terminal garfo assim:
![img](./front-frame-cable-10.png)

É assim que eles devem ficar:

![img](./front-frame-cable-9.png)

Eles vão na Protopanda no terminal de parafuso.

![img](./front-frame-cable-11.png)

Se você estiver fazendo o roteiro DIY, eles vão no esquema onde está escrito **HUB75 POWER**. Caso não esteja claro, vermelho significa positivo e preto negativo.

![img](./front-frame-cable-12.png)

## Outros cabos

Agora é uma boa ideia conectar os outros cabos.

![img](./front-frame-cable-13.png)

Se você estiver fazendo o roteiro DIY, pode escolher qualquer tipo de conector que quiser, ou apenas conectar diretamente ao display e ao sensor Boop. Eu usei conector PHT, mas foi minha preferência pela forma como fiz. Faça do seu jeito!

![img](./front-frame-stuff-1.png)

Especificamente para o controlador Proto Panda, há um local dedicado a ele. Mas o roteiro DIY pode ocupar mais espaço. Essa parte dependerá muito do que e como você construiu. Os furos dos parafusos são M2.

![img](./front-frame-stuff-3.png)

Com uma chave de fenda longa, você pode alcançar todos os 3 parafusos.

![img](./front-frame-stuff-4.png)

Então deve ficar assim.

![img](./front-frame-stuff-5.png)

## Tela interna

Você pode usar totalmente a tela com o header de pinos. Mas pessoalmente, gosto de usar conectores [KF2510](https://pt.aliexpress.com/item/1005011814149800.html).

Eu soldei o conector de 90 graus e cortei os pinos que saíam do outro lado.

![img](./front-frame-stuff-6.png)

Em seguida, pegue os parafusos e as porcas para fixá-la na estrutura:

![img](./front-frame-stuff-7.png)

Coloque os parafusos através dos furos

![img](./front-frame-stuff-8.png)

E aperte as porcas do outro lado

![img](./front-frame-stuff-9.png)

## Sensor Boop

Haverá um guia para fazer o sensor Boop no futuro. Farei isso, eu juro! Por enquanto, é apenas o sensor capacitivo com um pouco de fio enrolado ao redor.

![img](./front-frame-boop-1.png)

Prenda o sensor Boop na frente do painel

![img](./front-frame-boop-2.png)

## Cooler

Para o cooler, estamos usando 2x parafusos M3 16mm e as porcas. Primeiro conecte o fan ao soquete. Se estiver fazendo o roteiro DIY, use os mesmos pinos da alimentação do HUB75.

![img](./front-frame-fan-1.png)

Em seguida, coloque o fan na parte inferior da estrutura frontal, com os fios voltados para frente.

![img](./front-frame-fan-2.png)
![img](./front-frame-fan-3.png)

Use as porcas na parte inferior e aperte os parafusos.

![img](./front-frame-fan-4.png)

## Segundo painel

Agora que não estamos mais mexendo com fiações, vamos conectar os últimos cabos no segundo painel. **PRESTE ATENÇÃO NA DIREÇÃO DAS SETAS NO PAINEL.**

Deve haver 4 parafusos para fixá-lo no lugar. Eles estão circulados em amarelo.

![img](front-frame-panel-fix-1.png)

Não há necessidade do 5º. O processo para alinhar o painel é o mesmo do anterior.

## Suporte para o pelo

Caso você precise de um lugar para costurar ou prender o pelo, há um suporte.

![img](./front-frame-spp-1.png)

Use os parafusos M3 8mm e as porcas para fixá-lo no lugar. Ele deve se encaixar, mas precisa do parafuso para permanecer ali.

![img](./front-frame-spp-2.png)
![img](./front-frame-spp-3.png)

## Capa do conector

Como não queremos danificar os conectores HUB75 com nossas bochechas, vamos adicionar uma capa para eles.
Use 2x parafusos M3 8MM para fixá-la na estrutura frontal

![img](./front-frame-cover-1.png)

Deve ficar assim:

![img](./front-frame-cover-2.png)

## Lente de Fresnel

A lente de Fresnel é completamente opcional, mas é bom ter. Sem ela, pode ser difícil focar sua visão na tela interna. Com ela, é muito mais fácil. Os efeitos são principalmente presenciais.

![img](./front-frame-fresne-1.png)

Para montá-la, você precisará cortar a lente Fresnel em um retângulo de 64x34cm. Cole-a no slot impresso em 3D usando super cola (APENAS UMA GOTA PEQUENA)

![img](./front-frame-fresne-2.png)

Aparafuse um parafuso M3 8MM para prender ambas as partes

![img](./front-frame-fresne-3.png)

Em seguida, use os dois parafusos M3 5MM e coloque-os no furo da outra parte

![img](./front-frame-fresne-4.png)

Agora alinhe na estrutura frontal. Ela vai na parte interna superior direita.

![img](./front-frame-fresne-5.png)

Em seguida, aparafuse no lugar e você terá uma lente de Fresnel que pode ser movida para cima para sair do caminho quando o visor não estiver presente, ou simplesmente desparafusada caso não goste dela!

![img](./front-frame-fresne-6.png)

## Cabo USB

Agora, para o cabo USB. Na lista de materiais, pedi um USB-C de 90 graus. Não há muito espaço para um cabo USB reto ali, e isso pode forçar o cabo ou o conector. Até mesmo o cabo virado para o lado pode sofrer muita tensão.

![img](./front-frame-usb-1.png)

É por isso que vamos colocá-lo em um slot.

![img](./front-frame-usb-2.png)

Certifique-se de que o cabo está passando dentro daquele slot e fazendo uma pequena curva no interior. Se necessário, empurre-o um pouco para dentro.

![img](./front-frame-usb-3.png)

# ESTÁ COMPLETO!

![img](./front-frame-completed.png)

**Aí está! Você completou!!**