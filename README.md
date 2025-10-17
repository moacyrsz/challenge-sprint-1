# ⚡Challenge-Pulse⚡🔋🚗🛺

## ⚡Sistema IoT de Monitoramento da Saúde da Bateria para Estações de Carga de Veículos Elétricos

Este é um sistema IoT desenvolvido para monitorar a saúde das baterias em estações de recarga de veículos elétricos (EVs).
O projeto utiliza o microcontrolador **ESP32** para coletar dados ambientais e da bateria, enviando-os em tempo real via **MQTT** para o **Node-RED**, onde são visualizados em um painel interativo com **alertas automáticos via Telegram**.

---

## ⚡Recursos Principais

* Monitoramento de **temperatura ambiente**, **umidade**, **temperatura da bateria**, **tensão** e **corrente**.
* **Alertas visuais e sonoros** no ESP32 quando há condições fora da faixa ideal.
* **Painel Node-RED** com indicadores e gráficos em tempo real.
* **Envio automático de notificações no Telegram** quando o status não é “Ideal”.
* Comunicação por **protocolo MQTT** via broker público.

---

## ⚡Componentes Utilizados

| Componente                 | Função                                                   |
| -------------------------- | -------------------------------------------------------- |
| **ESP32**                  | Microcontrolador principal para leitura e envio de dados |
| **DHT22**                  | Sensor de temperatura e umidade ambiente                 |
| **NTC Termistor**          | Sensor de temperatura da bateria                         |
| **Sensores Analógicos**    | Leitura de tensão e corrente                             |
| **Display OLED (SSD1306)** | Exibição de dados e alertas                              |
| **Relé, Buzzer e LEDs**    | Sinalização visual e sonora de alertas                   |

---

## ⚡🛠️ Instruções de Configuração

### 🔌 Requisitos de Hardware

1. ESP32
2. Sensor DHT22
3. Termistor NTC
4. Sensores analógicos de tensão e corrente
5. Display OLED SSD1306
6. Módulo Relé
7. Buzzer
8. LEDs (Verde, Amarelo e Vermelho)

---

### 💻 Requisitos de Software

* **PlatformIO + Wokwi (VSCode)** → para simular e compilar o código ESP32
* **Node-RED** → para criar o dashboard e processar os dados MQTT
* **Broker MQTT** → `broker.emqx.io` (porta 1883)
* **Extensão Node-RED TelegramBot** → `node-red-contrib-telegrambot`
* **Arduino IDE (opcional)** → alternativa para upload de código

---

## ⚙️ Execução do Projeto

### 1️⃣ Clonar o Repositório

```bash
git clone https://github.com/moacyrsz/challenge-sprint-1.git
cd challenge-sprint-1
```

### 2️⃣ Configurar o PlatformIO (ESP32)

1. Instale a extensão **PlatformIO** no **VSCode**.
2. Abra o projeto (`File > Open Folder`).
3. Compile e inicie a simulação com o **Wokwi**.
4. O código principal está em `src/main.cpp`.

---

### 3️⃣ Configurar o Node-RED

1. Instale o **Node-RED** ([guia oficial](https://nodered.org/docs/getting-started/)).
2. Importe o fluxo:

   * Menu → `Import > Clipboard` → cole o conteúdo de `flow.json` → clique em **Importar**.
3. Configure o **Broker MQTT**:

   * Host: `broker.emqx.io`
   * Porta: `1883`
   * Tópicos: `/challenge/...`
4. Personalize o dashboard conforme necessário.

---

## ⚡ Integração com Telegram

### 🚀 Como Ativar as Notificações

1. No Telegram, acesse **@BotFather** e crie um novo bot com `/newbot`.
2. Copie o **token gerado** (exemplo: `123456789:ABCDefGhijkLmNoPQRSTuvWXyz`).
3. No Node-RED:

   * Instale o pacote `node-red-contrib-telegrambot`.
   * Adicione um nó **Telegram sender** e insira o token no campo **Bot Token**.
   * Conecte-o à sua **function** que envia alertas.
4. Descubra o seu **chatId**:

   * Fale com **@userinfobot** no Telegram e copie o número de ID.
5. Configure a função de alerta no Node-RED:

   ```js
   const status = String(msg.payload || "").trim();
   if (status.toLowerCase() === "ideal") return null;

   const chatId = 1406632269; // substitua pelo seu ID
   msg.payload = {
     chatId,
     type: "message",
     content: `⚠️ O status do sistema mudou para: *${status}*`,
     options: { parse_mode: "Markdown" }
   };
   return msg;
   ```
6. Sempre que o ESP32 publicar um status diferente de **“Ideal”**, o bot enviará uma mensagem automática de alerta no Telegram.

---

## 🌐 Publicações MQTT

O ESP32 publica dados nos seguintes tópicos:

```
/challenge/ambientTemperature
/challenge/humidity
/challenge/batteryTemperature
/challenge/voltage
/challenge/current
/challenge/chargingStatus
```

---

## 🔍 Verificação de Operação

* **Display OLED:** mostra leituras e alertas em tempo real.
* **Painel Node-RED:** exibe dados, gráficos e status atual.
* **Telegram:** envia notificações automáticas sempre que o status for “Abaixo do ideal” ou “Alerta! Condição não ideal”.

---

## ⚡Solução de Problemas

| Problema                             | Solução                                                     |
| ------------------------------------ | ----------------------------------------------------------- |
| **ESP32 não conecta ao Wi-Fi**       | Verifique SSID e senha do Wi-Fi                             |
| **Sem dados no Node-RED**            | Confira o broker MQTT e os tópicos                          |
| **Bot não envia mensagem**           | Verifique se fez `/start` no bot e se o chatId está correto |
| **Polling error no Telegram sender** | Token inválido ou rede bloqueando `api.telegram.org`        |

---

## ⚡Contribuições

Contribuições são bem-vindas!
Envie um *pull request* ou abra uma *issue* com sugestões de melhorias.

---

## 🔋Licença

Este projeto é licenciado sob a **MIT License**.
Consulte o arquivo [LICENSE](LICENSE) para mais detalhes.

---

## ⚡Autor⚡

**Desenvolvido por [Moacyr Cabral](https://github.com/moacyrsz)**