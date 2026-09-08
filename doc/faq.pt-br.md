# FAQ

**O que é o Protopanda?**

No início era só o nome do firmware que eu criei pra facilitar minha vida na hora de fazer protogens. Mas hoje é todo um ecossistema, incluindo hardware, firmware e modelos 3D.

---

**Preciso pagar alguma coisa?**

Só se você quiser montar o seu, aí vai ter que pagar pelos componentes e peças hehe.

---

**Funciona em [insira aqui outro MCU que não seja o ESP32-S3]?**

Não.

---

**Dá pra usar matrizes MAX7219 em vez de HUB75?**

Sim! [Dê uma olhada no guia de configuração.](./configuring.pt-br.md)

---

**Dá pra usar um microfone?**

Sim! [Dê uma olhada no guia de configuração.](./configuring.pt-br.md)

---

**Como eu mudo a velocidade da animação?**

[Aqui](./configuring.pt-br.md#expressões). Você também pode simplesmente repetir um frame quantas vezes quiser.

---

**Eu tenho um controle, mas ele não conecta, por quê?**

Segure o botão boot/interno por 5s até a tela mostrar "Waiting for controller".

---

**Eu realmente preciso do cartão SD?**

Sim. Tecnicamente é possível adaptar o código pra usar só a flash interna, mas isso traz uma série de problemas e limitações de espaço de armazenamento. Então fica com o cartão SD mesmo.

---

**Como eu mudo o padrão dos LEDs laterais?**

Você pode [dar uma olhada no guia de configuração](./configuring.pt-br.md) no tópico de LED.

---

**Fiquei travado na tela "SD NOT FOUND".**

Confira a fiação. Sério. Confira de novo, pino por pino. Na maioria das vezes é só fiação malfeita.
Se você soldou, confira a sua solda.
Se isso não resolver o problema, formate o cartão SD para FAT32.
Se ainda assim falhar, tente outro cartão SD.
Se ainda assim falhar, troque o módulo do cartão SD.

---

**Por que o boop está sempre sendo acionado?**

Você provavelmente ligou o sensor de boop errado. Troque os pinos no `misc.json`:
```json
        "gpio": 48,
        "power_gpio": 13,
```
Troque o 48 pelo 13 e o 13 pelo 48.

---

**Quando eu ligo meu proto, fico travado numa tela de "waiting controller".**

Na primeira vez que o Protopanda liga, ele vai pedir um controle remoto. Você pode pular isso mudando algumas configurações no `misc.json`.
Você pode desabilitar o bluetooth com `"mode": "BLE",`. Você pode escolher none ou infrared. Com none, nenhum input será usado e o Protopanda não terá como mudar de expressão a não ser que você programe algo.
No modo infrared, você vai precisar adicionar um receptor infravermelho para usar um controle IR.

---

**Já pareei um controle, mas continua travado nessa tela de controle!**

Você precisa conectar o botão interno! Ele está no esquemático. Se você já conectou, ou ele está preso pressionado, ou você ligou errado.

---

**Eu mudei as expressões faciais no cartão SD, mas nada mudou.**

As expressões são pré-decodificadas e colocadas em cache para exibi-las mais rápido. Então você vai precisar limpar esse cache.
Apague a pasta 'cache', ou vá em settings > Rebuild bulk file no menu do seu proto.

---

**Eu mudei um arquivo .json e regravei o firmware, mas nada mudou!**

Os arquivos JSON devem estar no cartão SD, não no firmware. Mude o json no seu cartão SD e vai funcionar.

---

**Comprei o controle recomendado no aliexpress, mas ele não pareia!**

Confira se o controle está ligado. Se for o mesmo controle e se no `keybinds.json` essa configuração está definida como true: `"enableHidControllers": true,`
