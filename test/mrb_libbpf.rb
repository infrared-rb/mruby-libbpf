##
## Libbpf Test
##

assert("Libbpf#hello") do
  t = Libbpf.new "hello"
  assert_equal("hello", t.hello)
end

assert("Libbpf#bye") do
  t = Libbpf.new "hello"
  assert_equal("hello bye", t.bye)
end

assert("Libbpf.hi") do
  assert_equal("hi!!", Libbpf.hi)
end
