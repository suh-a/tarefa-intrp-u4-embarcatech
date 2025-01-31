#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "main.pio.h"

// Pin definitions
#define LED_RGB_RED 13
#define LED_RGB_GREEN 11
#define LED_RGB_BLUE 12
#define BUTTON_A 5
#define BUTTON_B 6
#define MATRIX_PIN 7
#define NUM_PIXELS 25

// Timing constants
#define DEBOUNCE_DELAY_MS 50
#define RED_BLINK_INTERVAL_MS 200    // 5 times per second
#define RGB_BLINK_INTERVAL_MS 333    // 3 times per second
#define DIGIT_DISPLAY_DELAY_MS 1000

// Global variables
static volatile int current_number = 0;
static volatile uint32_t last_button_time = 0;
static volatile bool red_led_active = true;
static volatile bool green_led_active = false;
static volatile bool blue_led_active = false;
static volatile int button_a_count = 0;
static volatile int button_b_count = 0;


// Function prototypes
void setup_gpio(void);
uint32_t matrix_rgb(double r, double g, double b);
void display_number(PIO pio, uint sm, int number, double r, double g, double b);
void handle_rgb_leds(void);
bool debounce_check(void);
void gpio_callback(uint gpio, uint32_t events);

// LED matrix number patterns
static const double number_patterns[10][25] = {
    // 0
    {1,1,1,1,1, 
     1,0,0,0,1, 
     1,0,0,0,1, 
     1,0,0,0,1, 
     1,1,1,1,1},

    // 1
    {0,1,1,1,0, 
     0,0,1,0,0, 
     0,0,1,0,0, 
     0,1,1,0,0, 
     0,0,1,0,0},

    // 2
    {1,1,1,1,1, 
     1,0,0,0,0, 
     1,1,1,1,1, 
     0,0,0,0,1, 
     1,1,1,1,1},

    // 3
    {1,1,1,1,1, 
     0,0,0,0,1, 
     0,1,1,1,1, 
     0,0,0,0,1, 
     1,1,1,1,1},

    // 4
    {0,1,0,0,0, 
     0,0,0,1,0, 
     0,1,1,1,1, 
     1,0,0,1,0, 
     0,1,0,0,1},

    // 5
    {1,1,1,1,1, 
     0,0,0,0,1, 
     1,1,1,1,1, 
     1,0,0,0,0, 
     1,1,1,1,1},

    // 6
    {1,1,1,1,1, 
     1,0,0,0,1, 
     1,1,1,1,1, 
     1,0,0,0,0, 
     1,1,1,1,1},

    // 7
    {0,0,0,1,0, 
     0,0,1,0,0, 
     0,1,0,0,0, 
     0,0,0,0,1, 
     1,1,1,1,1},

    // 8
    {1,1,1,1,1, 
     1,0,0,0,1, 
     1,1,1,1,1, 
     1,0,0,0,1, 
     1,1,1,1,1},

    // 9
    {1,1,1,1,1, 
     0,0,0,0,1, 
     1,1,1,1,1, 
     1,0,0,0,1, 
     1,1,1,1,1}

};

void setup_gpio(void) {
    // Initialize RGB LED pins
    gpio_init(LED_RGB_RED);
    gpio_init(LED_RGB_GREEN);
    gpio_init(LED_RGB_BLUE);
    gpio_set_dir(LED_RGB_RED, GPIO_OUT);
    gpio_set_dir(LED_RGB_GREEN, GPIO_OUT);
    gpio_set_dir(LED_RGB_BLUE, GPIO_OUT);

    // Initialize buttons with pull-ups
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);

    // Setup interrupts for buttons
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

}

uint32_t matrix_rgb(double r, double g, double b) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

void display_number(PIO pio, uint sm, int number, double r, double g, double b) {
    if (number < 0 || number > 9) return;

    double brightness = 0.3; // Reduz intensidade (de 0.0 a 1.0)

    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t led_value = matrix_rgb(
            brightness * r * number_patterns[number][i],
            brightness * g * number_patterns[number][i],
            brightness * b * number_patterns[number][i]
        );
        pio_sm_put_blocking(pio, sm, led_value);
    }
}


void handle_rgb_leds(void) {
    static uint32_t last_red_blink = 0;
    static uint32_t last_rgb_blink = 0;
    static bool led_state = false;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Red LED blinking (5 times per second)
    if (red_led_active && (current_time - last_red_blink >= RED_BLINK_INTERVAL_MS)) {
        led_state = !led_state;
        gpio_put(LED_RGB_RED, led_state);
        last_red_blink = current_time;
    }

    // Green/Blue LED blinking (3 times per second)
    if ((green_led_active || blue_led_active) && 
        (current_time - last_rgb_blink >= RGB_BLINK_INTERVAL_MS)) {
        led_state = !led_state;
        if (green_led_active) gpio_put(LED_RGB_GREEN, led_state);
        if (blue_led_active) gpio_put(LED_RGB_BLUE, led_state);
        last_rgb_blink = current_time;
    }
}

bool debounce_check(void) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_button_time < DEBOUNCE_DELAY_MS) {
        return false;
    }
    last_button_time = current_time;
    return true;
}

void gpio_callback(uint gpio, uint32_t events) {
    if (!debounce_check()) return;

    if (gpio == BUTTON_A) {
        current_number = (current_number + 1) % 10;
        red_led_active = false;
        green_led_active = true;
        blue_led_active = false;
        gpio_put(LED_RGB_RED, 0);
        gpio_put(LED_RGB_BLUE, 0);
        button_a_count++;  // Incrementa o contador de A
        printf("Botão A pressionado! Contagem: %d\n", button_a_count);
    } 
    else if (gpio == BUTTON_B) {
        current_number = (current_number - 1 + 10) % 10;
        red_led_active = false;
        green_led_active = false;
        blue_led_active = true;
        gpio_put(LED_RGB_RED, 0);
        gpio_put(LED_RGB_GREEN, 0);
        button_b_count++;  // Incrementa o contador de B
        printf("Botão B pressionado! Contagem: %d\n", button_b_count);
    }
}

void print_button_presses_per_second() {
    static int last_button_a_count = 0;
    static int last_button_b_count = 0;
    static uint32_t last_time = 0;

    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_time >= 1000) {  // A cada 1 segundo
        int presses_a_per_sec = button_a_count - last_button_a_count;
        int presses_b_per_sec = button_b_count - last_button_b_count;

        printf("Pressionamentos por segundo -> A: %d, B: %d\n", presses_a_per_sec, presses_b_per_sec);

        last_button_a_count = button_a_count;
        last_button_b_count = button_b_count;
        last_time = current_time;
    }
}




int main() {
    stdio_init_all();
    setup_gpio();

    // Initialize PIO for LED matrix
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &main_program);
    uint sm = pio_claim_unused_sm(pio, true);
    main_program_init(pio, sm, offset, MATRIX_PIN);

    // Initial LED states
    red_led_active = true;
    green_led_active = false;
    blue_led_active = false;

   int last_displayed_number = -1; // Armazena o último número exibido

    while (true) {
        // Apenas atualiza a matriz de LEDs se o número mudou
        if (current_number != last_displayed_number) {
        display_number(pio, sm, current_number, 0.0, 1.0, 1.0);
        last_displayed_number = current_number;
        }

        print_button_presses_per_second(); 
        // Gerencia a piscagem dos LEDs RGB
        handle_rgb_leds();

        sleep_ms(10); // Pequeno atraso para evitar sobrecarga do processador
    }


    return 0;
}