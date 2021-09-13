# mruby-libbpf   [![Build Status](https://travis-ci.org/udzura/mruby-libbpf.svg?branch=master)](https://travis-ci.org/udzura/mruby-libbpf)
Libbpf class
## install by mrbgems
- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :github => 'udzura/mruby-libbpf'
end
```
## example
```ruby
p Libbpf.hi
#=> "hi!!"
t = Libbpf.new "hello"
p t.hello
#=> "hello"
p t.bye
#=> "hello bye"
```

## License
under the MIT License:
- see LICENSE file
