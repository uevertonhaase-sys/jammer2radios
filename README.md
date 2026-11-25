nRF24 Jammer (ESP32)
Warning

Aviso legal. Este projeto emite portadora contínua na faixa de 2,4 GHz, o que caracteriza interferência intencional de radiofrequência. No Brasil, o uso de bloqueador de sinais de radiocomunicações (BSR) é restrito a órgãos públicos específicos, conforme a Resolução Anatel nº 760/2023, e a operação clandestina de telecomunicações é tipificada no art. 183 da Lei nº 9.472/1997. Nos EUA, a prática viola o 47 U.S.C. § 333 e as regras da FCC. O uso só é legal com autorização expressa do órgão regulador e em ambiente controlado (por exemplo, câmara blindada / gaiola de Faraday). A responsabilidade pelo uso indevido é integralmente de quem opera o hardware.

Firmware de estudo de radiofrequência para ESP32 + nRF24L01+ PA+LNA em modo de portadora contínua (constant carrier / test carrier) com varredura de canal na faixa ISM de 2,4 GHz. O repositório mantém duas variantes do sketch: uma com rádio único e outra com dois rádios em barramentos SPI independentes.

Visão geral
Arquivo	Rádios	Barramentos	Descrição
single.txt	1	HSPI	Emissão de portadora com varredura triangular de canal.
dual.txt	2	HSPI + VSPI	Dois rádios defasados em 10 canais, cada um em seu próprio SPI.
Ambos os sketches fazem a mesma coisa em essência: desligam os rádios internos do ESP32 (WiFi e Bluetooth), inicializam o(s) nRF24L01+ e passam a alternar o canal de RF em um laço contínuo, mantendo a portadora ativa durante todo o tempo.

Hardware necessário
1x ESP32 (DevKit, WROOM-32 ou similar, com alimentação estável em 3,3 V)
1x nRF24L01+ PA+LNA (versão com antena SMA externa) para single.txt
2x nRF24L01+ PA+LNA para dual.txt
Capacitor eletrolítico de 10 µF a 100 µF entre VCC e GND de cada módulo nRF24, o mais próximo possível do conector (obrigatório em módulos PA+LNA)
Fonte externa de 3,3 V com capacidade de pelo menos 500 mA (LDO tipo AMS1117-3.3), especialmente no setup com dois módulos
Fios curtos (menos de 10 cm) para as linhas de SPI
O regulador 3,3 V da placa ESP32 não é adequado para alimentar dois módulos PA+LNA em transmissão contínua. Picos de corrente acima de 200 mA por módulo causam resets e falhas de inicialização.

Pinagem
Os módulos nRF24L01+ operam em 3,3 V. A lógica do ESP32 também é 3,3 V, então não use level shifter nas linhas SPI, mas nunca alimente o módulo com 5 V.

single.txt (HSPI)
Sinal nRF24	GPIO ESP32	Observação
CE	16	RF24 radio(16, 15, 10000000)
CSN	15	
SCK	14	padrão do HSPI
MISO	12	padrão do HSPI
MOSI	13	padrão do HSPI
VCC	3V3	com capacitor 10 µF
GND	GND	
dual.txt (HSPI + VSPI)
Sinal	Rádio #1 (HSPI)	Rádio #2 (VSPI)
CE	16	22
CSN	15	21
SCK	14	18
MISO	12	19
MOSI	13	23
VCC / GND	3V3 / GND	3V3 / GND
A linha de alimentação dos dois módulos deve ser comum, com capacitores individuais em cada módulo. Não compartilhe apenas um capacitor.

Como funciona
Desligamento dos rádios internos. disableEspRadios() chama esp_bt_controller_deinit(), esp_wifi_stop() e esp_wifi_deinit() para evitar que o WiFi/Bluetooth do ESP32 ocupem a mesma faixa e consumam corrente durante a operação.

Inicialização do SPI. hspi.begin() (e vspi.begin() no dual) configuram os barramentos com os pinos padrão do ESP32. O objeto RF24 é construído com velocidade de SPI de 10 MHz.

Configuração do rádio. setPALevel(RF24_PA_MAX, true) (potência máxima com LNA), setDataRate(RF24_2MBPS), setPayloadSize(5) e setAddressWidth(3). As duas últimas não afetam o modo portadora, mas ficam definidas para manter o registro do rádio em um estado conhecido.

Portadora contínua. startConstCarrier(RF24_PA_MAX, canal) coloca o nRF24L01+ no modo de teste de portadora, em que o rádio emite uma onda não modulada no canal escolhido.

Varredura de canal. O laço principal altera o canal a cada 200 µs em um movimento triangular:

2 → 4 → 6 → ... → 78 → 79 → 78 → ... → 4 → 2 → 4 → ...
O passo é CHANNEL_STEP = 2, com limites CHANNEL_MIN = 2 e CHANNEL_MAX = 79.

