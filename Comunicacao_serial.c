#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pio.h"
#include "Comunicacao_serial.pio.h"
#include "hardware/clocks.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define NUM_PIXELS 25 // Número de LEDs no display
#define WS2812_PIN 7 // Pino GPIO usado para o LED
#define IS_RGBW false // Define se o LED é RGBW ou RGB
#define LED_PIN_GREEN    11 // LED Green
#define LED_PIN_BLUE     12 // LED Blue
#define BUTTON_PIN_A     5 // Botão A
#define BUTTON_PIN_B     6 // Botão B

ssd1306_t ssd; // Inicializa a estrutura do display

// Intensidade das cores RGB
uint8_t color_red = 0;
uint8_t color_green = 0;
uint8_t color_blue = 50;

static volatile uint32_t last_time = 0; // Tempo do último clique

const uint8_t led_map[NUM_PIXELS] = {
  24, 23, 22, 21, 20,
  15, 16, 17, 18, 19,
  14, 13, 12, 11, 10,
   5,  6,  7,  8,  9,
   0,  1,  2,  3,  4
};

uint32_t led_buffer[10][NUM_PIXELS] = {
  // Número 0
  {
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0
  },
  // Número 1 
  {
      0, 0, 1, 0, 0,
      0, 1, 1, 0, 0,
      0, 0, 1, 0, 0,
      0, 0, 1, 0, 0,
      0, 1, 1, 1, 0
  },
  // Número 2
  {
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 0, 1, 0, 0,
      0, 1, 0, 0, 0,
      0, 1, 1, 1, 0
  },
  // Número 3 
  {
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0
  },
  // Número 4 
  {
      0, 1, 0, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 0, 0, 1, 0
  },
  // Número 5
  {
      0, 1, 1, 1, 0,
      0, 1, 0, 0, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0
  },
  // Número 6 
  {
      0, 1, 1, 1, 0,
      0, 1, 0, 0, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0
  },
  // Número 7 
  {
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 0, 1, 0, 0,
      0, 1, 0, 0, 0,
      0, 1, 0, 0, 0
  },
  // Número 8 
  {
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0
  },
  // 9
  {
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 1, 1, 0
  }
}; // Buffer para armazenar as cores de todos os LEDs

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void change_led_buffer(char number_display) {
  // Define a cor com base nos parâmetros fornecidos
  uint32_t color = urgb_u32(color_red, color_green, color_blue);

  int number = number_display - '0'; // Converte o caractere para um número inteiro

  // Define todos os LEDs com a cor especificada
  for (int row = 0; row < 5; row++) {
      int start = row * 5;
      int end = start + 5 - 1;

      // Usando o mapeamento físico para corrigir a ordem
      for (int i = start; i <= end; i++) {

          int mapped_index = led_map[i];

          if (led_buffer[number][mapped_index]) {
              put_pixel(color); // Liga o LED com um no buffer
          } else {
              put_pixel(0);  // Desliga os LEDs com zero no buffer
          }
      }
  }
}

void init_gpio_led(int led_pin, bool is_output, bool state) {
  gpio_init(led_pin);                                         // Inicializa o pino do LED
  gpio_set_dir(led_pin, is_output ? GPIO_OUT : GPIO_IN);      // Configura o pino como saída ou entrada
  gpio_put(led_pin, state);                                   // Garante que o LED inicie apagado
}

void init_gpio_button(int button_pin, bool is_output) {
  gpio_init(button_pin);                                          // Inicializa o botão
  gpio_set_dir(button_pin, is_output ? GPIO_OUT : GPIO_IN);       // Configura o pino como entrada pu saída
  gpio_pull_up(button_pin);                                       // Habilita o pull-up interno
}

// Função de interrupção para atualizar o número exibido
void switch_led (uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 500000)
    {
        last_time = current_time;
        if (gpio == 5) {
          gpio_put(LED_PIN_GREEN, !gpio_get(LED_PIN_GREEN));
          printf("LED verde %s\r\n", gpio_get(LED_PIN_GREEN) ? "ligado!" : "desligado!");
          ssd1306_draw_string(&ssd, gpio_get(LED_PIN_GREEN) ? "on " : "off", 90, 22); // Desenha uma string
        } else if (gpio == 6) {
          gpio_put(LED_PIN_BLUE, !gpio_get(LED_PIN_BLUE));
          printf("LED azul %s\r\n", gpio_get(LED_PIN_BLUE) ? "ligado!" : "desligado!");
          ssd1306_draw_string(&ssd, gpio_get(LED_PIN_BLUE) ? "on " : "off", 82, 32); // Desenha uma string
        }
        ssd1306_send_data(&ssd); // Atualiza o display
    }
}
int main()
{
  stdio_init_all();

  // Inicializando pino do LED RGB
  init_gpio_led(LED_PIN_BLUE, true, false);
  init_gpio_led(LED_PIN_GREEN, true, false);

  // Inicializando pino do botão
  init_gpio_button(BUTTON_PIN_A, false);
  init_gpio_button(BUTTON_PIN_B, false);

  PIO pio = pio0;
  int sm = 0;
  
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

  // Configuração da interrupção com callback
  gpio_set_irq_enabled_with_callback(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true, &switch_led );
  gpio_set_irq_enabled_with_callback(BUTTON_PIN_B, GPIO_IRQ_EDGE_FALL, true, &switch_led );

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  bool cor = true;

  printf("Digite algo e veja o eco:\r\n");
  
  char tecla;
  while (true)
  {
   
    cor = !cor;
    // Atualiza o conteúdo do display
    ssd1306_fill(&ssd, !cor); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 10); // Desenha uma string

    ssd1306_draw_string(&ssd, gpio_get(LED_PIN_GREEN) ? "LED verde on " : "LED verde off", 10, 22); // Desenha uma string
    ssd1306_draw_string(&ssd, gpio_get(LED_PIN_BLUE) ? "LED azul on " : "LED azul off", 10, 32); // Desenha uma string
    ssd1306_draw_string(&ssd, "Caractere \r\n", 10, 48); // Desenha uma string

    ssd1306_send_data(&ssd); // Atualiza o display

    scanf(" %c",&tecla);
    printf("Tecla digitada: %c\r\n", tecla);

    char str[2] = {tecla, '\0'};
    ssd1306_draw_string(&ssd, str, 90, 50); // Desenha uma string      
    ssd1306_send_data(&ssd); // Atualiza o display


    if (tecla >= '0' && tecla <= '9') {
      change_led_buffer(tecla);
    };

    sleep_ms(1000); // Aguarda 1 segundo
  }
}