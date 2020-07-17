#include <iostream>
#include <string>

#include <unistd.h>

#include <Options.hpp>

int main(int argc, char ** argv)
{
    try
    {
        Options options(argc, argv);

        if (options.help)
        {
            std::cout << options.helpString << std::endl;
            return 0;
        }

        if (options.numDebugRanks == 0)
        {
            if (options.thisRank == 0)
                std::cerr << "Warning: No ranks selected for debugging." << std::endl;

            if (execvp(options.execArgs.at(0), options.execArgs.data()) == -1)
            {
                std::cerr << "Error executing application " << options.execArgs[0] << ", error " << errno << std::endl;
                return -1;
            }
        }

        std::string header = "mpidb(" + std::to_string(options.thisRank) + "): ";

        // Print base info atomically and flush stream
        if (options.thisRank == 0)
            std::cout << header + options.appName + " " + std::to_string(options.numDebugRanks) << std::endl;
        
        // Print rank info atomically and flush stream
        if (options.debugThisRank)
        {
            std::cout << header + options.hostname + " " + std::to_string(options.port) << std::endl;
        }

        // Try to execute the debugger, launching the app on each rank to be debugged
        if (execvp(options.execArgs[0], options.execArgs.data()) == -1)
        {
            // If there was an error return the error number
            std::cerr << "Error executing application " << options.execArgs[0] << ", error " << errno << std::endl;
            return -1;
        }
    }
    catch (OptionsException & e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (EnvException & e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (std::exception & e)
    {
        std::cerr << "Caught exception: " << e.what() << ".\nThis may be a bug." << std::endl;
        return -1;
    }

    return 0;
}