Defasagem (apenas dual.txt). O rádio #2 opera CHANNEL_OFFSET = 10 canais à frente do rádio #1, com wrap para o início da faixa quando passa de 79. Assim os dois módulos nunca transmitem exatamente na mesma frequência.

Mapa de frequências
No nRF24L01+ o canal n corresponde a 2400 + n MHz:

Canal	Frequência
2	2402 MHz
45	2445 MHz
79	2479 MHz
A faixa varrida (2 a 79, passo 2) cobre de 2402 a 2479 MHz, dentro da faixa ISM de 2,4 GHz, passando pelos canais 1 a 13 usados por WiFi.

Estrutura do repositório
nrf24 Jammer/
├── README.md
├── dual.txt      # sketch ESP32 com dois rádios (HSPI + VSPI)
└── single.txt    # sketch ESP32 com um rádio (HSPI)
Os arquivos estão com extensão .txt para versionamento simples no GitHub. A IDE do Arduino exige a extensão .ino, veja a seção abaixo.

Compilação e gravação
Instale o Arduino IDE 2.x e o pacote de placas esp32 (Espressif) pelo Gerenciador de Placas.

No Gerenciador de Bibliotecas, instale RF24 de TMRh20 (versão 1.4.x ou mais recente; é necessário que a função startConstCarrier() exista na versão instalada).

Copie o sketch desejado para uma pasta com o mesmo nome e renomeie para .ino:

mkdir nrf24_single
Copy-Item single.txt nrf24_single\nrf24_single.ino
Ou, para a versão com dois rádios:

mkdir nrf24_dual
Copy-Item dual.txt nrf24_dual\nrf24_dual.ino
Abra o .ino na Arduino IDE e selecione a placa ESP32 Dev Module (ou o modelo correspondente ao seu hardware).

Conecte o ESP32 por USB, escolha a porta correta e faça o upload.

Abra o Monitor Serial em 115200 baud.

Se a IDE reclamar de esp_bt.h ou esp_wifi.h, confirme que o pacote de placas ESP32 está instalado; esses headers fazem parte do core, não de bibliotecas externas.

Saída serial esperada
RF24 started
SPI Speedz     = 10 Mhz
STATUS         = 0x0e RX_DR=0 TX_DS=0 MAX_RT=0 RX_P_NO=7 TX_FULL=0
RX_ADDR_P0-1   = 0x6565656565 0x6565656565
CHANNEL        = 45
Data Rate      = 2 MBPS
PA Power       = PA_MAX
...
Na variante dual.txt a saída é precedida por RF24 #1 and #2 started e cada rádio tem seu bloco de detalhes impresso separadamente. Em caso de falha de inicialização o firmware imprime RF24 init failed (ou RF24 #1/#2 init failed) e entra em um laço infinito com delay(1000).

Solução de problemas
Sintoma	Causa provável
RF24 init failed	Fiação de CE/CSN/SPI incorreta, módulo sem alimentação ou sem GND comum
Falha só no rádio #2 (dual.txt)	Pinos do VSPI trocados, ou CSN/CE em GPIO de boot strap
ESP32 reinicia ao iniciar a transmissão	Alimentação insuficiente; adicione capacitor e fonte externa de 3,3 V
Módulo funciona por alguns segundos e para	Queda de tensão no pico de transmissão (PA+LNA)
Alcance muito curto	Antena SMA não conectada, módulo sem LNA ou alimentação abaixo de 3,0 V
Monitor Serial vazio	Baud rate diferente de 115200 ou porta errada
Notas técnicas e limitações
Não há seleção de canal fixo. O firmware varre a faixa continuamente; qualquer ajuste exige alterar as constantes no topo do sketch e regravar o ESP32.
A varredura é determinística (triangular e com passo fixo), não aleatória.
dual.txt usa dois barramentos SPI independentes. Os dois módulos compartilham apenas alimentação e GND; CE e CSN são exclusivos de cada rádio.
Cuidado com os pinos de boot. GPIO 15 (CSN do rádio #1) tem pull-up interno e participa do processo de boot do ESP32. Se a placa não iniciar com o módulo conectado, revise essa ligação.
O modo portadora é um recurso de teste do RF24. Ele mantém o transmissor ligado ininterruptamente, o que aquece o módulo e aumenta bastante o consumo de corrente.
Nenhuma persistência ou telemetria. Não há gravação em NVS, display ou interface de controle; o estado é apenas o que está em RAM.
Licença
Nenhuma licença está definida neste repositório. Sem uma licença explícita, os arquivos permanecem sob direitos autorais reservados. Considere adicionar um arquivo LICENSE caso queira permitir reuso.
