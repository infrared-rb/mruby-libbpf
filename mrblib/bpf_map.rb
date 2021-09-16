module BPF
  class Map
    def key_packer
      @key_packer ||= lambda {|v| v}
    end

    def key_unpacker
      @key_unpacker ||= lambda {|v| v}
    end

    def value_packer
      @value_packer ||= lambda {|v| v}
    end

    def value_unpacker
      @value_unpacker ||= lambda {|v| v}
    end

    def key_packer=(blk)
      @key_packer = blk
    end

    def key_unpacker=(blk)
      @key_unpacker = blk
    end

    def value_packer=(blk)
      @value_packer = blk
    end

    def value_unpacker=(blk)
      @value_unpacker = blk
    end

    def key_type=(type)
      if type == Integer
        self.key_packer = lambda{|k| [k].pack("I") }
        self.key_unpacker = lambda{|k| k.unpack("I")[0] }
      end
    end

    def value_type=(type)
      if type == Integer
        self.value_packer = lambda{|v| [v].pack("I") }
        self.value_unpacker = lambda{|v| v.unpack("I")[0] }
      end
    end

    def value_pack_format=(fmt)
      self.value_packer = lambda{|v|
        v_ = v.is_a?(Array) ? v : [v]
        v_.pack(fmt)
      }
      self.value_unpacker = lambda{|v| v.unpack(fmt) }
    end

    def each
      key = nil
      ret = []

      while key = next_key(key)
        data = lookup(key)
        value = value_unpacker.call(data)
        kv = [key_unpacker.call(key), value]
        yield *kv
        ret << kv
      end

      ret
    end

    def keys
      r = []
      key = nil
      while key = next_key(key)
        r << key_unpacker.call(key)
      end
      r
    end

    def to_h
      ret = {}
      each {|k, v| ret[k] = v }
      ret
    end
    alias items to_h

    def values
      to_h.values
    end

    def [](key)
      key_ = key_packer.call(key)
      v = lookup(key_)
      if v
        value_unpacker.call(v)
      else
        nil
      end
    end

    def []=(key, value)
      key_ = key_packer.call(key)
      value_ = value_packer.call(value)
      v = lookup(key_)
      if v
        value_unpacker.call(v)
      else
        nil
      end
    end

    def delete(key)
      key_ = key_packer.call(key)
      do_delete(key_)
      key
    end

    def clear
      keys.each {|k| delete(k )}
    end

    include Enumerable
  end
end
