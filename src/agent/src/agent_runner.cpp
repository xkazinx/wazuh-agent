#include <agent_runner.hpp>

#include <config.h>
#include <iostream>
#include <logger.hpp>
#include <process_options.hpp>
#include <restart_handler.hpp>

/// Command-line options
static const auto OPT_HELP {"help"};
static const auto OPT_RUN {"run"};
static const auto OPT_STATUS {"status"};
static const auto OPT_CONFIG_FILE {"config-file"};
static const auto OPT_REGISTER_AGENT {"register-agent"};
static const auto OPT_URL {"url"};
static const auto OPT_USER {"user"};
static const auto OPT_PASS {"password"};
static const auto OPT_KEY {"key"};
static const auto OPT_NAME {"name"};
static const auto OPT_VERIFICATION_MODE {"verification-mode"};

namespace program_options = boost::program_options;

AgentRunner::AgentRunner(int argc, char* argv[])
{
    restart_handler::RestartHandler::SetCommandLineArguments(argc, argv);
    ParseOptions(argc, argv);
}

void AgentRunner::ParseOptions(int argc, char* argv[])
{
    cmdParser.add_options()(OPT_HELP, "Display this help menu")(
        OPT_RUN, "Run agent in foreground (this is the default behavior)")(
        OPT_STATUS, "Check if the agent is running (running or stopped)")(
        OPT_CONFIG_FILE,
        program_options::value<std::string>()->default_value(""),
        "Path to the Wazuh configuration file (optional)")(OPT_REGISTER_AGENT,
                                                           "Use this option to register as a new agent")(
        OPT_URL, program_options::value<std::string>(), "URL of the server management API")(
        OPT_USER, program_options::value<std::string>(), "User to authenticate with the server management API")(
        OPT_PASS, program_options::value<std::string>(), "Password to authenticate with the server management API")(
        OPT_KEY, program_options::value<std::string>()->default_value(""), "Key to register the agent (optional)")(
        OPT_NAME, program_options::value<std::string>()->default_value(""), "Name to register the agent (optional)")(
        OPT_VERIFICATION_MODE,
        program_options::value<std::string>()->default_value(config::agent::DEFAULT_VERIFICATION_MODE),
        "Verification mode to be applied on HTTPS connection to the server (optional)");

    AddPlatformSpecificOptions();

    program_options::store(program_options::parse_command_line(argc, argv, cmdParser), validOptions);
    program_options::notify(validOptions);
}

int AgentRunner::Run() const
{
    if (validOptions.count(OPT_REGISTER_AGENT))
    {
        return RegisterAgent();
    }
    if (validOptions.count(OPT_STATUS))
    {
        StatusAgent(validOptions[OPT_CONFIG_FILE].as<std::string>());
    }
    else if (const auto platformSpecificResult = HandlePlatformSpecificOptions(); platformSpecificResult.has_value())
    {
        return platformSpecificResult.value();
    }
    else if (validOptions.count(OPT_HELP))
    {
        std::cout << cmdParser << '\n';
    }
    else
    {
        StartAgent(validOptions[OPT_CONFIG_FILE].as<std::string>());
    }

    return 0;
}

int AgentRunner::RegisterAgent() const
{
    if (!validOptions.count(OPT_URL) || !validOptions.count(OPT_USER) || !validOptions.count(OPT_PASS))
    {
        std::cout << "--url, --user and --password args are mandatory\n";
        return 1;
    }

    ::RegisterAgent(validOptions[OPT_URL].as<std::string>(),
                    validOptions[OPT_USER].as<std::string>(),
                    validOptions[OPT_PASS].as<std::string>(),
                    validOptions[OPT_KEY].as<std::string>(),
                    validOptions[OPT_NAME].as<std::string>(),
                    validOptions[OPT_CONFIG_FILE].as<std::string>(),
                    validOptions[OPT_VERIFICATION_MODE].as<std::string>());

    return 0;
}
