#include <stdio.h>
#include <math.h>

#include "sdkconfig.h"
#include "esp_adc/adc_continuous.h"
#include "esp_dsp.h"


#include "Wire.h"
#include "Adafruit_NeoPixel.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define BLINK_GPIO 8
#define LED_Quant 1

#define SAMPLE_RATE 44100          // Abtastrate
#define ADC_CHANNEL ADC_CHANNEL_3  // ADC-Kanal (z. B. GPIO3)
adc_continuous_handle_t handle = NULL;
static adc_channel_t channel = ADC_CHANNEL_3;

// prototypes
void display_spectrum();
void ringbuf_add(uint16_t val);
static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle);
static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data);

const uint16_t ringbuf_size = 2048;      //4096;
uint16_t ringbuf[ringbuf_size] = { 0 };  // 2^12 Samples
uint16_t *ringbuf_ptr = ringbuf;
uint16_t *ringbuf_start = ringbuf;
uint16_t *ringbuf_end = &(ringbuf[ringbuf_size - 1]);
uint32_t ret_num = 0;

float time_vals[ringbuf_size * 2] = { 0 };
float frequency_vals[ringbuf_size] = { 0 };
float wind[ringbuf_size] = { 0 };



Adafruit_NeoPixel pixels(LED_Quant, BLINK_GPIO);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
uint16_t spectrum_vis[40] = { 0 };

void setup() {
  // init Button
  pinMode(9, INPUT);



  // init RGB-LED
  pinMode(BLINK_GPIO, OUTPUT);
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();

  // init oled-display
  Wire.begin(19, 18);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    delay(1000);
  }

  display.clearDisplay();
  display.display();

  // init adc-continuous
  continuous_adc_init(&channel, 1, &handle);
  adc_continuous_evt_cbs_t cbs = {
    .on_conv_done = s_conv_done_cb,
  };
  adc_continuous_register_event_callbacks(handle, &cbs, NULL);
  delay(200);
  adc_continuous_start(handle);

  // init dsp library
  dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);

  dsps_wind_hann_f32(wind, ringbuf_size);
}

void loop() {

  bool buttonpressed = false;

  // while (!buttonpressed) {
  //   buttonpressed = !digitalRead(9);
  // }
  // pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  // pixels.show();


  for (int i = 0; i < ringbuf_size; i++) {
    time_vals[i * 2] = ringbuf[i] * wind[i];  // Realteil
    time_vals[i * 2 + 1] = 0;                 // Imaginärteil
  }

  // printf("S,");
  // // printf("Time-Vals: ");
  // for (int i = 0; i < ringbuf_size * 2; i++) {
  //   printf("%f,", time_vals[i]);
  //   //delay(2);
  // }

  // printf("F\n");

  // printf("\n");

  dsps_fft2r_fc32(time_vals, ringbuf_size);
  // dsps_view(time_vals, ringbuf_size*2, 2048*2,10, -1,1,'|');
  dsps_bit_rev_fc32(time_vals, ringbuf_size);
  // dsps_view(time_vals, ringbuf_size*2, 2048*2,10, -1,1,'|');
  // for (int i=0;i<ringbuf_size;i++){
  //   printf("%f,",time_vals[i]);
  //   delay(2);
  // }
  dsps_cplx2reC_fc32(time_vals, ringbuf_size);

  for (int i = 0; i < ringbuf_size / 2; i++) {
    float real = time_vals[i * 2];
    float imag = time_vals[i * 2 + 1];
    frequency_vals[i] = 10 * log10f((real * real + imag * imag) / ringbuf_size);
  }


  /*
  Frequency Spectrum fro 0-44100Hz was computed
  Only 0-22050Hz is relevant (mirrored spectrum on die other side)
  Log10 is used to have a better visualisation of the spectrum
  */

  // printf("H,");

  // for (int i = 0; i < ringbuf_size; i++) {
  //   printf("%f,", frequency_vals[i]);
  //   //delay();
  // }

  // printf("E\n");
  // fflush(stdout);



  // delay(10000);

  // pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  // pixels.show();

  // dc-clearing
  frequency_vals[0] = 0;

  if (true) {
    float sum = 0;
    for (int i = 0; i < 40; i++) {

      for (int j = 0; j < (1020 / 40); j++) {
        sum += frequency_vals[i * (1020 / 40) + j];
      }

      sum /= 40;
      sum = (sum / 30) * 18.0;
      spectrum_vis[i] = sum;
    }
  } else {
    for (int i = 0; i < 40; i++) {
      float buf = 0;

      for (int j = 0; j < (1020 / 40); j++) {
        if (frequency_vals[i * (1020 / 40) + j] > buf)
          buf = frequency_vals[i * (1020 / 40) + j];
      }

      buf = (buf / 50) * 18.0;
      spectrum_vis[i] = buf;
    }
  }


  display_spectrum();
}


void display_spectrum() {
  display.clearDisplay();

  uint16_t rectwidth = SCREEN_WIDTH / 44;
  uint16_t rectheight = SCREEN_HEIGHT / 18;

  for (int i = 39; i >= 0; i--) {
    for (int j = 0; j < spectrum_vis[i]; j++) {
      //display.fillRect(i * rectwidth, j * rectheight, (i + 1) * rectwidth, (j + 1) * rectheight, SSD1306_WHITE);
      display.fillRect((39 - i) * rectwidth + (39 - i), j * rectheight, rectwidth, rectheight, SSD1306_WHITE);
    }
  }

  display.setRotation(2);

  display.display();
}

void ringbuf_add(uint16_t val) {
  if (ringbuf_ptr >= ringbuf_end) {
    ringbuf_ptr = ringbuf_start;
  }
  *ringbuf_ptr = val;
  ringbuf_ptr++;
}

static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle) {
  adc_continuous_handle_t handle = NULL;

  adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = 1024,
    .conv_frame_size = 256,
  };

  ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

  adc_continuous_config_t dig_cfg = {
    .sample_freq_hz = 44100,
    .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
  };

  adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = { 0 };
  dig_cfg.pattern_num = channel_num;

  adc_pattern[0].atten = ADC_ATTEN_DB_12;
  adc_pattern[0].channel = (*channel) & 0x7;
  adc_pattern[0].unit = ADC_UNIT_1;
  adc_pattern[0].bit_width = ADC_BITWIDTH_12;

  dig_cfg.adc_pattern = adc_pattern;
  ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

  *out_handle = handle;
}

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data) {

  uint8_t tmp_buffer[256] = { 0 };

  adc_continuous_read(handle, tmp_buffer, 256, &ret_num, 0);

  for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES) {
    ringbuf_add(((adc_digi_output_data_t *)&tmp_buffer[i])->type2.data);
  }

  // adc_digi_output_data_t *p = (adc_digi_output_data_t *)ringbuf;
  // uint32_t chan_num = p->type2.channel;
  // uint32_t data = p->type2.data;

  // SOC_ADC_DIGI_RESULT_BYTES;

  return true;
}
