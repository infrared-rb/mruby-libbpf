MRUBY_CONFIG=File.expand_path(ENV["MRUBY_CONFIG"] || "build_config/local.rb")
MRUBY_VERSION=ENV["MRUBY_VERSION"] || "3.0.0"

file :mruby do
  Dir.chdir ENV['HOME'] do
    sh "git clone --depth=1 git://github.com/mruby/mruby.git libbpf-mruby"
    if MRUBY_VERSION != 'master'
      Dir.chdir 'libbpf-mruby' do
        sh "git fetch --tags"
        rev = %x{git rev-parse #{MRUBY_VERSION}}
        sh "git checkout #{rev}"
      end
    end
  end

  sh 'ln -s ~/libbpf-mruby mruby'
end

desc "compile binary"
task :compile => :mruby do
  sh "cd mruby && rake all MRUBY_CONFIG=#{MRUBY_CONFIG}"
end

desc "test"
task :test => :mruby do
  sh "cd mruby && rake all test MRUBY_CONFIG=#{MRUBY_CONFIG}"
end

desc "cleanup"
task :clean do
  exit 0 unless File.directory?('mruby')
  sh "cd mruby && rake deep_clean"
end

task :default => :compile
