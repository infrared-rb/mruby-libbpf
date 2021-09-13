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
                                  int fd,
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

mrb_value mrb_libbpf_map_next_key(mrb_state *mrb, mrb_value self)
{
  if(DATA_PTR(self) == NULL)
    mrb_sys_fail(mrb, "Invalidly initialized map");

  int fd = ((mrb_libbpf_map_data *)DATA_PTR(self))->fd;
  int klen = ((mrb_libbpf_map_data *)DATA_PTR(self))->key_size;

  const char* key;
  char* nextkey;
  mrb_int keylen;
  mrb_get_args(mrb, "s", &key, &keylen);
  if (key == NULL && keylen == 0) {
    key = (void *)(&get_first);
    keylen = klen;
  }
  if (keylen != (mrb_int)klen) {
    mrb_sys_fail(mrb, "keysize unmatch");
  }

  if(!bpf_map_get_next_key(fd, &key, &nextkey))
    return mrb_nil_value();

  return mrb_str_new(mrb, nextkey, keylen);
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

  int err = bpf_map_lookup_elem(fd, &key, &value);
  if (err < 0) {
    // mrb_sys_fail(mrb, "no element found");
    return mrb_nil_value();
  }

  return mrb_str_new(mrb, value, vlen);
}

void mrb_libbpf_map_init(mrb_state *mrb, struct RClass *m)
{
  struct RClass *c = mrb_define_class_under(mrb, m, "Map", mrb->object_class);
  MRB_SET_INSTANCE_TT(c, MRB_TT_DATA);

  mrb_define_method(mrb, c, "next_key", mrb_libbpf_map_next_key, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, c, "lookup", mrb_libbpf_map_lookup, MRB_ARGS_REQ(1));
  /* mrb_define_class_method(mrb, libbpf, "hi", mrb_libbpf_hi, MRB_ARGS_NONE()); */
  DONE;
}
