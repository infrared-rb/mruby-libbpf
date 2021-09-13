/*
** mrb_libbpf.h - Libbpf class
**
** See Copyright Notice in LICENSE
*/

#ifndef MRB_LIBBPF_H
#define MRB_LIBBPF_H

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>

void mrb_libbpf_map_init(mrb_state *mrb, struct RClass *m);
void mrb_mruby_libbpf_gem_init(mrb_state *mrb);

#endif
