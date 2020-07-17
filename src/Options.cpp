#include <Options.hpp>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <exception>

#include <limits.h>
#include <unistd.h>

// _test_mani update name of configuration generator

const std::string Options::helpString_base =
    "Usage: MPIRUN [MPIOPT]... -- mpidb [OPTION]... [--] APP [APPOPT]...\n"
    "Debug multiple processes of MPI application APP.  For each MPI rank, an\n"
    "individual instance of gdbserver is spawned.\n\n"
    " MPIRUN is the MPI scheduler (mpirun, mpiexec, ...) and MPIOPT are its\n"
    "   options and arguments.\n"
    " APP is the MPI executable and APPOPT are its options and arguments.\n"
    " Information regarding the gdbserver instances is written to the standard\n"
    " output. It can be piped to mpidbc.py to generate configuration files.\n\n"
    "Options\n"
    " -h, --help\t\tDisplay this help text and exit\n"
    " -p, --port=PORT\tPort of each rank is PORT + rank ID, default=13370\n"
    " -r, --ranks=RANKS\tComma-seperated list of rank IDs to be debugged\n"
    "\t\t\tMust not include spaces; rank IDs always start from 0\n"
    "\t\t\tCan include ranges of the form a-b\n"
    "\t\t\tExample: 2-5,9,7 specifies ranks 2, 3, 4, 5, 7 and 9\n"
    " -g, --gdbserver=FILE\tPath to the gdbserver executable, default=gdbserver\n";


void getMPIInfo(int & numRanks, int & rank)
{
    auto numRanksPtr = getenv("OMPI_COMM_WORLD_SIZE");
    auto rankPtr = getenv("OMPI_COMM_WORLD_RANK");

    if (numRanksPtr == nullptr || rankPtr == nullptr)
    {
        numRanksPtr = getenv("PMI_SIZE");
        rankPtr = getenv("PMI_RANK");

        if (numRanksPtr == nullptr || rankPtr == nullptr)
            throw EnvException("Could not find MPI rank and size");
    }

    try
    {
        numRanks = std::stoi(numRanksPtr);
        rank = std::stoi(rankPtr);
    }
    catch (std::invalid_argument & e)
    {
        throw EnvException("MPI rank or size could not be interpreted as int: " + std::string(numRanksPtr) + ", " + std::string(rankPtr));
    }

    if (numRanks <= 0)
        throw EnvException("MPI size is negative: " + std::string(numRanksPtr));
    if (rank < 0)
        throw EnvException("MPI rank is negative: " + std::string(rankPtr));
    if (rank >= numRanks)
        throw EnvException("MPI rank is too large: " + std::string(rankPtr) + ", " + std::string(numRanksPtr));
}


Options::Options(int argc, char ** argv)
{
    // First, scan for --help
    for (int argIter = 1; argIter < argc; ++argIter)
    {
        auto option = ParseOption(argv[argIter]);

        if (option == Option::help)
        {
            help = true;
            helpString = helpString_base;
            return; // Quick and dirty return for fast exit
        }

        if (option == Option::nonOption || option == Option::stopParse)
            break;
    }

    // If no --help was found, begin parsing of options

    if (argc < 2) throw OptionsException("APP not specified");

    getMPIInfo(numRanks, thisRank);

    char hostnameC[HOST_NAME_MAX];
    gethostname(hostnameC, HOST_NAME_MAX);
    hostname = hostnameC;
    
    numDebugRanks = numRanks;

    int appArgPos;

    for (int argIter = 1; argIter < argc; ++argIter)
    {
        auto option = ParseOption(argv[argIter]);

        if (argIter + 1 >= argc && option != Option::nonOption)
            throw OptionsException("APP not specified");

        if (option == Option::invalid)
            throw OptionsException("Invalid option: " + std::string(argv[argIter]), argIter);

        if (option == Option::stopParse)
        {
            if (argIter + 1 >= argc) throw OptionsException("APP not specified");
            appArgPos = argIter + 1;
            break;
        }

        if (option == Option::nonOption)
        {
            appArgPos = argIter;
            break;
        }

        try
        {
            switch (option)
            {
                case Option::port:
                    startPort = std::stoi(argv[++argIter]);
                    if (startPort <= 0)
                        throw std::out_of_range("Start port must be positive: " + std::to_string(startPort));
                    break;
                case Option::ranks:
                    SetDebugRanks(argv[++argIter]);
                    break;
                case Option::gdbserver:
                    gdbserverPath = argv[++argIter];
                    break;
            }
        }
        catch (std::invalid_argument & e)
        {
            throw OptionsException(e.what(), argIter);
        }
        catch (std::out_of_range & e)
        {
            throw OptionsException(e.what(), argIter);
        }
    }

    port = startPort + thisRank;
    appName = argv[appArgPos];
    GenerateExecArgs(argc, argv, appArgPos);
}

