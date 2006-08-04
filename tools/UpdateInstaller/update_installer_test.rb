require 'test/unit'
require 'fileutils'
require 'pathname'

class UpdateInstallerTest < Test::Unit::TestCase
  EXE_PATH="../../../Win32/Bin/UpdateInstaller.exe"

  def setup
    FileUtils.rm_f "Updates/temp/log"
  end

  def test_exe_exists
    assert_exists EXE_PATH
  end

  def test_install
    assert system(EXE_PATH, ".")
    assert_exists "sub/quux.txt"
    assert_file_equals "", "sub/quux.txt"
    assert_file_equals "foo\r\n", "foo.txt"
    assert_file_equals "foo\r\n", "sub/foo.txt"
    assert_files_equal "Updates/release.spec", "release.spec"
    assert_files_equal("Updates/manifests/update/MANIFEST.base", 
                       "MANIFEST.base")
    assert_files_equal("Updates/manifests/update/MANIFEST.sub", 
                       "MANIFEST.sub")
    assert_file_equals <<EOF, "Updates/temp/log"
Checking if install is possible.\r
Install is possible; beginning install.\r
Update installed successfully. Relaunching.\r
EOF
  end

  def test_failed_install
    FileUtils.mv("Updates/pool/855426068ee8939df6bce2c2c4b1e7346532a133",
                 "Updates/pool/temp")
    assert !system(EXE_PATH, ".")
    assert_file_equals "", "foo.txt"
    assert_file_equals "", "sub/foo.txt"
    assert !File.exists?("sub/quux.txt")
    FileUtils.mv("Updates/pool/temp",
                 "Updates/pool/855426068ee8939df6bce2c2c4b1e7346532a133")
    assert_file_equals <<EOF, "Updates/temp/log"
Checking if install is possible.\r
Update is impossible; relaunching.\r
EOF
  end

  def assert_exists file
    assert File.exists?(file), "#{file} does not exist"
  end

  def assert_file_equals contents, file
    pn = Pathname.new(file)
    assert_equal contents, pn.read
  end
  
  def assert_files_equal file1, file2
    pn = Pathname.new(file1)
    assert_file_equals pn.read, file2
  end
end