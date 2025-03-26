#include <pthread.h>

#define TEST_THREAD_NUM (2)  // スレッドの数
char key_code = 0;

pthread_mutex_t using_blink_led;

const int spresense_main_board_leds[2] = {
  LED0,
  LED1
};

const int key_codes[2] = {
  0x31, // '1'
  0x32  // '2'
};

void* check_KeyBoard(void* arg) {
  while (1) {
    if (Serial.available()) {
      Serial.readBytes(&key_code, 1);
      printf("key input : %c\n", key_code);
    }
    usleep(10 * 1000); // 10msec sleep
  }

  return (void*)0;
}

void* blink_led(void* arg) {
  uint32_t led_index = (uint32_t)arg;
  uint32_t key_index = (uint32_t)arg;
  unsigned int sleep_sec = 1 + (uint32_t)arg; // sleepする秒数
  int led_value = 1;

  printf("start blink_led[%ld] thread ID = %d.\n", led_index, pthread_self());

  while (key_code != key_codes[key_index]) {  // 特定のキーが押下されたらループから抜ける
    pthread_mutex_lock(&using_blink_led);

    digitalWrite(spresense_main_board_leds[led_index], led_value);
    sleep(sleep_sec);
    led_value ^= 1;
    printf("blink_led[%ld] thread ID = %d led_value = %d key_code = 0x%02x.\n", led_index, pthread_self(), led_value, key_code);

    pthread_mutex_unlock(&using_blink_led);
  }

  printf("end blink_led[%ld] thread ID = %d.\n", led_index, pthread_self());

  return (void*)led_index;
}

void setup() {
  pthread_t thread[TEST_THREAD_NUM];
  pthread_t check_KeyBoard_thread;
  int ret[TEST_THREAD_NUM];
  int i;

  Serial.begin(115200);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  if (pthread_mutex_init(&using_blink_led, NULL) != 0) {
      printf("Error; pthread_mutex_init.\n");
      exit(1);
  }

  if(pthread_create(&check_KeyBoard_thread, NULL, check_KeyBoard, 0) != 0) {
      printf("Error; check_KeyBoard pthread_create.\n");
      exit(1);
  }

  for (i = 0; i < TEST_THREAD_NUM; i++) {
    if(pthread_create(&thread[i], NULL, blink_led, (void*)i) != 0) {
      printf("Error; pthread_create[%d].\n", i);
      exit(1);
    }
  }  

  for (i = 0; i < TEST_THREAD_NUM; i++){
    if (pthread_join(thread[i], (void**)&ret[i]) != 0) {
      printf("Error; pthread_join[%d].\n", i);
      exit(1);
    }
    printf("exit thread[%d] return value = %d\n", i, ret[i]);
  }

  if (pthread_mutex_destroy(&using_blink_led) != 0) {
      printf("Error; pthread_mutex_destroy.\n");
      exit(1);
  }

  printf("End.\n");
}

void loop() {
}
