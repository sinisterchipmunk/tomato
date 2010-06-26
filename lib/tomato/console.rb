class Tomato
  # An interactive console for running either Ruby or JavaScript code.
  # You can instantiate this at the command line (for instance, via IRB or a Rake task)
  # like so:
  #
  #   require 'tomato'
  #   Tomato::Console.new
  #     #=> Console started.
  #
  # Or you can invoke it programmatically by overriding the input and output streams,
  # which is useful for writing unit tests.
  #
  # See below for more information.
  class Console
    # Returns the current input mode (:js or :rb)
    attr_reader :input_mode
    
    # Sets up and starts this console. The console exits when either the input stream
    # is closed or the 'exit' command has been encountered.
    #
    # The input argument defaults to STDIN, but can be set to a stream or a String
    # to run a specific set of commands.
    #
    # The output argument defualts to STDOUT but can be replaced with another stream
    # to capture the output, or set to nil to silence all output.
    #
    # If the debug flag is set to true, the console will log all input and all output
    # to STDERR ($stderr).
    #
    def initialize(input = $stdin, output = $stdout, debug = false)
      @debug = debug
      @input = input
      @output = output
      banner
      command('js')
      process
    end
    
    # Processes a specific set of commands. Arguments are identical to those in the
    # initializer. Note that you can run #process on a console even after it has
    # already exited; doing so is just like re-starting the console. Useful for
    # running unit tests at incremental stages.
    #
    def process(input = @input, output = @output, debug = @debug)
      input = StringIO.new(input) if input.kind_of?(String)
      output = StringIO.new(output) if output.kind_of?(String)
      @old_output = @output
      @output = output
      catch(:exit) do
        cmd = ""
        while !input.closed?
          print "> "
          line = input.gets
          if debug
            $stderr.puts line
          end
          throw :exit if line.nil?

          line = line.strip
          if !line.blank?
            if (line = line.strip)[-1] == '\\' # defer to next line
              cmd.concat line[0...-1] + ' '
            else
              cmd.concat line
              command(cmd)
              cmd = ''
            end
          end
        end
      end
      @output = @old_output
      nil
    end
    
    # Prints the "Tomato Interactive Console" banner to the current output.
    def banner
      3.times { puts }
      puts "  >> Tomato Interactive Console <<"
      puts
      help
    end

    # Parses a single command.
    def command(cmd)
      throw :exit if cmd == 'exit'
      case cmd.downcase
        when 'js' then switch_to :js, "JavaScript"
        when 'rb' then switch_to :rb, "Ruby"
        when 'help' then help
        else execute(cmd)
      end
    end
    
    # Runs a single command directly within the context, ignoring Console commands
    # like 'js', 'rb', 'exit', etc.
    def execute(line)
      begin
        result = if @input_mode == :js
          namespace.instance_eval do
            tomato.run(line)
          end
        else
          namespace.instance_eval line
        end
        puts " => #{result.inspect}"
      rescue
        puts "#{$!.class}: #{$!.message}"
      end
    end
    
    # Prints a summary of how to use the console.
    def help
      puts "Type 'rb' to enter Ruby code; 'js' to enter JS code."
      puts "All code is evaluated in the global namespace, just like IRB."
      puts "You can span multiple lines by ending a line with '\\'."
      puts "Type 'exit' at any time to quit."
      puts
    end
    
    # Prints a line of text to the output stream.
    def puts(*what)
      write(:puts, *what)
    end
    
    # Prints a series of text to the output stream, without ending the line.
    def print(*what)
      write(:print, *what)
    end
    
    # Switches to the specified input mode. This should be either :js or :rb.
    # The label is displayed to the user.
    #
    # Ex:
    #  console.switch_to(:js, "JavaScript")
    #
    def switch_to(which, label = which.to_s)
      @input_mode = which
      puts "Now expecting #{label} code."
      puts
    end

    # Returns the namespace within which all Ruby commands are executed.
    # JS methods are executed on the "tomato" object in this namespace.
    def namespace
      return @namespace if @namespace
      @namespace = Object.new
      def @namespace.tomato
        @tomato ||= Tomato.new
      end
      @namespace
    end
    
    private
    def write(method_name, *what)
      if @debug && !$stderr.closed?
        $stderr.send(method_name, *what)
      end
      if @output && !@output.closed?
        @output.send(method_name, *what)
      end
    end
  end
end
