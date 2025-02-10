# Projeto: Interface de Comunicação Serial com RP2040

## Descrição

Este projeto tem como objetivo explorar e implementar comunicação serial utilizando a placa de desenvolvimento **BitDogLab** com o microcontrolador **RP2040**. Durante o desenvolvimento, foi utilizado o protocolo **I2C** para interação com periféricos, bem como controle de LEDs e botões com tratamento de interrupções e debounce.

## Componentes Utilizados

- **Matriz 5x5 de LEDs WS2812** (endereçáveis) - GPIO 7
- **LED RGB** - GPIOs 11 e 12
- **Botão A** - GPIO 5
- **Botão B** - GPIO 6
- **Display SSD1306** - GPIOs 14 e 15

## Funcionalidades Implementadas

1. **Modificação da Biblioteca `font.h`**
   - Adição de caracteres minúsculos personalizados para exibição no display.

2. **Entrada de caracteres via Serial Monitor**
   - Captura de caracteres digitados no **VS Code Serial Monitor**.
   - Exibição dos caracteres no **display SSD1306**.
   - Quando um número de **0 a 9** é digitado, um símbolo correspondente é exibido na **matriz de LEDs WS2812**.

3. **Interação com os Botões**
   - **Botão A:** Alterna o estado do **LED RGB Verde**.
     - Exibe mensagem informativa no **display SSD1306**.
     - Envia um texto descritivo ao **Serial Monitor**.
   - **Botão B:** Alterna o estado do **LED RGB Azul**.
     - Exibe mensagem informativa no **display SSD1306**.
     - Envia um texto descritivo ao **Serial Monitor**.

## Requisitos

- **Uso de interrupções:** Todas as interações com os botões utilizam **rotinas de interrupção (IRQ)**.
- **Debouncing:** Implementado via software para garantir leituras confiáveis dos botões.
- **Controle de LEDs:** Foram utilizados tanto LEDs comuns quanto endereçáveis (WS2812).
- **Utilização do Display 128 x 64:** Com suporte a **maiúsculas e minúsculas**.
- **Organização do Código:** Estruturado e bem comentado para facilitar a compreensão.

## Instrução de Uso

1. Clone o repositório do projeto:
https://github.com/FilipeVBF/EmbarcaTech_tarefa_comunicacao_serial.git
2. Importe a pasta do projeto para o ambiente de desenvolvimento do Pico SDK.
3. Carregue o programa no Raspberry Pi Pico W
4. Compile o código
5. Para execução na simulação do Wokwi, abra o arquivo diagram.json e inicie a simulação
6. Conecte o hardware conforme a configuração sugerida.
7. Pressione os botões para alternar o estado dos LEDs:
    - Botão A: Alterna o LED Verde.
    - Botão B: Alterna o LED Azul.
8. Sempre que um LED mudar de estado, a informação será atualizada no display SSD1306 e enviada ao Serial Monitor.
9. Digite um número (0-9) no Serial Monitor para exibir o símbolo correspondente na matriz de LEDs WS2812.
10. Digite qualquer letra ou número no Serial Monitor para exibi-lo no display SSD1306.

## Vídeo da Solução
Segue abaixo o link do vídeo da demonstração do projeto:
[Ver vídeo](https://drive.google.com/file/d/1aLMba1WNZz24jvzbeRZMH-ZRNyGpJIKk/view?usp=sharing)
