1. [Funções Lua](#funções-lua)  
2. [Constantes Lua](#constantes-lua)  
3. [Comportamentos de LED](#comportamentos-de-led)  
4. [Interface Bluetooth](#interface-bluetooth)  

# Funções Lua  

- [Energia](#energia)  
- [Sistema](#sistema)  
- [Sensores](#sensores)  
- [Desenho no Painel de LED](#desenho-no-painel-de-led)  
- [Funções de Dicionário](#funções-de-dicionário)  
- [Controle de Servo](#controle-de-servo)  
- [Desenho](#desenho)  
- [Decodificação de Imagem](#decodificação-de-imagem)  
- [Tela Interna](#tela-interna)  
- [Controle Remoto](#controle-remoto)  
- [Fitas de LED](#Fitas-de-led-1)  
- [Núcleo Arduino](#núcleo-arduino)  

## Energia  
#### `waitForPower()`  
Força o processo a esperar até que a tensão da bateria ou USB esteja acima de um limite.  
- **Retorna**: `nil`  

#### `panelPowerOn()`  
Liga o regulador, alimentando o painel e a saída de 5V.  
- **Retorna**: `nil`  

#### `panelPowerOff()`  
Desliga o regulador.  
- **Retorna**: `nil`  

#### `getBatteryVoltage()`  
Retorna a tensão da bateria/USB em volts.  
- **Retorna**: `float`  

#### `getAvgBatteryVoltage()`  
Retorna a tensão média da bateria/USB em volts.  
- **Retorna**: `float`  

#### `setAutoCheckPowerLevel(bool)`  
Define a verificação automática de energia. Se a energia ficar abaixo do `setVoltageStopThreshold` e o hardware estiver configurado usando um conversor buck, a tensão é cortada.  
- **Retorna**: `nil`  

#### `setVoltageStopThreshold(voltage)`  
Define o limite de tensão no qual o sistema desligará automaticamente para evitar danos por baixa tensão da bateria.  
- **Parâmetros**:  
  - `voltage` (int): O limite de tensão em volts.  
- **Retorna**: `nil`  

#### `setVoltageStartThreshold(voltage)`  
Define o limite de tensão no qual o sistema ligará automaticamente após ser desligado por baixa tensão da bateria.  
- **Parâmetros**:  
  - `voltage` (int): O limite de tensão em volts.  
- **Retorna**: `nil`  

#### `setPoweringMode(mode)`  
Define o modo de alimentação do sistema. O modo pode ser `POWER_MODE_USB_5V`, `POWER_MODE_USB_9V` ou `POWER_MODE_BATTERY`.  
- **Parâmetros**:  
  - `mode` (int): O modo de alimentação a ser definido.  
- **Retorna**: `nil`  

## Sistema  

### `getFreePsram()`  
Retorna a quantidade de PSRAM (Pseudo Static RAM) livre em bytes.  
- **Retorna**: `int` (A quantidade de PSRAM livre em bytes).  

### `getFreeHeap()`  
Retorna a quantidade de memória heap livre em bytes.  
- **Retorna**: `int` (A quantidade de memória heap livre em bytes).  

#### `restart()`  
Reinicia o microcontrolador ESP32.  
- **Retorna**: `nil`  

#### `getResetReason()`  
Retorna o motivo do último reinício.  
- **Retorna**: `int` (Uma das constantes ESP_RST_*).  

#### `setBrownoutDetection(bool enable)`  
Habilita ou desabilita a detecção de brownout.  
- **Parâmetros**:  
  - `enable` (bool): `true` para habilitar, `false` para desabilitar.  
- **Retorna**: `nil`  

#### `listFiles(path, recursive)`  
Lista arquivos em um diretório.  
- **Parâmetros**:  
  - `path` (string): Caminho do diretório.  
  - `recursive` (bool): Se deve listar recursivamente.  
- **Retorna**: `array de string`  

#### `moveFile(src, dest)`  
Move/renomeia um arquivo.  
- **Parâmetros**:  
  - `src` (string): Caminho de origem.  
  - `dest` (string): Caminho de destino.  
- **Retorna**: `bool` (Sucesso).  

#### `removeFile(path)`  
Exclui um arquivo.  
- **Parâmetros**:  
  - `path` (string): Caminho do arquivo.  
- **Retorna**: `bool` (Sucesso).  

#### `deleteBulkFile()`  
Exclui o arquivo frames.bulk.  
- **Retorna**: `nil`  

## Sensores  
#### `readButtonStatus(int)`  
Lê o status do botão do controle remoto. Há um total de 5 botões. Os estados possíveis são:  
```  
BUTTON_RELEASED = 0  
BUTTON_JUST_PRESSED = 1  
BUTTON_PRESSED = 2  
BUTTON_JUST_RELEASED = 3  
```  
- **Parâmetros**:  
  - `button` (int): O ID do botão a ser verificado.  
- **Retorna**: `int` (O estado do botão).  

#### `readAccelerometerX([device])`  
Retorna o valor bruto do sensor acelerômetro do controle remoto, apenas no eixo X.  
- **Parâmetros**:  
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.  
- **Retorna**: `float` (O valor bruto do acelerômetro no eixo X).  

#### `readAccelerometerY([device])`  
Retorna o valor bruto do sensor acelerômetro do controle remoto, apenas no eixo Y.  
- **Parâmetros**:  
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.  
- **Retorna**: `float` (O valor bruto do acelerômetro no eixo Y).  

#### `readAccelerometerZ([device])`  
Retorna o valor bruto do sensor acelerômetro do controle remoto, apenas no eixo Z.  
- **Parâmetros**:  
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.  
- **Retorna**: `float` (O valor bruto do acelerômetro no eixo Z).  

#### `readGyroX([device])`  
Retorna o valor bruto do sensor giroscópio do controle remoto, apenas no eixo X.  
- **Parâmetros**:  
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.  
- **Retorna**: `int` (O valor bruto do giroscópio no eixo X).  

#### `readGyroY([device])`  
Retorna o valor bruto do sensor giroscópio do controle remoto, apenas no eixo Y.  
- **Parâmetros**:  
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.  
- **Retorna**: `int` (O valor bruto do giroscópio no eixo Y).  

#### `readGyroZ([device])`  
Retorna o valor bruto do sensor giroscópio do controle remoto, apenas no eixo Z.  
- **Parâmetros**:  
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.  
- **Retorna**: `int` (O valor bruto do giroscópio no eixo Z).  

#### `hasLidar()`  
Verifica a presença de um lidar.  
- **Retorna**: `bool` (`true` se um lidar estiver presente, caso contrário `false`).  

#### `readLidar()`  
Retorna a distância em mm do lidar.  
- **Retorna**: `int` (A distância em milímetros).  

#### `getInternalButtonStatus()`  
Retorna o status do botão interno.  
- **Retorna**: `int` (1 para pressionado e 0 para solto).  

## Desenho no Painel de LED  

#### `flipPanelBuffer()`  
Os painéis têm dois buffers. Um para desenho e outro para renderização. Quando o desenho é concluído em um buffer, esta chamada os alterna para que você possa desenhar no outro.  
- **Retorna**: `nil`  

#### `DrawPixel(x, y, color)`  
Desenha um pixel nas coordenadas especificadas com a cor fornecida.  
- **Parâmetros**:  
  - `x` (int): A coordenada X.  
  - `y` (int): A coordenada Y.  
  - `color` (int): A cor para desenhar (use `color565` ou `color444` para gerar a cor).  
- **Retorna**: `nil`  

#### `drawPanelCircle(x, y, radius, color)`  
Desenha um círculo nas coordenadas especificadas com o raio e cor fornecidos.  
- **Parâmetros**:  
  - `x` (int): A coordenada X do centro.  
  - `y` (int): A coordenada Y do centro.  
  - `radius` (int): O raio do círculo.  
  - `color` (int): A cor para desenhar.  
- **Retorna**: `nil`  

### `drawPanelChar(x, y, c, color, bg, size)`  
Desenha um caractere no painel nas coordenadas especificadas com a cor e fundo fornecidos.  
- **Parâmetros**:  
  - `x` (int): A coordenada X.  
  - `y` (int): A coordenada Y.  
  - `c` (char): O caractere a ser desenhado.  
  - `color` (int): A cor do caractere (use `color565` ou `color444` para gerar a cor).  
  - `bg` (int): A cor de fundo (use `color565` ou `color444` para gerar a cor).  
  - `size` (int): O tamanho do caractere (1 para tamanho normal, valores maiores para escalonamento).  
- **Retorna**: `nil`  

#### `drawPanelFillCircle(x, y, radius, color)`  
Desenha um círculo preenchido nas coordenadas especificadas com o raio e cor fornecidos.  
- **Parâmetros**:  
  - `x` (int): A coordenada X do centro.  
  - `y` (int): A coordenada Y do centro.  
  - `radius` (int): O raio do círculo.  
  - `color` (int): A cor para preencher.  
- **Retorna**: `nil`  

#### `drawPanelLine(x0, y0, x1, y1, color)`  
Desenha uma linha entre dois pontos com a cor especificada.  
- **Parâmetros**:  
  - `x0` (int): A coordenada X do ponto inicial.  
  - `y0` (int): A coordenada Y do ponto inicial.  
  - `x1` (int): A coordenada X do ponto final.  
  - `y1` (int): A coordenada Y do ponto final.  
  - `color` (int): A cor para desenhar.  
- **Retorna**: `nil`  

#### `drawPanelRect(x, y, width, height, color)`  
Desenha um retângulo na posição especificada com a largura, altura e cor fornecidos.  
- **Parâmetros**:  
  - `x` (int): A coordenada X do canto superior esquerdo.  
  - `y` (int): A coordenada Y do canto superior esquerdo.  
  - `width` (int): A largura do retângulo.  
  - `height` (int): A altura do retângulo.  
  - `color` (int): A cor para desenhar.  
- **Retorna**: `nil`  

#### `drawPanelFillRect(x, y, width, height, color)`  
Desenha um retângulo preenchido na posição especificada com a largura, altura e cor fornecidos.  
- **Parâmetros**:  
  - `x` (int): A coordenada X do canto superior esquerdo.  
  - `y` (int): A coordenada Y do canto superior esquerdo.  
  - `width` (int): A largura do retângulo.  
  - `height` (int): A altura do retângulo.  
  - `color` (int): A cor para preencher.  
- **Retorna**: `nil`  

#### `color444(r, g, b)`  
Converte valores de cor RGB888 para o formato RGB444.  
- **Parâmetros**:  
  - `r` (int): O componente vermelho (0-255).  
  - `g` (int): O componente verde (0-255).  
  - `b` (int): O componente azul (0-255).  
- **Retorna**: `int` (O valor da cor RGB444).  

#### `color565(r, g, b)`  
Converte valores de cor RGB888 para o formato RGB565.  
- **Parâmetros**:  
  - `r` (int): O componente vermelho (0-255).  
  - `g` (int): O componente verde (0-255).  
  - `b` (int): O componente azul (0-255).  
- **Retorna**: `int` (O valor da cor RGB565).  

#### `clearPanelBuffer()`  
Limpa o buffer da tela.  
- **Retorna**: `nil`  

#### `drawPanelFace(int)`  
Desenha um rosto específico. Este rosto já deve ter sido carregado anteriormente.  
- **Parâmetros**:  
  - `faceId` (int): O ID do rosto a ser desenhado.  
- **Retorna**: `nil`  

#### `setPanelAnimation(frames int_array, duration, [repeat, [drop, [storage]]])`  
Se o painel estiver no modo gerenciado, isso definirá uma animação para rodar nele.  
- **Parâmetros**:  
  - `frames` (array de int): Os IDs de cada quadro na animação.  
  - `duration` (int): A duração de cada quadro em milissegundos.  
  - `repeat` (int, opcional): O número de vezes que a animação deve repetir. Padrão é `-1` (infinito).  
  - `drop` (bool, opcional): Se `true`, todas as animações empilhadas serão apagadas, e esta será a única animação.  
  - `storage` (int, opcional): Quando definido, você pode consultar este armazenamento através de `getCurrentAnimationStorage()`. Pode ser útil quando as animações compartilham os mesmos quadros.  
- **Retorna**: `nil`  

### `getCurrentAnimationStorage()`  
Retorna o ID do armazenamento de animação atual sendo usado.  
- **Retorna**: `int` (O ID do armazenamento de animação atual).  

#### `setPanelManaged(bomanagedol)`  
Habilita ou desabilita o modo gerenciado. No modo gerenciado, a renderização é tratada assincronamente, e você só precisa definir as animações.  
- **Parâmetros**:  
  - `managed` (bool): `true` para habilitar o modo gerenciado, `false` para desabilitar.  
- **Retorna**: `nil`  

#### `isPanelManaged()`  
Retorna se o painel está no modo gerenciado.  
- **Retorna**: `bool` (`true` se o modo gerenciado estiver habilitado, caso contrário `false`).  

#### `getPanelCurrentFace()`  
Retorna o ID do quadro atual sendo exibido.  
- **Retorna**: `int` (O ID do quadro atual).  

#### `drawPanelCurrentFrame()`  
Desenha o quadro atual.  
- **Retorna**: `nil`  

#### `getAnimationStackSize()`  
Retorna o número de animações atualmente empilhadas.  
- **Retorna**: `int` (O número de animações empilhadas).  

#### `popPanelAnimation()`  
Remove a animação atual da pilha.  
- **Retorna**: `nil`  

#### `setPanelBrightness(brightness)`  
Define o brilho do painel.  
- **Parâmetros**:  
  - `brightness` (int): O nível de brilho (0-255, onde 255 é 100%).  
- **Retorna**: `nil`  

#### `gentlySetPanelBrightness(brightness, rate)`  
Ajusta gradualmente o brilho do painel para o nível especificado na taxa fornecida. Esta função é útil para transições suaves de brilho.  
- **Parâmetros**:  
  - `brightness` (int): O nível de brilho alvo (0-255).  
  - `rate` (int): A velocidade da mudança de brilho (valores maiores significam transições mais lentas).  
- **Retorna**: `nil`  

#### `setSpeakingFrames(frames, frameDuration)`  
Define os quadros a serem exibidos quando o sensor de fala é acionado.  
- **Parâmetros**:  
  - `frames` (array de int): Os IDs dos quadros a serem exibidos.  
  - `frameDuration` (int): A duração de cada quadro em milissegundos.  
- **Retorna**: `nil`  

#### `setRainbowShader(enabled)`  
Habilita ou desabilita o shader de arco-íris, que converte pixels para um padrão de arco-íris.  
- **Parâmetros**:  
  - `enabled` (bool): `true` para habilitar o shader, `false` para desabilitar.  
- **Retorna**: `nil`  

#### `getFrameOffsetByName(name)`  
Retorna o offset do quadro associado aos frames.  
- **Parâmetros**:  
  - `name` (string): O nome dos frames.  
- **Retorna**: `int` (O offset).  

#### `getFrameCountByName(name)`  
Retorna a quantidade de frames que um grupo de frames tem
- **Parâmetros**:  
  - `name` (string): O nome dos frames.  
- **Retorna**: `int` (Quantos frames).  


## Decodificação de Imagem  

### `decodePng(filename)`  
Decodifica um arquivo de imagem PNG do cartão SD e retorna os dados brutos dos pixels.  
- **Parâmetros**:  
  - `filename` (string): O caminho para o arquivo PNG no cartão SD.  
- **Retorna**: `tabela uint16`  

## Funções de Dicionário  
#### `dictGet(key)`  
Obtém um valor do armazenamento persistente de dicionário.  
- **Parâmetros**:  
  - `key` (string): A chave para procurar.  
- **Retorna**: `string` (O valor armazenado).  

#### `dictSet(key, value)`  
Define um valor no armazenamento persistente de dicionário.  
- **Parâmetros**:  
  - `key` (string): A chave para armazenar.  
  - `value` (string): O valor para armazenar.  
- **Retorna**: `nil`  

#### `dictDel(key)`  
Exclui uma chave do armazenamento persistente de dicionário.  
- **Parâmetros**:  
  - `key` (string): A chave para excluir.  
- **Retorna**: `nil`  

#### `dictSave()`  
Salva o dicionário no armazenamento persistente.  
- **Retorna**: `nil`  

#### `dictLoad()`  
Carrega o dicionário do armazenamento persistente.  
- **Retorna**: `nil`  

#### `dictFormat()`  
Formata/limpa o armazenamento do dicionário.  
- **Retorna**: `nil`  

## Controle de Servo  

#### `servoPause(servoId)`  
Pausa o servo especificado.  
- **Parâmetros**:  
  - `servoId` (int): O ID do servo a ser pausado.  
- **Retorna**: `bool` (`true` se o servo foi pausado com sucesso, caso contrário `false`).  

#### `servoResume(servoId)`  
Retoma o servo especificado.  
- **Parâmetros**:  
  - `servoId` (int): O ID do servo a ser retomado.  
- **Retorna**: `bool` (`true` se o servo foi retomado com sucesso, caso contrário `false`).  

#### `servoMove(servoId, angle)`  
Move o servo especificado para o ângulo fornecido.  
- **Parâmetros**:  
  - `servoId` (int): O ID do servo a ser movido.  
  - `angle` (float): O ângulo alvo para o servo.  
- **Retorna**: `bool` (`true` se o servo foi movido com sucesso, caso contrário `false`).  

#### `hasServo()`  
Verifica se o sistema tem um servo conectado.  
- **Retorna**: `bool` (`true` se um servo estiver presente, caso contrário `false`).  

## Tela Interna  

### `oledDrawPixel(x, y, color)`  
Desenha um único pixel na tela OLED nas coordenadas especificadas com a cor fornecida.  
- **Parâmetros**:  
  - `x` (int): A coordenada X.  
  - `y` (int): A coordenada Y.  
  - `color` (int): A cor para desenhar (1 para branco, 0 para preto).  
- **Retorna**: `nil`  

### `oledDrawBottomBar()`  
Desenha a barra inferior.  
- **Retorna**: `nil`  

### `oledSetTextColor(fg[, bg])`  
Define a cor do texto para desenhar na tela OLED.  
- **Parâmetros**:  
  - `fg` (int): A cor do primeiro plano (1 para branco, 0 para preto).  
  - `bg` (int, opcional): A cor do fundo (1 para branco, 0 para preto).  
- **Retorna**: `nil`  

### `oledSetFontSize(size)`  
Define o tamanho do texto, padrão é 1.  
- **Parâmetros**:  
  - `size` (int): Padrão é 1.  
- **Retorna**: `nil`  

#### `oledSetCursor(x, y)`  
Define a posição do cursor na tela OLED.  
- **Parâmetros**:  
  - `x` (int): A coordenada X.  
  - `y` (int): A coordenada Y.  
- **Retorna**: `nil`  

#### `oledFaceToScreen(x, y)`  
Desenha a imagem atual sendo exibida nos painéis externos na tela interna na posição especificada.  
- **Parâmetros**:  
  - `x` (int): A coordenada X.  
  - `y` (int): A coordenada Y.  
- **Retorna**: `nil`  

#### `oledDrawTopBar()`  
Desenha a barra superior com algumas utilidades.  
- **Retorna**: `nil`  

#### `oledClearScreen()`  
Limpa o buffer da tela OLED.  
- **Retorna**: `nil`  

#### `oledDisplay()`  
Envia o buffer para a tela OLED.  
- **Retorna**: `nil`  

#### `oledDrawText(msg string)`  
Desenha texto na posição atual do cursor.  
- **Parâmetros**:  
  - `msg` (string): O texto a ser desenhado.  
- **Retorna**: `nil`  

#### `oledDrawRect(x, y, width, height, color)`  
Desenha um retângulo na tela OLED.  
- **Parâmetros**:  
  - `x` (int): A coordenada X.  
  - `y` (int): A coordenada Y.  
  - `width` (int): A largura do retângulo.  
  - `height` (int): A altura do retângulo.  
  - `color` (int): A cor para desenhar (1 para branco, 0 para preto).  
- **Retorna**: `nil`  

#### `oledCreateIcon(width, height, data)`  
Cria um ícone a partir dos dados fornecidos e retorna seu ID.  
- **Parâmetros**:  
  - `width` (int): A largura do ícone.  
  - `height` (int): A altura do ícone.  
  - `data` (array de int): Os dados binários para o ícone.  
- **Retorna**: `int` (O ID do ícone).  

#### `oledDrawIcon(x, y, iconId)`  
Desenha um ícone na posição especificada.  
- **Parâmetros**:  
  - `x` (int): A coordenada X.  
  - `y` (int): A coordenada Y.  
  - `iconId` (int): O ID do ícone a ser desenhado.  
- **Retorna**: `nil`  

## Fitas de LED  

#### `ledsBegin(led_count, max_brightness])`  
Inicializa a tira de LED com o número especificado de LEDs e brilho máximo opcional.  
- **Parâmetros**:  
  - `led_count` (int): O número de LEDs na tira.  
  - `max_brightness` (int, opcional): O nível máximo de brilho (0-255). Padrão é 128.  
- **Retorna**: `bool` (`true` se bem-sucedido, caso contrário `false`).  

#### `ledsBeginDual(led_count, led_count2, max_brightness])`  
Inicializa duas Fitas de LED individuais.  
- **Parâmetros**:  
  - `led_count` (int): O número de LEDs na primeira tira.  
  - `led_count2` (int): O número de LEDs na segunda tira.  
  - `max_brightness` (int, opcional): O nível máximo de brilho (0-255). Padrão é 128.  
- **Retorna**: `bool` (`true` se bem-sucedido, caso contrário `false`).  

#### `ledsIsManaged()`  
Verifica se os LEDs estão no modo gerenciado.  
- **Retorna**: `bool`.  

#### `ledsGentlySeBrightness(brightness, [rate, [startAmount]])`  
Aumenta lentamente o brilho até um determinado valor.  
- **Parâmetros**:  
  - `brightness` (int): Brilho máximo a ser aumentado.  
  - `rate` (int, opcional): Quantas unidades de brilho aumentar a cada quadro. Padrão é 1.  
  - `startAmount` (int, opcional): Brilho inicial. Padrão é 0.  

#### `ledsStackCurrentBehavior()`  
Armazena o estado atual do comportamento dos LEDs em uma pilha. É útil para comportamentos temporários.  
- **Retorna**: `int`  

#### `ledsPopBehavior()`  
Atualiza o comportamento atual dos LEDs com o comportamento armazenado na pilha.  
- **Retorna**: `int`  

#### `ledsSegmentRange(id int, from int, to int)`  
Define um segmento da tira de LED para controle independente.  
- **Parâmetros**:  
  - `id` (int): O ID do segmento (0-15).  
  - `from` (int): O índice do LED inicial.  
  - `to` (int): O índice do LED final.  
- **Retorna**: `nil`  

#### `ledsSetManaged(managed)`  
Habilita ou desabilita o modo gerenciado.  
- **Retorna**: `nil`  

#### `ledsSetBrightness(brightness)`  
Define o brilho da tira de LED.  
- **Parâmetros**:  
  - `brightness` (int): O nível de brilho (0-255).  
- **Retorna**: `nil`  

#### `ledsGetBrightness()`  
Retorna o brilho atual da tira de LED.  
- **Retorna**: `int`  

#### `ledsDisplay()`  
Exibe as alterações nos LEDs na tira. Para ser usado quando o modo gerenciado estiver desabilitado.  
- **Retorna**: `int`  

#### `ledsSegmentBehavior(id, behavior, [parameter, parameter2, parameter3, parameter4])`  
Define o comportamento para um segmento específico de LED.  
- **Parâmetros**:  
  - `id` (int): O ID do segmento (0-15).  
  - `behavior` (int): O comportamento a ser definido (ex: `BEHAVIOR_PRIDE`, `BEHAVIOR_ROTATE`, etc.).  
  - `parameter` (int, opcional): Parâmetro adicional para o comportamento.  
  - `parameter2` (int, opcional): Parâmetro adicional para o comportamento.  
  - `parameter3` (int, opcional): Parâmetro adicional para o comportamento.  
  - `parameter4` (int, opcional): Parâmetro adicional para o comportamento.  
- **Retorna**: `nil`  

#### `ledsSegmentTweenBehavior(id, behavior, [parameter, parameter2, parameter3, parameter4])`  
Define um comportamento de transição suave para um segmento específico de LED.  
- **Parâmetros**:  
  - `id` (int): O ID do segmento (0-15).  
  - `behavior` (int): O comportamento para transição (ex: `BEHAVIOR_PRIDE`, `BEHAVIOR_ROTATE`, etc.).  
  - `parameter` (int, opcional): Parâmetro adicional para o comportamento.  
  - `parameter2` (int, opcional): Parâmetro adicional para o comportamento.  
  - `parameter3` (int, opcional): Parâmetro adicional para o comportamento.  
  - `parameter4` (int, opcional): Parâmetro adicional para o comportamento.  
- **Retorna**: `nil`  

#### `ledsSegmentTweenSpeed(id, speed)`  
Define a velocidade de transição para um segmento específico de LED.  
- **Parâmetros**:  
  - `id` (int): O ID do segmento (0-15).  
  - `speed` (int): A velocidade de transição (valores maiores significam transições mais lentas).  
- **Retorna**: `nil`  

#### `ledsSetColor(id, r, g, b)`  
Define a cor de um LED individual. Idealmente, não deve ser usado no modo gerenciado ou quando o comportamento é definido como nenhum.  
- **Parâmetros**:  
  - `id` (int): LED individual.  
  - `r` (int): Valor de vermelho (0-255).  
  - `g` (int): Valor de verde (0-255).  
  - `b` (int): Valor de azul (0-255).  
- **Retorna**: `nil`  

#### `ledsSegmentColor(id, r, g, b)`  
Define a cor de um grupo de LEDs. Idealmente, não deve ser usado no modo gerenciado ou quando o comportamento é definido como nenhum.  
- **Parâmetros**:  
  - `id` (int): Segmento de LED.  
  - `r` (int): Valor de vermelho (0-255).  
  - `g` (int): Valor de verde (0-255).  
  - `b` (int): Valor de azul (0-255).  
- **Retorna**: `nil`  

## Núcleo Arduino  

#### `tone(frequency)`  
Gera um tom no pino especificado.  
- **Parâmetros**:  
  - `frequency` (int): Frequência do tom em Hz.  
- **Retorna**: `nil`  

#### `toneDuration(frequency, duration)`  
Gera um tom com duração.  
- **Parâmetros**:  
  - `frequency` (int): Frequência do tom em Hz.  
  - `duration` (int): Duração em milissegundos.  
- **Retorna**: `nil`  

#### `noTone()`  
Para a geração de tom.  
- **Retorna**: `nil`  

#### `millis()`  
Retorna o número de milissegundos desde que o sistema começou a rodar.  
- **Retorna**: `int` (O número de milissegundos).  

#### `delayMicroseconds(us)`  
Atrasa a execução pelo número especificado de microssegundos.  
- **Parâmetros**:  
  - `us` (int): O número de microssegundos para atrasar.  
- **Retorna**: `nil`  

#### `delay(ms)`  
Atrasa a execução pelo número especificado de milissegundos.  
- **Parâmetros**:  
  - `ms` (int): O número de milissegundos para atrasar.  
- **Retorna**: `nil`  

#### `digitalWrite(pin, value)`  
Escreve um valor digital em um pino.  
- **Parâmetros**:  
  - `pin` (int): O número do pino.  
  - `value` (int): O valor a ser escrito (`HIGH` ou `LOW`).  
- **Retorna**: `nil`  

#### `analogRead(pin)`  
Lê um valor analógico de um pino.  
- **Parâmetros**:  
  - `pin` (int): O número do pino.  
- **Retorna**: `int` (O valor analógico).  

#### `pinMode(pin, mode)`  
Define o modo de um pino (ex: `INPUT`, `OUTPUT`).  
- **Parâmetros**:  
  - `pin` (int): O número do pino.  
  - `mode` (int): O modo a ser definido.  
- **Retorna**: `nil`  

#### `digitalRead(pin)`  
Lê um valor digital de um pino.  
- **Parâmetros**:  
  - `pin` (int): O número do pino.  
- **Retorna**: `int` (O valor digital, `HIGH` ou `LOW`).  

### Comunicação Serial  
SerialIo significa usar os pinos io1 e io2 como tx e rx.  
#### `serialWriteString(data: string)`  
Escreve uma string na porta serial primária.  
- **Parâmetros**:  
  - `data` (string): A string a ser escrita na porta serial.  
- **Retorna**: `nil`  

#### `serialIoWriteString(data: string)`  
Escreve uma string na porta serial secundária.  
- **Parâmetros**:  
  - `data` (string): A string a ser escrita na porta serial secundária.  
- **Retorna**: `nil`  

#### `serialIoAvailableForWrite()`  
Retorna o número de bytes disponíveis para escrita na porta serial secundária.  
- **Retorna**: `int` (O número de bytes disponíveis para escrita).  

#### `serialAvailableForWrite()`  
Retorna o número de bytes disponíveis para escrita na porta serial primária.  
- **Retorna**: `int` (O número de bytes disponíveis para escrita).  

### Comunicação I2C  

#### `wireAvailable()`  
Retorna o número de bytes disponíveis para leitura no barramento I2C.  
- **Retorna**: `int` (O número de bytes disponíveis para leitura).  

#### `wireBegin(addr: uint8_t)`  
Inicializa o barramento I2C com o endereço especificado.  
- **Parâmetros**:  
  - `addr` (uint8_t): O endereço I2C do dispositivo.  
- **Retorna**: `bool` (`true` se bem-sucedido, caso contrário `false`).  

#### `wireFlush()`  
Limpa o buffer I2C.  
- **Retorna**: `nil`  

#### `wireBeginTransmission(addr: uint8_t)`  
Inicia uma transmissão para o endereço I2C especificado.  
- **Parâmetros**:  
  - `addr` (uint8_t): O endereço I2C do dispositivo.  
- **Retorna**: `nil`  

#### `wireEndTransmission(sendStop: bool)`  
Finaliza a transmissão I2C.  
- **Parâmetros**:  
  - `sendStop` (bool): Se `true`, uma condição de parada é enviada após a transmissão.  
- **Retorna**: `uint8_t` (O status da transmissão).  

#### `wireRead()`  
Lê um byte do barramento I2C.  
- **Retorna**: `int` (O byte lido do barramento I2C).  

#### `wireReadBytes(length: int)`  
Lê um número especificado de bytes do barramento I2C.  
- **Parâmetros**:  
  - `length` (int): O número de bytes a serem lidos.  
- **Retorna**: `std::vector<uint8_t>` (Um vetor contendo os bytes lidos).  

#### `wireRequestFrom(address: uint16_t, size: size_t, sendStop: bool)`  
Solicita dados de um endereço I2C especificado.  
- **Parâmetros**:  
  - `address` (uint16_t): O endereço I2C do dispositivo.  
  - `size` (size_t): O número de bytes a serem solicitados.  
  - `sendStop` (bool): Se `true`, uma condição de parada é enviada após a solicitação.  
- **Retorna**: `uint8_t` (O número de bytes recebidos).  

#### `wirePeek()`  
Espia o próximo byte no buffer I2C sem removê-lo.  
- **Retorna**: `int` (O próximo byte no buffer).  

#### `wireParseFloat()`  
Analisa um float do buffer I2C.  
- **Retorna**: `float` (O valor float analisado).  

#### `wireParseInt()`  
Analisa um inteiro do buffer I2C.  
- **Retorna**: `int` (O valor inteiro analisado).  

#### `wireSetTimeout(timeout: uint32_t)`  
Define o tempo limite para operações I2C.  
- **Parâmetros**:  
  - `timeout` (uint32_t): O tempo limite em milissegundos.  
- **Retorna**: `nil`  

#### `wireGetTimeout()`  
Retorna o tempo limite atual para operações I2C.  
- **Retorna**: `uint32_t` (O tempo limite em milissegundos).  

### Serial Secundária (IO1/IO2)  

#### `beginSerialIo(baud)`  
Inicializa a porta serial secundária (pinos IO1/IO2).  
- **Parâmetros**:  
  - `baud` (int, opcional): Taxa de transmissão (padrão: 115200).  
- **Retorna**: `nil`  

#### `serialIoAvailable()`  
Retorna o número de bytes disponíveis para leitura na porta serial secundária.  
- **Retorna**: `int` (Número de bytes disponíveis).  

#### `serialIoRead()`  
Lê um byte da porta serial secundária.  
- **Retorna**: `int` (O byte lido, ou -1 se nenhum disponível).  

#### `serialIoReadStringUntil(terminator)`  
Lê caracteres da porta serial secundária até que o terminador seja encontrado.  
- **Parâmetros**:  
  - `terminator` (char, opcional): O caractere terminador (padrão: '\n').  
- **Retorna**: `string` (A string lida).  

#### `serialIoAvailableForWrite()`  
Retorna o número de bytes que podem ser escritos sem bloqueio na serial secundária.  
- **Retorna**: `int` (Espaço disponível no buffer de saída).  

#### `serialIoWrite(data)`  
Escreve um único byte na porta serial secundária.  
- **Parâmetros**:  
  - `data` (int): O byte a ser escrito (0-255).  
- **Retorna**: `int` (Número de bytes escritos).  

#### `serialIoWriteString(data)`  
Escreve uma string na porta serial secundária.  
- **Parâmetros**:  
  - `data` (string): A string a ser escrita.  
- **Retorna**: `int` (Número de bytes escritos).  

### Serial Primária (USB/Console)  

#### `serialAvailable()`  
Retorna o número de bytes disponíveis para leitura na porta serial primária.  
- **Retorna**: `int` (Número de bytes disponíveis).  

#### `serialRead()`  
Lê um byte da porta serial primária.  
- **Retorna**: `int` (O byte lido, ou -1 se nenhum disponível).  

#### `serialReadStringUntil(terminator)`  
Lê caracteres da porta serial primária até que o terminador seja encontrado.  
- **Parâmetros**:  
  - `terminator` (char, opcional): O caractere terminador (padrão: '\n').  
- **Retorna**: `string` (A string lida).  

#### `serialAvailableForWrite()`  
Retorna o número de bytes que podem ser escritos sem bloqueio.  
- **Retorna**: `int` (Espaço disponível no buffer de saída).  

#### `serialWrite(data)`  
Escreve um único byte na porta serial primária.  
- **Parâmetros**:  
  - `data` (int): O byte a ser escrito (0-255).  
- **Retorna**: `int` (Número de bytes escritos).  

#### `serialWriteString(data)`  
Escreve uma string na porta serial primária.  
- **Parâmetros**:  
  - `data` (string): A string a ser escrita.  
- **Retorna**: `int` (Número de bytes escritos).  

### Comunicação I2C  

#### `wireBegin(address)`  
Inicializa a comunicação I2C como mestre ou escravo.  
- **Parâmetros**:  
  - `address` (uint8_t): Endereço do escravo de 7 bits (0 para modo mestre).  
- **Retorna**: `bool` (`true` se bem-sucedido).  

#### `wireAvailable()`  
Retorna o número de bytes disponíveis para leitura.  
- **Retorna**: `int` (Número de bytes disponíveis).  

#### `wireBeginTransmission(address)`  
Inicia uma transmissão para o dispositivo I2C especificado.  
- **Parâmetros**:  
  - `address` (uint8_t): Endereço do dispositivo de 7 bits.  
- **Retorna**: `nil`  

#### `wireEndTransmission([sendStop])`  
Finaliza uma transmissão para o dispositivo I2C.  
- **Parâmetros**:  
  - `sendStop` (bool, opcional): Se deve enviar condição de parada (padrão: true).  
- **Retorna**: `uint8_t` (Código de status da transmissão).  

#### `wireRequestFrom(address, size, [sendStop])`  
Solicita bytes de um dispositivo I2C escravo.  
- **Parâmetros**:  
  - `address` (uint16_t): Endereço do dispositivo de 7 bits.  
  - `size` (size_t): Número de bytes a serem solicitados.  
  - `sendStop` (bool, opcional): Se deve enviar condição de parada (padrão: true).  
- **Retorna**: `uint8_t` (Número de bytes recebidos).  

#### `wireRead()`  
Lê um byte do buffer I2C.  
- **Retorna**: `int` (O byte lido).  

#### `wireReadBytes(length)`  
Lê vários bytes do buffer I2C.  
- **Parâmetros**:  
  - `length` (int): Número de bytes a serem lidos.  
- **Retorna**: `tabela` (Array de bytes lidos).  

#### `wirePeek()`  
Espia o próximo byte no buffer I2C sem removê-lo.  
- **Retorna**: `int` (O próximo byte).  

#### `wireFlush()`  
Limpa o buffer I2C.  
- **Retorna**: `nil`  

#### `wireParseFloat()`  
Analisa um float do buffer I2C.  
- **Retorna**: `float`.  

#### `wireParseInt()`  
Analisa um inteiro do buffer I2C.  
- **Retorna**: `int`.  

#### `wireSetTimeout(timeout)`  
Define o tempo limite para operações I2C.  
- **Parâmetros**:  
  - `timeout` (uint32_t): Tempo limite em milissegundos.  
- **Retorna**: `nil`  

#### `wireGetTimeout()`  
Obtém o tempo limite atual para operações I2C.  
- **Retorna**: `uint32_t` (Tempo limite em milissegundos).  

# Constantes Lua  

- [Relacionadas ao Motor](#relacionadas-ao-motor)  
- [Entrada](#entrada)  
- [Comportamento de LED](#comportamento-de-led)  
- [Pinos](#pinos)  
- [Motivo de Reinício do ESP32](#motivo-de-reinício-do-esp32)  

## Relacionadas ao motor  

- `PANDA_VERSION`: Uma constante de string que representa a versão atual do firmware do Protopanda.  
- `VCC_THRESHOLD_START`: O limite mínimo de tensão necessário para o sistema iniciar.  
- `VCC_THRESHOLD_HALT`: O limite mínimo de tensão abaixo do qual o sistema será interrompido para evitar danos.  
- `OLED_SCREEN_WIDTH`: A largura da tela OLED interna em pixels.  
- `OLED_SCREEN_HEIGHT`: A altura da tela OLED interna em pixels.  
- `PANEL_WIDTH`: A largura do painel HUB75 em pixels.  
- `PANEL_HEIGHT`: A altura do painel HUB75 em pixels.  
- `POWER_MODE_USB_5V`: Modo de alimentação para entrada USB 5V.  
- `POWER_MODE_USB_9V`: Modo de alimentação para entrada USB 9V.  
- `POWER_MODE_BATTERY`: Modo de alimentação para entrada de bateria.  
- `MAX_BLE_BUTTONS`: Número de botões por dispositivo BLE.  
- `MAX_BLE_CLIENTS`: Número máximo de dispositivos BLE conectados.  
- `SERVO_COUNT`: O número de servos.  

## Entrada  

- `BUTTON_RELEASED`: Indica que um botão está no estado solto (não pressionado).  
- `BUTTON_JUST_PRESSED`: Indica que um botão acabou de ser pressionado (transição de solto para pressionado).  
- `BUTTON_PRESSED`: Indica que um botão está atualmente pressionado.  
- `BUTTON_JUST_RELEASED`: Indica que um botão acabou de ser solto (transição de pressionado para solto).  
- `DEVICE_X_BUTTON_LEFT`: Mapeamento do botão esquerdo no dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `DEVICE_X_BUTTON_RIGHT`: Mapeamento do botão direito no dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `DEVICE_X_BUTTON_UP`: Mapeamento do botão para cima no dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `DEVICE_X_BUTTON_DOWN`: Mapeamento do botão para baixo no dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `DEVICE_X_BUTTON_CONFIRM`: Mapeamento do botão de confirmação no dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `DEVICE_X_BUTTON_AUX_A`: Mapeamento do botão de função auxiliar A no dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `DEVICE_X_BUTTON_AUX_B`: Mapeamento do botão de função auxiliar B dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `DEVICE_X_BUTTON_BACK`: Mapeamento do botão de voltar no dispositivo (0 a `MAX_BLE_CLIENTS`).  
- `BUTTON_LEFT`: Igual a DEVICE_0_BUTTON_LEFT.  
- `BUTTON_RIGHT`: Igual a DEVICE_0_BUTTON_RIGHT.  
- `BUTTON_UP`: Igual a DEVICE_0_BUTTON_UP.  
- `BUTTON_CONFIRM`: Igual a DEVICE_0_BUTTON_CONFIRM.  
- `BUTTON_DOWN`: Igual a DEVICE_0_BUTTON_DOWN.  
- `BUTTON_AUX_A`: Igual a DEVICE_0_BUTTON_AUX_A
- `BUTTON_AUX_B`: Igual a DEVICE_0_BUTTON_AUX_B
- `BUTTON_BACK`: Igual a DEVICE_0_BUTTON_BACK

## Comportamento de LED  

- `BEHAVIOR_PRIDE`: Exibe um padrão de cores arco-íris na tira de LED.  
- `BEHAVIOR_ROTATE`: Rotaciona cores ao longo da tira de LED.  
- `BEHAVIOR_RANDOM_COLOR`: Muda aleatoriamente a cor dos LEDs.  
- `BEHAVIOR_FADE_CYCLE`: Percorre cores com um efeito de fade.  
- `BEHAVIOR_ROTATE_FADE_CYCLE`: Combina rotação e efeitos de fade para uma exibição dinâmica.  
- `BEHAVIOR_STATIC_RGB`: Define uma cor RGB estática para os LEDs.  
- `BEHAVIOR_STATIC_HSV`: Define uma cor HSV estática para os LEDs.  
- `BEHAVIOR_RANDOM_BLINK`: Pisca LEDs aleatoriamente.  
- `BEHAVIOR_ROTATE_SINE_V`: Rotaciona cores com um efeito de onda senoidal no componente V (valor) do HSV.  
- `BEHAVIOR_ROTATE_SINE_S`: Rotaciona cores com um efeito de onda senoidal no componente S (saturação) do HSV.  
- `BEHAVIOR_ROTATE_SINE_H`: Rotaciona cores com um efeito de onda senoidal no componente H (matiz) do HSV.  
- `BEHAVIOR_FADE_IN`: Aumenta gradualmente o brilho dos LEDs de desligado para uma cor especificada.  
- `MAX_LED_GROUPS`: Número de grupos de LED disponíveis.  

## Pinos  
- `D1`, `D2`: As entradas/saídas externas, io1 e io2.  
- `HIGH`, `LOW`: Constantes que representam estados alto e baixo para pinos digitais.  
- `INPUT`, `OUTPUT`: Constantes que representam modos de pino.  
- `INPUT_PULLUP`, `INPUT_PULLDOWN`: Constantes que representam modos pull-up e pull-down para pinos de entrada.  
- `ANALOG`: Constante que representa o modo de pino analógico.  
- `OUTPUT_OPEN_DRAIN`, `OPEN_DRAIN`: Constantes que representam o modo de saída em dreno aberto.  
- `PULLDOWN`: Constante que representa o modo pull-down.  

## Motivo de Reinício do ESP32  
- `ESP_RST_UNKNOWN`  
- `ESP_RST_POWERON`  
- `ESP_RST_EXT`  
- `ESP_RST_SW`  
- `ESP_RST_PANIC`  
- `ESP_RST_INT_WDT`  
- `ESP_RST_TASK_WDT`  
- `ESP_RST_WDT`  
- `ESP_RST_DEEPSLEEP`  
- `ESP_RST_BROWNOUT`  
- `ESP_RST_SDIO`  

#### Modos de Energia  
- `BUILT_IN_POWER_MODE`  

# Comportamentos de LED  

* `BEHAVIOR_PRIDE`  
  - **Descrição**: Um efeito arco-íris que percorre as cores em um padrão suave, semelhante à bandeira do orgulho.  
  - **Parâmetros**: Nenhum.  

* `BEHAVIOR_ROTATE`  
  - **Descrição**: Rotaciona uma única cor através do segmento, criando um efeito de luz móvel.  
  - **Parâmetros**:  
    - `1`: Vermelho (0-255).  
    - `2`: Verde (0-255).  
    - `3`: Azul (0-255).  
    - `4`: Velocidade de rotação (atraso em milissegundos).  

* `BEHAVIOR_RANDOM_COLOR`  
  - **Descrição**: Define cada LED no segmento para uma cor aleatória.  
  - **Parâmetros**: Nenhum.  

* `BEHAVIOR_FADE_CYCLE`  
  - **Descrição**: Aumenta e diminui o brilho dos LEDs no segmento em um ciclo.  
  - **Parâmetros**:  
    - `1`: Valor de matiz (0-255).  
    - `2`: Controla a velocidade do ciclo de fade.  
    - `3`: Valor mínimo de brilho.  

* `BEHAVIOR_ROTATE_FADE_CYCLE`  
  - **Descrição**: Combina rotação e fade, criando um efeito de luz móvel com brilho variável.  
  - **Parâmetros**:  
    - `1`: Valor de matiz (0-255).  
    - `2`: Controla a velocidade do ciclo de fade.  
    - `3`: Valor mínimo de brilho.  
    - `4`: Velocidade de rotação (atraso em milissegundos).  

* `BEHAVIOR_STATIC_RGB`  
  - **Descrição**: Define todos os LEDs no segmento para uma cor RGB estática.  
  - **Parâmetros**:  
    - `1`: Valor de vermelho (0-255).  
    - `2`: Valor de verde (0-255).  
    - `3`: Valor de azul (0-255).  

* `BEHAVIOR_STATIC_HSV`  
  - **Descrição**: Define todos os LEDs no segmento para uma cor HSV estática.  
  - **Parâmetros**:  
    - `1`: Valor de matiz (0-255).  
    - `2`: Valor de saturação (0-255).  
    - `3`: Valor de brilho (0-255).  

* `BEHAVIOR_RANDOM_BLINK`  
  - **Descrição**: Pisca LEDs aleatoriamente no segmento com cores e brilhos aleatórios.  
  - **Parâmetros**:  
    - `1`: Valor de matiz base (0-255).  
    - `2`: Faixa de variação de matiz.  
    - `3`: Valor máximo de brilho.  
    - `4`: Atraso entre piscadas (em milissegundos).  

* `BEHAVIOR_ROTATE_SINE_V`  
  - **Descrição**: Rotaciona um efeito de onda senoidal através do segmento, variando o brilho.  
  - **Parâmetros**:  
    - `1`: Valor de matiz base (0-255).  
    - `2`: Valor de brilho base (0-255).  
    - `3`: Controla a velocidade da onda senoidal.  

* `BEHAVIOR_ROTATE_SINE_S`  
  - **Descrição**: Rotaciona um efeito de onda senoidal através do segmento, variando a saturação.  
  - **Parâmetros**:  
    - `1`: Valor de matiz base (0-255).  
    - `2`: Valor de brilho base (0-255).  
    - `3`: Controla a velocidade da onda senoidal.  

* `BEHAVIOR_ROTATE_SINE_H`  
  - **Descrição**: Rotaciona um efeito de onda senoidal através do segmento, variando a matiz.  
  - **Parâmetros**:  
    - `1`: Valor de saturação base (0-255).  
    - `2`: Valor de brilho base (0-255).  
    - `3`: Controla a velocidade da onda senoidal.  

* `BEHAVIOR_FADE_IN`  
  - **Descrição**: Aumenta gradualmente o brilho dos LEDs no segmento para uma cor HSV especificada.  
  - **Parâmetros**:  
    - `1`: Valor de matiz (0-255).  
    - `2`: Valor de saturação (0-255).  
    - `3`: Controla a velocidade do fade-in.  
    - `4`: Atraso entre etapas de fade (em milissegundos).  

* `BEHAVIOR_NONE`  
  - **Descrição**: Nenhum comportamento é aplicado. Os LEDs no segmento permanecerão desligados ou inalterados.  
  - **Parâmetros**: Nenhum.  

# Interface Bluetooth


## Radio and BLE functions
#### `startBLE()`  
Inicia o ambiente de Bluetooth Low Energy (BLE) mas não inicia o rádio.  
- **Retorna**: `bool` (`true` se bem-sucedido, caso contrário `false`).  

#### `startBLERadio()`  
Inicia o rádio
- **Retorna**: `bool` (`true` se bem-sucedido, caso contrário `false`).  

#### `getConnectedRemoteControls()`  
Retorna o número de controles remotos conectados.  
- **Retorna**: `int` (O número de dispositivos conectados).  

#### `isElementIdConnected(id)`  
Verifica se um controle remoto com o ID fornecido está conectado.  
- **Parâmetros**:  
  - `id` (int): O ID do controle remoto.  
- **Retorna**: `bool` (`true` se conectado, caso contrário `false`).  

#### `beginBleScanning()`  
Inicia a varredura de dispositivos BLE.  
- **Retorna**: `nil`  

#### `setLogDiscoveredBleDevices(bool)`  
When true, each scanned device will be saved on the log filew
- **Retorna**: `nil`  

#### `setMaximumControls(count)`  
Define o número máximo de controles remotos conectados.  
- **Parâmetros**:  
  - `count` (int): O número máximo de dispositivos.  
- **Retorna**: `nil`  


#### `getCharacteristicsFromService(connectionId, uuid, refresh)`
Get all characteristics a certain service has
- **Parameters**:
  - `connectionId` (int): connection id.
  - `uuid` (string): uuid.
  - `refresh` (bool): refresh information?
- **Returns**: `nil`

## Handling BLE connections

Exemplo de como aceitamos conexões de um mouse/teclado/joystick:
```lua
  drivers.mouseHandler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
  drivers.mouseHandler:SetOnConnectCallback(drivers.onConnectHID)
  drivers.mouseHandler:SetOnDisconnectCallback(drivers.onDisconnectHID)
  drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
  drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
  drivers.mouseListener:SetCallbackModeStream(false)
```
Primeiro, criamos um manipulador de serviço e passamos um UUID. Esse UUID específico é de um Dispositivo de Interface Humana (HID) genérico. Portanto, qualquer dispositivo que tenha capacidades de mouse/teclado/joystick terá este serviço.
No momento em que o registramos, qualquer dispositivo escaneado que esteja anunciando e tenha este UUID de serviço específico terá uma tentativa de conexão.

A próxima função cria um callback quando o dispositivo é conectado. Aqui está um exemplo do callback:
```lua
function drivers.onConnectHID(connectionId, controllerId, address, name)
    log("Conectado conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."]")
end
```
O connectionId pode ser usado para obter informações sobre o dispositivo. Ele é único para cada conexão.
O controllerId é o ID usado pelo protopanda. O primeiro dispositivo conectado terá o id 0, e o segundo terá 1. Mas no momento em que um deles desconectar, esse ID será liberado, então a próxima conexão poderá reutilizar o mesmo ID.
Address é apenas uma string com o endereço e name é o nome, se presente.

O mesmo vale para `SetOnDisconnectCallback`, mas em vez de endereço e nome, obtemos o motivo da desconexão.
```lua
function drivers.onDisconnectHID(connectionId, controllerId, reason)
    log("Desconectado "..connectionId.." devido a ".. reason)
end
``` 

Uma vez definidos esses callbacks (opcional), podemos anexar um listener a cada uma das características que desejamos:
```lua
drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback) 
drivers.mouseListener:SetRequired(true)
drivers.mouseListener:SetCallbackModeStream(false)
```
Uma vez definida a característica, podemos configurá-la como obrigatória (se não estiver presente, a conexão é encerrada) e um callback se houver uma inscrição no serviço.
O callback segue assim:
```lua
function drivers.onMouseCallback(connectionId, controllerId, data)
-- manipulação
end
```

Lá podemos encontrar o connectionId, o controllerId e um array contendo os dados. Os dados virão como valores de 0 a 255.

## Manipulando funções BLE

### `BleServiceHandler(uuid)`
Cria um objeto manipulador. Tente manter este objeto em um ambiente global ou armazenado. Porque o coletor de lixo (GC) não excluirá a referência C++, mas excluirá a referência Lua.
- **Parâmetros**:
  - `uuid` (string): uuid.
- **Retorna**: `BleServiceHandlerObject`

### `BleServiceHandler::ReadFromCharacteristics(clientId, characteristicsUuid)`
Lê dados de um determinado cliente e de uma determinada característica.
- **Parâmetros**:
  - `clientId` (int): connectionId.
  - `uuid` (string): pode ser o uuid de 16 bits.
- **Retorna**: `array de int`

### `BleServiceHandler::GetServices(clientId)`
Obtém os serviços disponíveis no cliente.
- **Parâmetros**:
  - `clientId` (int): connectionId.
- **Retorna**: `array de string`

### `BleServiceHandler::AddAddressRequired(address)`
Quando definido pela primeira vez, força a conexão a ser aceita apenas se o endereço tiver sido inserido anteriormente.
Exemplo seria:
```lua
Handler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
Handler:AddAddressRequired("AA:BB:CC:DD:EE")
Handler:AddAddressRequired("FF:00:11:22:3")
```
Dessa forma, apenas dispositivos com o serviço específico e esses dois endereços específicos podem conectar.
Se tanto `AddAddressRequired` quanto `AddNameRequired` estiverem definidos. A condição é um "e" (and). Precisa corresponder a qualquer um dos endereços **e** qualquer um dos nomes
- **Parâmetros**:
  - `address` (string): endereço.

### `BleServiceHandler::AddNameRequired(name)`
Quando definido pela primeira vez, força a conexão a ser aceita apenas se o nome tiver sido definido anteriormente
Exemplo seria:
```lua
Handler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
Handler:AddNameRequired("VR-PARK")
Handler:AddNameRequired("Beauty-r1")
```
Dessa forma, apenas dispositivos com o serviço específico e esses dois nomes específicos podem conectar.
Se tanto `AddAddressRequired` quanto `AddNameRequired` estiverem definidos. A condição é um "e" (and). Precisa corresponder a qualquer um dos endereços **e** qualquer um dos nomes
- **Parâmetros**:
  - `name` (string): nome.

### `BleServiceHandler::GetCharacteristics(clientId)`
Obtém as características disponíveis no serviço atual.
- **Parâmetros**:
  - `clientId` (int): connectionId.
- **Retorna**: `array de string`

### `BleServiceHandler::WriteToCharacteristics(message, clientId, characteristics[, reply])`
Escreve uma mensagem para a característica especificada.
- **Parâmetros**:
  - `message` (array de int): bytes da mensagem.
  - `clientId` (int): connectionId.
  - `uuid` (string): pode ser o uuid de 16 bits.
  - `reply` (bool): aguardar resposta.
- **Retorna**: `bool`

### `BleServiceHandler::SetOnDisconnectCallback(callback)`
Define um callback para quando um dispositivo desconectar.
- **Parâmetros**:
  - `callback` (função Lua): callback.

### `BleServiceHandler::SetOnConnectCallback(callback)`
Define um callback para quando um dispositivo conectar.
- **Parâmetros**:
  - `callback` (função Lua): callback.

### `BleServiceHandler::AddCharacteristics(callback)`
Retorna um manipulador para a característica especificada.
- **Parâmetros**:
  - `uuid` (string): pode ser o uuid de 16 bits.
- **Retorna**: `BleCharacteristicsHandlerObject`

### `BleCharacteristicsHandler::SetSubscribeCallback(callback)`
Retorna um manipulador para quando uma mensagem da característica especificada chegar.
- **Parâmetros**:
  - `uuid` (string): pode ser o uuid de 16 bits.

### `BleCharacteristicsHandler::SetCallbackModeStream(stream)`
Quando habilitado, cada mensagem é processada em cada loop. Dessa forma, as mensagens podem se acumular, mas uma mensagem por loop. Quando falso, todas as mensagens são processadas no loop até que a fila esteja vazia.
- **Parâmetros**:
  - `stream` (bool): verdadeiro ou falso

### `BleCharacteristicsHandler::SetRequired(req)`
Se definido como obrigatório, quando o dispositivo se conecta e a característica especificada não está presente, uma desconexão é emitida.
- **Parâmetros**:
  - `req` (bool): verdadeiro ou falso