#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>

struct Args
{
    size_t mins_before_sleep;
    size_t mins_before_reduce_volume;
};

std::optional<Args> parseArgs(int argc, char** argv)
{
    if(argc < 3)
        return std::nullopt;

    Args args;
    args.mins_before_sleep = std::atoi(argv[1]);
    args.mins_before_reduce_volume = std::atoi(argv[2]);

    return args;
}

bool exec(const char* cmd, std::string* result)
{
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

    if(!pipe)
        return false;

    if(!result)
        return true;

    std::array<char, 256> buffer;

    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        *result += buffer.data();

    return true;
}

// \note If we can't get current volume for any reason, returns 0 so the sound will be shutted down
size_t getVolume()
{
    std::string output;

    if(!exec("amixer sget Master | grep \"Mono: Playback\"", &output))
        return 0;

    auto pos = output.find("%]");

    if(pos == std::string::npos)
        return 0;

    output = output.substr(0, pos);
    pos = output.rfind("[");

    if(pos == std::string::npos)
        throw "Can't parse volume";

    output = output.substr(pos + 1);

    return stoi(output);
}

void decreaseVolume(size_t volume, size_t remaining_time_ms)
{
    if(volume == 0)
        return;

    const std::string prefix = "amixer set Master ";
    const std::string suffix = "%";
    auto nbr_ticks = volume;
    auto decrease_tick = remaining_time_ms / volume;

    std::cout << "Reducing volume to 0 in " << remaining_time_ms << " ms." << std::endl;

    std::string cmd;
    cmd.reserve(prefix.length() + suffix.length() + 2);

    for(size_t i = 0; i < nbr_ticks; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(decrease_tick));
        --volume;

        cmd = prefix + std::to_string(volume) + suffix;

        if(!exec(cmd.data(), nullptr))
            std::cerr << "Error while decreasing volume." << std::endl;
    }
}

void putToSleep(size_t mins_before_sleep, size_t mins_before_reduce_volume)
{
    std::cout << "Waiting " << mins_before_reduce_volume << " min(s) before reducing volume." << std::endl;
    std::this_thread::sleep_for(std::chrono::minutes(mins_before_reduce_volume));

    auto volume = getVolume();
    auto remaining_time_ms = (mins_before_sleep - mins_before_reduce_volume) * 60000;
    decreaseVolume(volume, remaining_time_ms);

    std::cout << "sleeping" << std::endl;

    if(!exec("systemctl suspend", nullptr))
        std::cerr << "Error while suspending." << std::endl;
}

int main(int argc, char** argv)
{
    auto args_opt = parseArgs(argc, argv);

    if(!args_opt)
    {
        std::cerr << "Invalid arguments" << std::endl;
        return EXIT_FAILURE;
    }

    const auto& args = *args_opt;
    putToSleep(args.mins_before_sleep, args.mins_before_reduce_volume);

    return EXIT_SUCCESS;
}
