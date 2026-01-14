#include <ctime>
#include <discord_presence/discord.h>
#include <iostream>
#include <thread>

#define CLIENT_ID 1460749513667907658
using namespace std::chrono_literals;

int main() {

    discord::Core *core{};
    discord::Result result{
        discord::Core::Create(CLIENT_ID, DiscordCreateFlags_Default, &core)};

    if (result != discord::Result::Ok) {
        std::cerr << "Error creating the SDK Discord instance\n";
        return -1;
    }

    core->SetLogHook(discord::LogLevel::Debug, [](discord::LogLevel level,
                                                  const char *message) {
        std::cout << "[Discord SDK Log]: " << message << std::endl;
    });

    discord::Activity activity{};
    activity.SetDetails("Waiting to play Minecraft Bedrock");
    activity.SetState("Testing this... 1, 2");

    activity.GetAssets().SetLargeImage(
        "trini"); // Just trinit because it's the image name
    activity.GetAssets().SetLargeText("Hi! I'm Trini");
    activity.GetAssets().SetSmallText("Playing with Trinity Launcher");

    activity.GetTimestamps().SetStart(std::time(nullptr));

    core->ActivityManager().UpdateActivity(
        activity, [](discord::Result result) {
            if (result == discord::Result::Ok)
                std::cout << "SUCCESS: Accepted the activity from Discord.\n";
            else
                std::cout << "ERROR: Failed updating the state\n";
        });

    while (true) {
        core->RunCallbacks();
        std::this_thread::sleep_for(16ms);
    }

    return 0;
}
