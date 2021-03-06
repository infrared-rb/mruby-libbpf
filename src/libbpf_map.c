#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/value.h>
#include <mruby/error.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "mrb_libbpf.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

typedef struct {
  int fd;
  int key_size;
  int value_size;
  enum bpf_map_type type;
  struct bpf_map *ptr;
} mrb_libbpf_map_data;

static const struct mrb_data_type mrb_libbpf_map_data_type = {
  "mrb_libbpf_map_data", mrb_free,
};

mrb_value mrb_libbpf_map_generate(mrb_state *mrb,
                                  int key_size,
                                  int value_size,
                                  enum bpf_map_type type,
                                  struct bpf_map *ptr)
{
  struct RClass *m = mrb_module_get(mrb, "BPF");
  struct RClass *c = mrb_class_get_under(mrb, m, "Map");
  mrb_value self = mrb_obj_new(mrb, c, 0, NULL);
  DATA_TYPE(self) = &mrb_libbpf_map_data_type;

  mrb_libbpf_map_data *data = mrb_malloc(mrb, sizeof(mrb_libbpf_map_data));
  data->key_size = key_size;
  data->value_size = value_size;
  data->type = type;
  data->ptr = ptr;

  data->fd = bpf_map__fd(ptr);

  DATA_PTR(self) = data;

  return self;
}

const __u32 get_first = -2;

#define MAX_SLOTS	15
struct hist {
  __u32 slots[MAX_SLOTS];
};

mrb_value mrb_libbpf_map__debug(mrb_state *mrb, mrb_value self)
{
  if(DATA_PTR(self) == NULL)
    mrb_sys_fail(mrb, "Invalidly initialized map");

  //struct bpf_map *hists = ((mrb_libbpf_map_data *)DATA_PTR(self))->ptr;
  int fd = ((mrb_libbpf_map_data *)DATA_PTR(self))->fd;
  __u32 lookup_key = -2, next_key;
  struct hist h;

  while (!bpf_map_get_next_key(fd, &lookup_key, &next_key)) {
    int err = bpf_map_lookup_elem(fd, &next_key, &h);
    if (err < 0) {
      fprintf(stderr, "failed to lookup hist: %d\n", err);
      mrb_sys_fail(mrb, "Error");
    }
    printf("pid: %d\n", next_key);
    for(int i = 0; i < MAX_SLOTS; i++)
      printf("\tvalue: hist[%d] = %d\n", i, h.slots[i]);

    lookup_key = next_key;
  }
  return mrb_true_value();
}

mrb_value mrb_libbpf_map_next_key(mrb_state *mrb, mrb_value self)
{
  if(DATA_PTR(self) == NULL)
    mrb_sys_fail(mrb, "Invalidly initialized map");

  int fd = ((mrb_libbpf_map_data *)DATA_PTR(self))->fd;
  int klen = ((mrb_libbpf_map_data *)DATA_PTR(self))->key_size;

  const char* key;
  char* nextkey;
  mrb_int keylen;
  mrb_get_args(mrb, "s!", &key, &keylen);
  if (key == NULL && keylen == 0) {
    key = (void *)(&get_first);
    keylen = klen;
  }
  if (keylen != (mrb_int)klen) {
    mrb_sys_fail(mrb, "keysize unmatch");
  }

  nextkey = mrb_calloc(mrb, 1, keylen);

  if(bpf_map_get_next_key(fd, key, nextkey) == -1)
    return mrb_nil_value();

  mrb_value s = mrb_str_new(mrb, nextkey, keylen);
  mrb_free(mrb, nextkey);
  return s;
}

mrb_value mrb_libbpf_map_lookup(mrb_state *mrb, mrb_value self)
{
  if(DATA_PTR(self) == NULL)
    mrb_sys_fail(mrb, "Invalidly initialized map");

  int fd = ((mrb_libbpf_map_data *)DATA_PTR(self))->fd;
  int klen = ((mrb_libbpf_map_data *)DATA_PTR(self))->key_size;
  int vlen = ((mrb_libbpf_map_data *)DATA_PTR(self))->value_size;

  const char* key;
  char* value = mrb_malloc(mrb, vlen);
  mrb_int keylen;
  mrb_get_args(mrb, "s", &key, &keylen);
  if (keylen != (mrb_int)klen) {
    mrb_sys_fail(mrb, "keysize unmatch");
  }

  int err = bpf_map_lookup_elem(fd, key, value);
  if (err < 0) {
    // mrb_sys_fail(mrb, "no element found");
    return mrb_nil_value();
  }

  return mrb_str_new(mrb, value, vlen);
}

mrb_value mrb_libbpf_map_do_update(mrb_state *mrb, mrb_value self)
{
  if(DATA_PTR(self) == NULL)
    mrb_sys_fail(mrb, "Invalidly initialized map");

  int fd = ((mrb_libbpf_map_data *)DATA_PTR(self))->fd;
  int klen = ((mrb_libbpf_map_data *)DATA_PTR(self))->key_size;
  int vlen = ((mrb_libbpf_map_data *)DATA_PTR(self))->value_size;
  const char *key, *value;
  mrb_int keylen, valuelen;

  mrb_get_args(mrb, "ss", &key, &keylen, &value, &valuelen);
  if (keylen != (mrb_int)klen) {
    mrb_sys_fail(mrb, "keysize unmatch");
  }
  if (valuelen != (mrb_int)vlen) {
    mrb_sys_fail(mrb, "valuesize unmatch");
  }

  int err = bpf_map_update_elem(fd, key, value, 0);
  if (err < 0) {
    mrb_sys_fail(mrb, "failed to update elem");
  }

  return mrb_str_new(mrb, value, vlen);
}

mrb_value mrb_libbpf_map_do_delete(mrb_state *mrb, mrb_value self)
{
  if(DATA_PTR(self) == NULL)
    mrb_sys_fail(mrb, "Invalidly initialized map");

  int fd = ((mrb_libbpf_map_data *)DATA_PTR(self))->fd;
  int klen = ((mrb_libbpf_map_data *)DATA_PTR(self))->key_size;
  const char* key;
  mrb_int keylen;

  mrb_get_args(mrb, "s", &key, &keylen);
  if (keylen != (mrb_int)klen) {
    mrb_sys_fail(mrb, "keysize unmatch");
  }

  int err = bpf_map_delete_elem(fd, key);
  if (err < 0) {
    mrb_sys_fail(mrb, "failed to delete elem");
  }

  return mrb_str_new(mrb, key, klen);
}

void mrb_libbpf_map_init(mrb_state *mrb, struct RClass *m)
{
  struct RClass *c = mrb_define_class_under(mrb, m, "Map", mrb->object_class);
  MRB_SET_INSTANCE_TT(c, MRB_TT_DATA);

  mrb_define_method(mrb, c, "next_key", mrb_libbpf_map_next_key, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, c, "lookup", mrb_libbpf_map_lookup, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, c, "do_update", mrb_libbpf_map_do_update, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, c, "do_delete", mrb_libbpf_map_do_delete, MRB_ARGS_REQ(1));

  mrb_define_method(mrb, c, "__debug", mrb_libbpf_map__debug, MRB_ARGS_NONE());
  /* mrb_define_class_method(mrb, libbpf, "hi", mrb_libbpf_hi, MRB_ARGS_NONE()); */
  DONE;
}
