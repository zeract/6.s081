#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

struct entry {
  int key;
  int value;
  struct entry *next;
};
struct entry *table[NBUCKET];
int keys[NKEYS];
int nthread = 1;

pthread_mutex_t lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_mutex_t lock4;
double
now()
{
 struct timeval tv;
 gettimeofday(&tv, 0);
 return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void 
insert(int key, int value, struct entry **p, struct entry *n)
{
  struct entry *e = malloc(sizeof(struct entry));
  e->key = key;
  e->value = value;
  e->next = n;
  *p = e;
}

static 
void put(int key, int value)
{
  int i = key % NBUCKET;
  // is the key already present?
  struct entry *e = 0;
  
  for (e = table[i]; e != 0; e = e->next) {
    
    if (e->key == key)
      break;
  }
  
  if(e){
    // update the existing key.
    e->value = value;
  } else {
    // the new is new.
    
    insert(key, value, &table[i], table[i]);
  }
  
}

static struct entry*
get(int key)
{
  int i = key % NBUCKET;
  
  struct entry *e = 0;
  //pthread_mutex_lock(&lock);
  for (e = table[i]; e != 0; e = e->next) {
    
    if (e->key == key) break;
  }
  //pthread_mutex_unlock(&lock);
  return e;
}

static void *
put_thread(void *xa)
{
  int n = (int) (long) xa; // thread number
  int b = NKEYS/nthread;

  for (int i = 0; i < b; i++) {
    if((keys[b*n + i])%NBUCKET==0){
      pthread_mutex_lock(&lock);
      put(keys[b*n + i], n);
      pthread_mutex_unlock(&lock);
    }
    else if((keys[b*n + i])%NBUCKET==1){
      pthread_mutex_lock(&lock1);
      put(keys[b*n + i], n);
      pthread_mutex_unlock(&lock1);
    }
    else if((keys[b*n + i])%NBUCKET==2){
      pthread_mutex_lock(&lock2);
      put(keys[b*n + i], n);
      pthread_mutex_unlock(&lock2);
    }
    else if((keys[b*n + i])%NBUCKET==3){
      pthread_mutex_lock(&lock3);
      put(keys[b*n + i], n);
      pthread_mutex_unlock(&lock3);
    }
    else if((keys[b*n + i])%NBUCKET==4){
      pthread_mutex_lock(&lock4);
      put(keys[b*n + i], n);
      pthread_mutex_unlock(&lock4);
    }
  }

  return NULL;
}

static void *
get_thread(void *xa)
{
  int n = (int) (long) xa; // thread number
  int missing = 0;

  for (int i = 0; i < NKEYS; i++) {
    struct entry *e = get(keys[i]);
    if (e == 0) missing++;
  }
  printf("%d: %d keys missing\n", n, missing);
  return NULL;
}

int
main(int argc, char *argv[])
{
  pthread_t *tha;
  void *value;
  double t1, t0;
  pthread_mutex_init(&lock,NULL);
  pthread_mutex_init(&lock1,NULL);
  pthread_mutex_init(&lock2,NULL);
  pthread_mutex_init(&lock3,NULL);
  pthread_mutex_init(&lock4,NULL);
  if (argc < 2) {
    fprintf(stderr, "Usage: %s nthreads\n", argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);
  assert(NKEYS % nthread == 0);
  for (int i = 0; i < NKEYS; i++) {
    keys[i] = random();
  }

  //
  // first the puts
  //
  t0 = now();
  for(int i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, put_thread, (void *) (long) i) == 0);
  }
  for(int i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  t1 = now();

  printf("%d puts, %.3f seconds, %.0f puts/second\n",
         NKEYS, t1 - t0, NKEYS / (t1 - t0));

  //
  // now the gets
  //
  t0 = now();
  for(int i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, get_thread, (void *) (long) i) == 0);
  }
  for(int i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  t1 = now();

  printf("%d gets, %.3f seconds, %.0f gets/second\n",
         NKEYS*nthread, t1 - t0, (NKEYS*nthread) / (t1 - t0));
}
