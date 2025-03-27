#include <pthread.h>

#define TEST_THREAD_NUM (3)  // スレッドの数

// 初期化完了同期用バリア変数
pthread_barrier_t initialization_completion_barrier;

const int spresense_main_board_leds[2] = {
  LED0,
  LED1
};

void* thread(void* arg) {
  int thread_index = (uint32_t)arg;
  int ret;
#if 0
  unsigned int initialize_time = 3 * (thread_index + 1);
#else
  unsigned int initialize_time;

  if (thread_index == 0) {
    initialize_time = 9;    
  } else if (thread_index == 1) {
    initialize_time = 6;    
  } else {
    initialize_time = 3;
  }
#endif

  printf("thread index[%d] start.initialize_time = %d(sec).\n", thread_index, initialize_time);

  sleep(initialize_time);

  printf("thread index[%d] initialize end.\n", thread_index);

  ret = pthread_barrier_wait(&initialization_completion_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait ret = %d.\n", thread_index, ret);
    exit(1);
  }

  printf("thread index[%d] barrier exit.ret = %d.\n", thread_index, ret);

  return (void*)thread_index;
}

void setup() {
  pthread_t thread_id[TEST_THREAD_NUM];
  int ret[TEST_THREAD_NUM];
  int i;

  Serial.begin(115200);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  if (pthread_barrier_init(&initialization_completion_barrier, NULL, TEST_THREAD_NUM) != 0) {
      printf("Error; pthread_barrier_init.\n");
      exit(1);
  }

  for (i = 0; i < TEST_THREAD_NUM; i++) {
    if(pthread_create(&thread_id[i], NULL, thread, (void*)i) != 0) {
      printf("Error; pthread_create [%d].\n", i);
      exit(1);
    }
  }

  for (i = 0; i < TEST_THREAD_NUM; i++){
    if (pthread_join(thread_id[i], (void**)&ret[i]) != 0) {
      printf("Error; pthread_join[%d].\n", i);
      exit(1);
    }
    printf("main:thread index[%d] exit.return value = %d\n", i, ret[i]);
  }

  if (pthread_barrier_destroy(&initialization_completion_barrier) != 0) {
      printf("Error; pthread_barrier_destroy.\n");
      exit(1);
  }

  printf("End.\n");
}

void loop() {
}
