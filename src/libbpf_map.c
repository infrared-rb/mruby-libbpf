#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/value.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "mrb_libbpf.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

typedef struct {
  mrb_int fd;
  mrb_int key_size;
  mrb_int value_size;
  enum bpf_map_type type;
  struct bpf_map *ptr;
} mrb_libbpf_map_data;

static const struct mrb_data_type mrb_libbpf_map_data_type = {
  "mrb_libbpf_map_data", mrb_free,
};

mrb_value mrb_libbpf_generate_map(mrb_state *mrb,
                                  mrb_int fd,
                                  mrb_int key_size,
                                  mrb_int value_size,
                                  enum bpf_map_type type,
                                  struct bpf_map *ptr)
{
  struct RClass *m = mrb_module_get(mrb, "BPF");
  struct RClass *c = mrb_class_get_under(mrb, m, "Map");
  mrb_value self = mrb_obj_new(mrb, c, 0, NULL);
  DATA_TYPE(self) = &mrb_libbpf_map_data_type;

  mrb_libbpf_map_data *data = mrb_malloc(mrb, sizeof(mrb_libbpf_map_data));
  data->fd = fd;
  data->key_size = key_size;
  data->value_size = value_size;
  data->type = type;
  data->ptr = ptr;
  DATA_PTR(self) = data;

  return self;
}

void mrb_libbpf_map_init(mrb_state *mrb, struct RClass *m)
{
  struct RClass *c = mrb_define_class_under(mrb, m, "Map", mrb->object_class);
  MRB_SET_INSTANCE_TT(c, MRB_TT_DATA);
  /* mrb_define_method(mrb, libbpf, "initialize", mrb_libbpf_init, MRB_ARGS_REQ(1)); */
  /* mrb_define_method(mrb, libbpf, "hello", mrb_libbpf_hello, MRB_ARGS_NONE()); */
  /* mrb_define_class_method(mrb, libbpf, "hi", mrb_libbpf_hi, MRB_ARGS_NONE()); */
  DONE;
}
