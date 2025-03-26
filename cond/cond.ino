#include <pthread.h>

#define TEST_THREAD_NUM (2)  // スレッドの数
char key_code = 0;

pthread_mutex_t mutex[TEST_THREAD_NUM];
pthread_cond_t cond[TEST_THREAD_NUM];

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

void* set_cond_signal(void* arg) {
  int index = (uint32_t)arg;
  unsigned int sleep_sec = 1 + (uint32_t)arg; // sleepする秒数
  int led_value = 1;

  while (key_code != key_codes[index]) {  // 特定のキーが押下されたらループから抜ける
    digitalWrite(spresense_main_board_leds[index], led_value);
    sleep(sleep_sec);
    led_value ^= 1;
  }

  pthread_cond_signal(&cond[index]);

  printf("thread set_cond_signal[%d]: cond_signal[%d] set.\n", index, index);
  digitalWrite(spresense_main_board_leds[index], 0);

  return (void*)index;
}

void* wait_cond_signal(void* arg) {
  int index = (uint32_t)arg;

  printf("thread wait_cond_signal[%d]:cond_wait[%d]...\n", index, index);

  // 条件待ち
  pthread_mutex_lock(&mutex[index]);
  if (pthread_cond_wait(&cond[index], &mutex[index]) != 0) {
      printf("Fatal error on pthread[%d]_cond_wait.\n", index);
      exit(1);
  }
  pthread_mutex_unlock(&mutex[index]);

  printf("thread wait_cond_signal[%d]:wait end.\n", index);

  return (void*)index;
}


void setup() {
  pthread_t thread_cond_signal[TEST_THREAD_NUM];
  pthread_t thread_cond_wait[TEST_THREAD_NUM];
  pthread_t check_KeyBoard_thread;
  int ret[TEST_THREAD_NUM];
  int i;

  Serial.begin(115200);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  for(i = 0; i < TEST_THREAD_NUM; i++){
    if (pthread_mutex_init(&mutex[i], NULL) != 0) {
        printf("Error; pthread_mutex_init-%d.\n", i);
        exit(1);
    }
  }

  for(i = 0; i < TEST_THREAD_NUM; i++){
    if (pthread_cond_init(&cond[i], NULL) != 0) {
        printf("Error; pthread_cond_init-%d.\n", i);
        exit(1);
    }
  }

  if(pthread_create(&check_KeyBoard_thread, NULL, check_KeyBoard, 0) != 0) {
      printf("Error; check_KeyBoard pthread_create.\n");
      exit(1);
  }

  for (i = 0; i < TEST_THREAD_NUM; i++) {
    if(pthread_create(&thread_cond_signal[i], NULL, set_cond_signal, (void*)i) != 0) {
      printf("Error; pthread_create cond_signal[%d].\n", i);
      exit(1);
    }
    if(pthread_create(&thread_cond_wait[i], NULL, wait_cond_signal, (void*)i) != 0) {
      printf("Error; pthread_create cond_wait[%d].\n", i);
      exit(1);
    }
  }

  for (i = 0; i < TEST_THREAD_NUM; i++){
    if (pthread_join(thread_cond_signal[i], (void**)&ret[i]) != 0) {
      printf("Error; thread_cond_signal[%d].\n", i);
      exit(1);
    }
    printf("main:thread_cond_signal[%d] exit.return value = %d\n", i, ret[i]);
  }

  for (i = 0; i < TEST_THREAD_NUM; i++){
    if (pthread_join(thread_cond_wait[i], (void**)&ret[i]) != 0) {
      printf("Error; thread_cond_wait[%d].\n", i);
      exit(1);
    }
    printf("main:thread_cond_wait[%d] exit.return value = %d\n", i, ret[i]);
  }

  for (i = 0; i < TEST_THREAD_NUM; i++) {
    if (pthread_mutex_destroy(&mutex[i]) != 0) {
        printf("Error; pthread_mutex_destroy-%d.\n", i);
        exit(1);
    }
  }

  for (i = 0; i < TEST_THREAD_NUM; i++) {
    if (pthread_cond_destroy(&cond[i]) != 0) {
        printf("Error; pthread_cond_destroy-%d.\n", i);
        exit(1);
    }
  }

  printf("End.\n");
}

void loop() {
}
