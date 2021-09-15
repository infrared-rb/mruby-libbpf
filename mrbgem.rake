require 'open3'
require 'fileutils'
require_relative 'mrblib/libbpf/versions'

unless defined? DEFAULT_LIBBPF_VERSION
  DEFAULT_LIBBPF_VERSION = LibBPF::LIBBPF_VERSION
end

MRuby::Gem::Specification.new('mruby-libbpf') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Uchio Kondo'
  spec.version = LibBPF::VERSION

  def run_command env, command
    STDOUT.sync = true
    puts "EXEC\t[mruby-libbpf] #{command}"
    Open3.popen2e(env, command) do |stdin, stdout, thread|
      print stdout.read
      fail "#{command} failed" if thread.value != 0
    end
  end

  def spec.get_libbpf_version
    if self.cc.defines.flatten.find{|d| d =~ /^MRB_LIBBPF_VERSION=([\.0-9]+)$/ }
      return $1
    else
      DEFAULT_LIBBPF_VERSION
    end
  end

  def spec.bundle_libbpf
    version = get_libbpf_version

    def lib_dir(b); "#{b.build_dir}/vendor/libbpf"; end
    def objs_dir(b); "#{lib_dir(b)}/.objs"; end
    def header(b); "#{lib_dir(b)}/include/libbpf.h"; end
    def lib_a(b); libfile "#{objs_dir(b)}/lib/libbpf"; end

    task :clean do
      FileUtils.rm_rf [lib_dir(build)]
    end

    file "#{lib_dir(build)}/src/bpf.c" do
      unless File.exist? lib_dir(build)
        tmpdir = ENV['TMPDIR'] || '/tmp'
        run_command ENV, "rm -rf #{tmpdir}/libbpf-#{version}"
        run_command ENV, "mkdir -p #{File.dirname(lib_dir(build))}"
        run_command ENV, "curl -L https://github.com/libbpf/libbpf/archive/refs/tags/v#{version}.tar.gz | tar -xz -f - -C #{tmpdir}"
        run_command ENV, "mv -f #{tmpdir}/libbpf-#{version} #{lib_dir(build)}"
      end
    end

    file lib_a(build) => "#{lib_dir(build)}/src/bpf.c" do
      sh "mkdir -p #{objs_dir(build)} #{File.dirname(lib_a(build))}"
      Dir.chdir lib_dir(build) + "/src" do
        e = ENV
        e['BUILD_STATIC_ONLY'] = 'y'
        e['PREFIX'] = objs_dir(build)
        e['LIBDIR'] = ''
        e['OBJDIR'] = objs_dir(build)
        e['DESTDIR'] = File.dirname(lib_a(build))
        e['CFLAGS'] = '-g -O2 -Werror -Wall -fPIC'
        # e['LDFLAGS'] = '-lelf'

        run_command e, "make"
        run_command e, "make install"
      end
    end

    libmruby_a = libfile("#{build.build_dir}/lib/libmruby")
    file libmruby_a => lib_a(build)

    self.cc.include_paths << File.dirname(header(build))
    self.linker.library_paths << File.dirname(lib_a(build))
    self.linker.libraries << 'bpf' << 'z' << 'elf'
  end

  spec.bundle_libbpf

end
