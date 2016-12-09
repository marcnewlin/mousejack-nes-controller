# MouseJack NES Controller

## Software Configuration in config.h (SPI pins)

### nRF24L01+

```
// Radio count and CE/CS pins
const uint8_t radio_count = 5;
const uint8_t radio_pins[radio_count][2] = { {17, 16}, {19, 18}, {4, 3}, {5, 6}, {8, 7} };
```

### NES buttons

```
// Button pins
const uint8_t nes_button_count = 8;
const uint8_t nes_button_pins[nes_button_count] = { 25, 26, 27, 28, 29, 30, 31, 32 };
```

### Voltage divider

```
#define VDIV1 A7
#define VDIV2 A8
#define VDIV3 A8
```

### microSD

```
#define SD_CS 20
```


### OLED

```
#define OLED_CS 2
#define OLED_A0 1
#define OLED_RST 0
```