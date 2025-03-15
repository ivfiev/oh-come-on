#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>
#include <stdint.h>

typedef union keyval_t {
  void *ptr;
  char *str;
  uint64_t uint64;
  int int32;
  float float32;
  int vec32[2];
  short vec16[4];
  uint8_t vec8[8];
} kv;

typedef struct list_node {
  kv key;
  kv val;
  struct list_node *next;
} list_node;

typedef struct hashtable {
  list_node **nodes;
  size_t cap;
  size_t len;

  int (*cmp)(kv k1, kv k2);

  uint64_t (*hash)(kv k, uint64_t N);
} hashtable;

#define KV(def) ((kv) {def})

#define FOREACH_KV(hash_tbl, code) \
  do {                                 \
  kv *kvs = hash_keys(hash_tbl);   \
  size_t tbl_len = hash_len(hash_tbl);         \
  for (int k_ix = 0; k_ix < tbl_len; k_ix++) { \
    kv key = kvs[k_ix];       \
    kv val = hash_getv(hash_tbl, key);  \
    code \
  } \
  free(kvs);      \
  } while (0)

hashtable *hash_new(size_t cap, uint64_t (*hash)(kv k, size_t N), int (*cmp)(kv k1, kv k2));

size_t hash_len(hashtable *ht);

void hash_set(hashtable *ht, kv k, kv v);

int hash_hask(hashtable *ht, kv k);

kv hash_getv(hashtable *ht, kv k);

void hash_del(hashtable *ht, kv k);

kv *hash_keys(hashtable *ht);

void hash_free(hashtable *ht);

uint64_t hash_int(kv k, size_t N);

uint64_t hash_str(kv k, size_t N);

int hash_cmp_str(kv k1, kv k2);

int hash_cmp_int(kv k1, kv k2);

#endif