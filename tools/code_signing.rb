# Support for singing code using Authenticode.  This only works with
# Cygwin Ruby under Windows, AFAIK.
module CodeSigning

  #========================================================================
  #  Password prompting
  #========================================================================
  
  # This section of termios-related code is taken verbatim from one of my
  # personal projects, and I hereby place it into the public domain.
  # -Eric Kidd, 14 Nov 2007
  begin
    require 'termios'
    
    # Code for reading a password from the console without echo.  Adapted from
    # the termios gem documentation, and apparently available at
    # [ruby-list:15968].  This code doesn't stand a chance of working on
    # Windows, unfortunately.
    def self.gets_secret
      oldt = Termios.tcgetattr($stdin)
      newt = oldt.dup
      newt.lflag &= ~Termios::ECHO
      Termios.tcsetattr($stdin, Termios::TCSANOW, newt)
      result = $stdin.gets
      Termios.tcsetattr($stdin, Termios::TCSANOW, oldt)
      print "\n"
      result.chomp
    end
    HIDDEN_PASSWORDS = true
  rescue MissingSourceFile
    def self.gets_secret
      STDIN.gets.chomp
    end
    HIDDEN_PASSWORDS = false
  end


  #========================================================================
  #  Code signing
  #========================================================================

  # The default timestamp service.  Using this allows our signatures to
  # outlast our singing keys.
  DEFAULT_TIMESTAMP_URL = 'http://timestamp.verisign.com/scripts/timstamp.dll'

  # Sign a file with the specified key and other parameters.
  def self.sign_file file, options
    key_file        = options[:key_file] || raise('Must specify key_file')
    password        = options[:password] || raise('Must specify password')
    description     = options[:description]
    description_url = options[:description_url]
    timestamp_url   = options[:timestamp_url] || DEFAULT_TIMESTAMP_URL
    
    # signtool /f <key_file> /p <pass> /d <desc> /du <desc_url>
    #          /t <timestamp_url>
    # Returns 0 on success, 1 on failure and 2 on warning
    flags  = ['/q', '/f', key_file, '/p', password, '/t', timestamp_url]
    flags += ['/d',  description]     if description
    flags += ['/du', description_url] if description
    
    # Call this using 'system', so that we don't display our password
    # to the console.
    system 'signtool', 'sign', *(flags + [file]) or
      raise 'Error running signtool'
  end
  
  # Look for a specified *.pfx keyfile on the root level of any removable
  # drives.  The theory here is that our *.pfx key lives on a USB stick.
  def self.find_key name
    for drive in 'D'..'L'
      candidate = "#{drive}:/#{name}.pfx"
      if File.exists?(candidate)
        return candidate
      end
    end
    raise "Cannot find key file '#{name}.pfx' on any removable drive"
  end


  #========================================================================
  #  Rake integration
  #========================================================================
  
  # A Rake task for singing code.
  class Task < Rake::Task
    # The name of this task.
    attr_accessor :name
    # The files to sign.
    attr_accessor :files
    # The descriptions of the file to sign (optional).  This is a hash
    # table, keyed by the values of 'files'.
    attr_accessor :description
    # The description to use for a file if no entry in 'description' matches.
    attr_accessor :default_description
    # The base name of the key file to use when signing.  This will be used
    # to search removable drives for the actual key file itself.
    attr_accessor :key_file
    # A URL with more information about the file to sign (optional).
    attr_accessor :description_url
    # The URL of the timestamp server (optional).
    attr_accessor :timestamp_url
    
    # Create a new CodeSigning::Task.
    def initialize name = :sign # :yield: self
      @name = name
      @files = []
      @description = {}
      yield self if block_given?
      define
    end
    
    # Define the underlying tasks we'll need.
    def define
      task @name do
        # Get the private key and the password to unlock it.
        unless HIDDEN_PASSWORDS
          raise "Must install Ruby termios gem to sign code"
        end
        key_path = CodeSigning::find_key @key_file
        print "Password for #{key_path}: "
        password = CodeSigning::gets_secret

        # Loop over each file we need to sign.
        for file in @files
          puts "Signing #{file}"
          description = @description[file] || @default_description
          CodeSigning::sign_file(file, :key_file => key_path,
                                 :password => password,
                                 :description => description,
                                 :description_url => description_url,
                                 :timestamp_url => timestamp_url)
        end
      end
    end
  end
end
