/*
** mrb_libbpf.c - Libbpf class
**
** Copyright (c) Uchio Kondo 2021
**
** See Copyright Notice in LICENSE
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "mrb_libbpf.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

int mrb_libbpf_print_fn(enum libbpf_print_level level,
        const char *format, va_list args)
{
  if (level == LIBBPF_DEBUG)
    return 0;
  return vfprintf(stderr, format, args);
}



void mrb_mruby_libbpf_gem_init(mrb_state *mrb)
{
  libbpf_set_print(mrb_libbpf_print_fn);

  struct RClass *m = mrb_define_module(mrb, "BPF");
  mrb_libbpf_map_init(mrb, m);

  DONE;
}

void mrb_mruby_libbpf_gem_final(mrb_state *mrb)
{
}
