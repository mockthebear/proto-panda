# Flashing Protopanda

Já que você está aqui, há algumas coisas que precisam ser mencionadas antes.
Este guia cobrirá como configurar o ambiente, como compilar, como gravar o firmware e o que colocar no cartão SD.
Atualmente, este guia NÃO inclui uma maneira de atualizar coisas do cartão SD de uma versão mais antiga para uma mais nova. Se você está aqui para isso, considere reescrever o animation.json ou outro arquivo que você modificou com suas alterações. Está planejada uma ferramenta para atualização automática no futuro, mas por enquanto é totalmente manual.

## Guias

1) [Configurando o ambiente](#configurando-o-ambiente)
2) [Compilando e gravando o firmware](#compilando-e-gravando-o-firmware)
3) [O que vai no cartão SD](#o-que-vai-no-cartão-sd)

# Configurando o ambiente

O Protopanda é escrito em C++ e projetado para ser usado em um ESP32S3 utilizando o framework Arduino. Para isso, você precisará apenas de um software e um plugin para ele. E TALVEZ um driver.

## Passo 1 - Software e drivers

O primeiro software que você precisa é o [Visual Studio Code](https://code.visualstudio.com/download). Ele está disponível para Linux e Windows.

Depois de instalado, abra e vá até o menu da direita onde está escrito Extensions (ou pressione Ctrl+Shift+X).
Dentro dele, procure por `pioarduino` e instale.

![](./flashing-guide-1.png)

Pode levar algum tempo para baixar todos os arquivos e pode solicitar que você reinicie o VS Code.

**Certifique-se de não ter platformio e pioarduino instalados ao mesmo tempo**

## Passo 2 - Baixando o Protopanda

Agora vá até a página do Protopanda no GitHub, clique para baixá-lo.

![](./flashing-guide-2.png)

Descompacte-o em algum lugar, depois vá para o VS Code e selecione "Open Folder" (Abrir Pasta) para a pasta que você descompactou.

![](./flashing-guide-3.png)

Pode perguntar se você deseja baixar o toolchain e os recursos para este projeto, marque sim e aguarde até que tudo seja baixado.

![](./flashing-guide-4.png)

## Passo 3 - Conectando o dispositivo

Se você estiver usando uma placa ESP32S3 (o caminho DIY/faz você mesmo), a placa tem duas portas USB-C. Verifique embaixo da placa qual está escrito "COM". É essa!

![](./flashing-guide-8.png)

Mas se você decidiu comprar um Protopanda montado ou fez o seu usando a PCB fornecida, use a porta USB-MINI na frente do controlador.

![](./flashing-guide-9.png)

## Passo 4 - Drivers?!

Agora conecte o Protopanda ou o ESP32 ao seu computador. Se aparecer uma mensagem dizendo "Dispositivo desconhecido" ou algo assim, significa que você pode precisar de um driver.

Primeiramente, dependendo de qual placa você está usando, pode ser necessário baixar um driver. Se você estiver usando um Protopanda comprado montado, será um [driver ch340](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all). Caso contrário, será outro. **Baixe apenas se for realmente necessário!**
A maioria dos sistemas operacionais já vem com a maioria desses drivers ou os instala automaticamente.

Se você não receber nenhuma mensagem, pode clicar no ícone de tomada no canto inferior esquerdo do VS Code. Esse ícone é usado para selecionar a porta de comunicação serial com o dispositivo.

![](./flashing-guide-5.png)

Clicando nele, um menu aparecerá na parte superior da tela.

![](./flashing-guide-6.png)

Se você vir apenas "COM1", então significa que você está "ferrado" e precisa instalar um driver (ou o dispositivo não está conectado).
Se você vir algo como "COM<número>", então significa que está tudo certo, selecione essa porta e prossiga.

![](./flashing-guide-7.png)

No cenário de haver mais de duas entradas, primeiro desconecte o dispositivo, veja qual está faltando e depois conecte novamente, essa é a porta certa.

# Compilando e gravando o firmware

Certifique-se de ter concluído a etapa de configuração anterior.

## Compilação

Para compilar, basta ir até a barra inferior, lá tem um ícone ✅. Quando você passar o mouse, ele mostra: Build (Compilar). Clique nele!

![](./flashing-guide-10.png)

Clicando, ele mostrará um terminal integrado na parte inferior do VS Code.
Se for a primeira vez, pode ser necessário instalar mais dependências, o que é normal, apenas faça isso e aguarde enquanto vários arquivos são compilados.

![](./flashing-guide-11.png)

Tudo bem se aparecerem algumas mensagens de aviso amarelas.
Quando terminar, você verá algo como isto no terminal:

```
Linking .pio\build\debug\firmware.elf
Retrieving maximum program size .pio\build\debug\firmware.elf
Checking size .pio\build\debug\firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [==        ]  20.4% (used 66692 bytes from 327680 bytes)
Flash: [=======   ]  69.6% (used 2188584 bytes from 3145728 bytes)
Building .pio\build\debug\firmware.bin
esptool.py v4.8.1.1
Creating esp32s3 image...
Merged 2 ELF sections
Successfully created esp32s3 image.
```

Pronto, você compilou!

## Configuração

Existe um arquivo `include/tools/config_defaults.hpp`. Nele você pode alterar algumas configurações do projeto.
Se você decidiu montar o seu próprio Protopanda, então você DEVE modificar essas configurações:

```cpp
#define PANDA_SD_MODE 2
#define SPI_MAX_CLOCK (80 * 1000 * 1000)
```
Mude para:
```cpp
#define PANDA_SD_MODE 1
#define SPI_MAX_CLOCK (40 * 1000 * 1000)
```

Isso porque quando se usa fios, especialmente mais longos, pode causar ruído quando se opera em altas velocidades. Trocando o clock para a metade impede que esse ruído seja significativo ao ponto de atrapalhar a operação.

Além disso, alguns módulos SD têm uma modificação que um ou dois pinos estão ligados ao GND, fazendo assim impossível que ele opere no modo SD_MMC. Para trocar de volta para o modo SPI basta alterar PANDA_SD_MODE para 1. O modo SPI é mais lento porém funciona na maioria dos casos.

Sempre que alterar algo nesse arquivo, é necessário recompilar e gravar novamente o firmware.

## Gravando (Flashing)

Agora que você compilou, certifique-se de que o dispositivo está conectado e que você [configurou corretamente a porta COM](#passo-3---conectando-o-dispositivo). Se fez tudo certo, ao lado do ✅ há uma seta apontando para a direita. Clique nela!
Se for a primeira vez, também pode solicitar o download de alguma outra ferramenta. Essa é a última!

Você verá algo assim:

```
Checking size .pio\build\release\firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [==        ]  20.4% (used 66692 bytes from 327680 bytes)
Flash: [=======   ]  67.0% (used 2106412 bytes from 3145728 bytes)
Configuring upload protocol...
AVAILABLE: cmsis-dap, esp-bridge, esp-builtin, esp-prog, espota, esptool, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa
CURRENT: upload_protocol = esptool
Looking for upload port...
Auto-detected: COM7
Uploading .pio\build\release\firmware.bin
esptool.py v4.8.1.1
Serial port COM7
Connecting....
```

Se tudo der certo, você verá:

```
SHA digest in image updated
Compressed 19728 bytes to 12773...
Writing at 0x00000000... (100 %)
Wrote 19728 bytes (12773 compressed) at 0x00000000 in 0.4 seconds (effective 449.3 kbit/s)...
Hash of data verified.
Compressed 3072 bytes to 144...
Writing at 0x00008000... (100 %)
Wrote 3072 bytes (144 compressed) at 0x00008000 in 0.0 seconds (effective 653.3 kbit/s)...
Hash of data verified.
Compressed 8192 bytes to 47...
Writing at 0x0000e000... (100 %)
Wrote 8192 bytes (47 compressed) at 0x0000e000 in 0.1 seconds (effective 914.8 kbit/s)...
Hash of data verified.
Compressed 2188960 bytes to 1262258...
Writing at 0x00010000... (1 %)
Writing at 0x0001cd16... (2 %)
Writing at 0x00029fbb... (3 %)
Writing at 0x00037e4d... (5 %)
Writing at 0x0003fc5a... (6 %)
Writing at 0x0004811a... (7 %)
Writing at 0x0004dd1f... (8 %)
Writing at 0x00052c60... (10 %)
Writing at 0x0005c4a7... (11 %)
Writing at 0x000685e1... (12 %)
Writing at 0x0007e43d... (14 %)
```

Mas se você continuar vendo:

```
Connecting..................
```

Isso provavelmente significa que o ESP não está entrando no modo de boot corretamente. Você pode forçar segurando o botão flash e depois pressionando uma vez o botão reset na placa.
No caso do Protopanda montado, há dois botões na parte superior do case, o da esquerda é o flash e o da direita é o reset.

![](./flashing-guide-12.png)

Na placa ESP32 eles estão escritos como "RST" e "BOOT".

![](./flashing-guide-13.png)

## Lendo o serial

Você pode depurar e ler algumas saídas seriais que o Protopanda gera. Com o dispositivo gravado e conectado ao computador, há um segundo ícone de tomada, chamado "Serial monitor" (Monitor Serial).

![](./flashing-guide-14.png)

Clicando nele, uma aba será aberta e mostrará alguns logs. É assim que eles se parecem:

```
--- Terminal on COM7 | 115200 8-N-1
--- Available filters and text transformations: debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time
--- More details at https://bit.ly/pio-monitor-filters
--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0xa (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce2820,len:0x10f0
load:0x403c8700,len:0x4
load:0x403c8704,len:0xbec
load:0x403cb700,len:0x2fcc
entry 0x403c88ac
Starting proto panda v3.0.2!
[241][I] Running I2C Scan...
[244][I] I2C device found at address 60

Starting sd card mode as MMC
[3166][I] [Memory] 85.9% free - 270888 of 315504 bytes free (psram: 8388608 / 8284620  -> 98.8%)
[3222][I] DMA display initialized!
[3238][I] [Memory] memmory_difference msg="Dma display" heap=-125884 psram=-1196
[3254][I] [Memory] 45.9% free - 144868 of 315504 bytes free (psram: 8388608 / 8283424  -> 98.7%)
[3271][I] [Memory] memmory_difference msg="Storage" heap=-152 psram=932
[3287][I] [Memory] 45.8% free - 144588 of 315504 bytes free (psram: 8388608 / 8284356  -> 98.8%)
6[3330][I] Reset reason: Vbat power on reset
[3356][I] [Memory] memmory_difference msg="Devices" heap=-576 psram=0
[3383][I] [Memory] 45.7% free - 144040 of 315504 bytes free (psram: 8388608 / 8284356  -> 98.8%)
[3409][I] Starting FFAT
[3439][I] [Memory] memmory_difference msg="FFAT" heap=-924 psram=-18896
[3465][I] [Memory] 45.4% free - 143140 of 315504 bytes free (psram: 8388608 / 8265460  -> 98.5%)
[3491][I] FFAT totalBytes=10.24 Mb
[3517][I] FFAT usedBytes=0.10 Mb
[3542][I] FFAT Avaliable=99.04%
[3569][I] [Memory] memmory_difference msg="Loaded bulk file" heap=-4368 psram=0
[3595][I] [Memory] 43.9% free - 138356 of 315504 bytes free (psram: 8388608 / 8265460  -> 98.5%)
[4281][I] Completed
[4306][I] [Memory] memmory_difference msg="Frame repo" heap=12 psram=-4888
[4332][I] [Memory] 43.9% free - 138380 of 315504 bytes free (psram: 8388608 / 8260572  -> 98.5%)
[4386][I] [Memory] memmory_difference msg="Lua" heap=-13964 psram=-77992
[4412][I] [Memory] 39.4% free - 124372 of 315504 bytes free (psram: 8388608 / 8182580  -> 97.5%)
[4736][I] [Memory] memmory_difference msg="init.
```

# O que vai no cartão SD

Se você está com preguiça, copie todo o repositório do GitHub diretamente para o cartão SD.

Se você quer as coisas organizadas, isto é o que você precisa:

* Pasta `scripts`
* Pasta `expressions`
* Pasta `lualib`
* Todos os arquivos `.lua` na raiz
* Todos os arquivos `.json` na raiz
* Todos os arquivos `.html` na raiz

![](./flashing-guide-15.png)