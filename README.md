# mruby-libbpf [![CI](https://github.com/infrared-rb/mruby-libbpf/actions/workflows/ci.yml/badge.svg)](https://github.com/infrared-rb/mruby-libbpf/actions/workflows/ci.yml)

libbpf binding for mruby

## install by mrbgems

- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :github => 'udzura/mruby-libbpf'
end
```

## Supported feature

* [ ] Skelton
* [x] BPF Maps
* [ ] Perf buffer poller

## License

under the MIT License:

- see LICENSE file
