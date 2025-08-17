#include <iostream>
#include <string>

static void uci_print(const std::string& str) {
    std::cout << str << "\n";
    std::cout.flush();
}

static void cmd_uci() {
    uci_print("id name Engima");
    uci_print("id author Syed Zaidi");
    uci_print("uci_ok");
}

static void cmd_setoption(const std::string& str) {
    // TODO
}

static void cmd_isready() {
    uci_print("readyok");
}

static void cmd_ucinewgame() {
    // TODO (clear game state)
}

static void cmd_position(const std::string& str) {
    
}

static void cmd_go() {
    // TODO
}

static void cmd_debug() {
    // TODO
}

static void cmd_register() {
    // TODO
}

static void cmd_ponderhit() {
    // TODO
}

static void cmd_stop() {
    // TODO
}

static void cmd_quit() {
    // TODO
}

void uci_loop() {
    // Remove sync with stdio to improve performance
    std::ios::sync_with_stdio(false);

    // Untie cin from cout to prevent automatic flushing (will be manually controlled)
    std::cin.tie(nullptr);

    std::string cmd;
    while (std::getline(std::cin, cmd)) {
        if (cmd == "uci") {
            cmd_uci();
        } else if (cmd.starts_with("setoption")) {
            cmd_setoption(cmd);
        } else if (cmd == "isready") {
            cmd_isready();
        } else if (cmd == "ucinewgame") {
            cmd_ucinewgame();
        } else if (cmd.starts_with("position")) {
            cmd_position(cmd);
        } else if (cmd.starts_with("go")) {
            cmd_go();
        } else if (cmd == "debug") {
            cmd_debug();
        } else if (cmd == "register") {
            cmd_register();
        } else if (cmd == "ponderhit") {
            cmd_ponderhit();
        } else if (cmd == "stop") {
            cmd_stop();
        } else if (cmd == "quit") {
            cmd_quit();
        }
    }

}