#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

hashtable *hash_new(size_t cap, uint64_t (*hash)(kv ptr, uint64_t N), int (*cmp)(kv k1, kv k2)) {
  list_node **vs = calloc(cap, sizeof(list_node *));
  hashtable *ht = malloc(sizeof(hashtable));
  ht->nodes = vs;
  ht->cap = cap;
  ht->hash = hash;
  ht->len = 0;
  ht->cmp = cmp;
  return ht;
}

size_t hash_len(hashtable *ht) {
  return ht->len;
}

void hash_set(hashtable *ht, kv k, kv v) {
  uint64_t h = ht->hash(k, ht->cap);
  list_node *node = ht->nodes[h];
  list_node *prev = NULL;
  while (node) {
    if (!ht->cmp(k, node->key)) {
      node->val = v;
      return;
    }
    prev = node;
    node = node->next;
  }
  list_node *new = malloc(sizeof(struct list_node));
  new->next = NULL;
  new->key = k;
  new->val = v;
  if (!prev) {
    ht->nodes[h] = new;
  } else {
    prev->next = new;
  }
  ht->len++;
}

int hash_hask(hashtable *ht, kv k) {
  uint64_t h = ht->hash(k, ht->cap);
  list_node *node = ht->nodes[h];
  while (node) {
    if (!ht->cmp(k, node->key)) {
      return 1;
    }
    node = node->next;
  }
  return 0;
}

kv hash_getv(hashtable *ht, kv k) {
  uint64_t h = ht->hash(k, ht->cap);
  list_node *node = ht->nodes[h];
  while (node) {
    if (!ht->cmp(k, node->key)) {
      return node->val;
    }
    node = node->next;
  }
  return KV(.ptr=NULL);
}

void hash_del(hashtable *ht, kv k) {
  uint64_t h = ht->hash(k, ht->cap);
  list_node *node = ht->nodes[h];
  list_node *prev = NULL;
  while (node) {
    if (!ht->cmp(k, node->key)) {
      if (!prev) {
        ht->nodes[h] = node->next;
      } else {
        prev->next = node->next;
      }
      ht->len--;
      free(node); // free the node's mem outside
      return;
    }
    prev = node;
    node = node->next;
  }
}

kv *hash_keys(hashtable *ht) {
  kv *keys = calloc(sizeof(kv), ht->len);
  for (int i = 0, k = 0; i < ht->cap; i++) {
    list_node *node = ht->nodes[i];
    while (node) {
      keys[k++] = node->key;
      node = node->next;
    }
  }
  return keys;
}

void hash_free(hashtable *ht) {
  kv *keys = hash_keys(ht);
  for (int i = 0; i < ht->len; i++) {
    hash_del(ht, keys[i]);
  }
  free(keys);
  free(ht->nodes);
  free(ht);
}

uint64_t hash_int(kv k, size_t N) {
  uint64_t ll = k.uint64;
  uint64_t i = ((ll >> 32) ^ ll);
  return i % N;
}

uint64_t hash_str(kv k, size_t N) {
  uint64_t i = 0;
  char c;
  while ((c = *k.str++)) {
    i += i * 89 + (int)c;
    i %= N;
  }
  return i;
}

int hash_cmp_int(kv k1, kv k2) {
  if (k1.uint64 < k2.uint64) {
    return -1;
  }
  if (k1.uint64 > k2.uint64) {
    return 1;
  }
  return 0;
}

int hash_cmp_str(kv k1, kv k2) {
  return strcmp(k1.str, k2.str);
}