void Options::GenerateExecArgs(int argc, char ** argv, int appArgPos)
{
    execArgs = std::vector<char *>();

    if (debugThisRank)
    {
        debuggerArgs = { gdbserverPath, hostname + ":" + std::to_string(port) };
        for (auto & arg : debuggerArgs)
            execArgs.push_back(& arg[0]);
    }
    
    for (auto argPtr = argv + appArgPos; argPtr != argv + argc; ++argPtr)
        execArgs.push_back(* argPtr);
    execArgs.push_back(nullptr);
}

void Options::SetDebugRanks(const std::string & argument)
{
    std::string token;
    std::stringstream stream(argument);

    numDebugRanks = 0;
    std::vector<bool> rankBuffer(numRanks, false);

    while (std::getline(stream, token, ','))
    {
        auto dashPos = token.find_first_of('-');
        if (dashPos == std::string::npos)
        {
            int rank = std::stoi(token);

            if (rank < 0 || rank >= numRanks)
                throw std::out_of_range("Rank out of range: " + std::to_string(rank));

            if (! rankBuffer[rank])
            {
                rankBuffer[rank] = true;
                debugThisRank = rank == thisRank;
                ++numDebugRanks;
            }
        }
        else
        {
            int startRank = std::stoi(token.substr(0, dashPos));
            int endRank = std::stoi(token.substr(dashPos + 1));

            if (startRank > endRank) std::swap(startRank, endRank);

            if (startRank < 0 || startRank >= numRanks)
                throw std::out_of_range("Rank out of range: " + std::to_string(startRank));
            if (endRank < 0 || endRank >= numRanks)
                throw std::out_of_range("Rank out of range: " + std::to_string(endRank));

            for (int rank = startRank; rank <= endRank; ++rank)
            {
                if (! rankBuffer[rank])
                {
                    rankBuffer[rank] = true;
                    debugThisRank = rank == thisRank;
                    ++numDebugRanks;
                }
            }
        }
    }
}

Options::Option Options::ParseOption(const std::string & word)
{
    if (word.size() == 0)
        throw std::invalid_argument("Argument size 0");

    if (word[0] != '-')
        return Option::nonOption;

    if (word.size() == 1)
        return Option::invalid;

    if (word[1] == '-')
    {
        if (word.size() == 2)
            return Option::stopParse;

        auto opt = word.substr(2);
        if (opt == "help")
            return Option::help;
        if (opt == "port")
            return Option::port;
        if (opt == "ranks")
            return Option::ranks;
        if (opt == "gdbserver")
            return Option::gdbserver;
        return Option::invalid;
    }
    
    if (word.size() > 2)
        return Option::invalid;
    
    if (word[1] == 'h')
        return Option::help;
    if (word[1] == 'p')
        return Option::port;
    if (word[1] == 'r')
        return Option::ranks;
    if (word[1] == 'g')
        return Option::gdbserver;

    return Option::invalid;
}


OptionsException::OptionsException(const std::string & message)
    : message("Error parsing command-line arguments: " + message + "\nFor more information, use --help.")
{}

OptionsException::OptionsException(const std::string & message, int argNum)
    : message("Error parsing command-line argument #" + std::to_string(argNum) + ": " + message + "\nFor more information, use --help.")
{}

const char * OptionsException::what() const noexcept
{
    return message.c_str();
}



EnvException::EnvException(const std::string & message)
    : message("Environment error: " + message)
{}

const char * EnvException::what() const noexcept
{
    return message.c_str();
}