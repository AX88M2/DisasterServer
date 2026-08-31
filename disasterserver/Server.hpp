#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include "DisasterServer.hpp"
#include "Util/Time.hpp"
#include "Player.hpp"

enum class SurvivalCharacters {
    NONE = -1,

    TAILS,
    KNUX,
    EGGMAN,
    AMY,
    CREAM,
    SALLY
};

enum class ExeCharacters {
    NONE = -1,

    ORIGINAL,
    CHAOS,
    EXETIOR,
    EXELLER
};

enum class States {
    LOBBY,
    MAPVOTE,
    CHARSELECT,
    GAME,
    RESULTS
};

enum BigRingState
{
    BS_NONE,
    BS_DEACTIVATED,
    BS_ACTIVATED,
};

enum Ending
{
    ED_EXEWIN,
    ED_SURVWIN,
    ED_TIMEOVER
};

struct PeerData
{
    uint16_t id;
    std::string ip;
    ENetPeer *peer;

    /* General info */
    Player plr;
    String nickname;
    String udid;
    uint8_t lobby_icon;
    int8_t pet;

    bool verified;
    bool in_game;
    bool op;
    bool ready;
    bool mod_tool;
    bool is_mobile;
    bool can_vote;
    bool voted;
    bool disconnecting;

    //auth_peer_data auth;

    /* Character */
    SurvivalCharacters surv_char;
    ExeCharacters exe_char;

    bool should_timeout;

    /* State info */
    uint8_t exe_chance;
    double timeout;
    double vote_cooldown;

    Server *server;
};

class Server {
    uint16_t id;
    bool running = true;

    States state = States::LOBBY;
    std::mutex mutex;

    // States


    int8_t last_map = -1;
    int16_t map_pickrates[30] = {};

    double delta = 0;
    std::vector<PeerData> peers;
    ENetHost *host;
public:
    Server(uint16_t basePort, uint16_t id);
    ~Server();

    void worker();
};


#endif //DISASTERSERVER_SERVER_HPP
