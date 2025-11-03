
/*
mod-auto-announcer.cpp */

#include "ScriptMgr.h"
#include "World.h"
#include "Player.h"
#include "Chat.h"
#include "Config.h"
#include "WorldSessionMgr.h"
#include "SharedDefines.h"
#include "DatabaseEnv.h"
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <vector>
#include <set>
#include <ctime>

// --- 전역 변수 ---

// 설정 파일 기반 공지
bool g_confAnnouncerEnabled = false;
uint32 g_confAnnouncerInterval = 120;
std::vector<std::string> g_confAnnouncerMessages;
uint32 g_confAnnouncerTimer = 0;
uint32 g_confAnnouncerCurrentIndex = 0;

// DB 기반 공지
bool g_dbAnnouncerEnabled = false;
uint32 g_dbAnnouncerUpdateInterval = 300;
uint32 g_dbAnnouncerBroadcastInterval = 60;
std::string g_dbAnnouncerColor = "|cffFFFF00|r";
std::vector<std::string> g_dbAnnouncerMessages;
uint32 g_dbAnnouncerUpdateTimer = 0;
uint32 g_dbAnnouncerBroadcastTimer = 0;
uint32 g_dbAnnouncerCurrentIndex = 0;

// --- 함수 선언 ---
void LoadDBAnnouncements();

// --- 헬퍼 함수 ---
static std::vector<std::string> SplitString(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (!token.empty())
        {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// --- 설정 로드 ---
void LoadModuleSpecificConfig_AutoAnnouncer()
{
    std::string configFilePath = "./configs/modules/mod-auto-announcer.conf";
    std::ifstream configFile(configFilePath);

    if (!configFile.is_open())
    {
        LOG_INFO("module", "[자동 알림] 설정 파일을 찾을 수 없어 기본값으로 작동합니다: {}", configFilePath);
        return;
    }

    LOG_INFO("module", "[자동 알림] 설정 파일 로드: {}", configFilePath);

    // 기본값 초기화
    g_confAnnouncerEnabled = false;
    g_confAnnouncerInterval = 120;
    g_dbAnnouncerEnabled = false;
    g_dbAnnouncerUpdateInterval = 300;
    g_dbAnnouncerBroadcastInterval = 60;
    g_dbAnnouncerColor = "|cffFFFF00|r";
    g_confAnnouncerMessages.clear();

    std::string line;
    while (std::getline(configFile, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value))
        {
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (key == "AutoAnnouncer.Enable")
                g_confAnnouncerEnabled = (value == "1");
            else if (key == "AutoAnnouncer.IntervalSeconds")
                g_confAnnouncerInterval = std::stoul(value);
            else if (key == "AutoAnnouncer.Messages")
                g_confAnnouncerMessages = SplitString(value, ';');
            else if (key == "AutoAnnouncer.DB.Enable")
                g_dbAnnouncerEnabled = (value == "1");
            else if (key == "AutoAnnouncer.DB.UpdateInterval")
                g_dbAnnouncerUpdateInterval = std::stoul(value);
            else if (key == "AutoAnnouncer.DB.IntervalSeconds")
                g_dbAnnouncerBroadcastInterval = std::stoul(value);
            else if (key == "AutoAnnouncer.DB.Color")
                g_dbAnnouncerColor = value;
        }
    }
    configFile.close();
}

// --- DB 공지 로드 ---
void LoadDBAnnouncements()
{
    if (!g_dbAnnouncerEnabled)
    {
        if (!g_dbAnnouncerMessages.empty()) { g_dbAnnouncerMessages.clear(); }
        return;
    }

    LOG_INFO("module", "[자동 알림 DB] 데이터베이스에서 공지사항을 로드합니다...");

    QueryResult result = LoginDatabase.Query("SELECT scroll_message FROM acore_auth.visitor_stats WHERE visit_date = CURDATE() AND scroll_message IS NOT NULL AND scroll_message != ''");

    g_dbAnnouncerMessages.clear();

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            g_dbAnnouncerMessages.push_back(fields[0].Get<std::string>());
        } while (result->NextRow());

        LOG_INFO("module", "[자동 알림 DB] 데이터베이스에서 {}개의 공지사항을 로드했습니다.", g_dbAnnouncerMessages.size());
    }
    else
    {
        LOG_INFO("module", "[자동 알림 DB] 오늘 날짜의 데이터베이스 공지사항이 없습니다.");
    }

    if (g_dbAnnouncerCurrentIndex >= g_dbAnnouncerMessages.size())
    {
        g_dbAnnouncerCurrentIndex = 0;
    }
}

// --- 월드 스크립트 ---
class mod_auto_announcer_world : public WorldScript
{
public:
    mod_auto_announcer_world() : WorldScript("mod_auto_announcer_world") { }

    void OnBeforeConfigLoad(bool reload) override
    {
        LoadModuleSpecificConfig_AutoAnnouncer();
        if (reload)
        {
            LoadDBAnnouncements();
        }
    }

    void OnStartup() override
    {
        LoadDBAnnouncements();
    }

    void OnUpdate(uint32 diff) override
    {
        // --- 설정 파일 기반 공지 처리 ---
        if (g_confAnnouncerEnabled && !g_confAnnouncerMessages.empty())
        {
            g_confAnnouncerTimer += diff;
            if (g_confAnnouncerTimer >= (g_confAnnouncerInterval * 1000))
            {
                g_confAnnouncerTimer = 0;
                std::string message = g_confAnnouncerMessages[g_confAnnouncerCurrentIndex];
                WorldSessionMgr::Instance()->SendServerMessage(SERVER_MSG_STRING, message.c_str());
                LOG_INFO("module", "[자동 알림 Conf] 메시지 전송: {}", message);
                g_confAnnouncerCurrentIndex = (g_confAnnouncerCurrentIndex + 1) % g_confAnnouncerMessages.size();
            }
        }

        // --- DB 기반 공지 처리 ---
        if (g_dbAnnouncerEnabled)
        {
            // DB 내용 업데이트 타이머
            g_dbAnnouncerUpdateTimer += diff;
            if (g_dbAnnouncerUpdateTimer >= (g_dbAnnouncerUpdateInterval * 1000))
            {
                g_dbAnnouncerUpdateTimer = 0;
                LoadDBAnnouncements();
            }

            // DB 메시지 방송 타이머
            if (!g_dbAnnouncerMessages.empty())
            {
                g_dbAnnouncerBroadcastTimer += diff;
                if (g_dbAnnouncerBroadcastTimer >= (g_dbAnnouncerBroadcastInterval * 1000))
                {
                    g_dbAnnouncerBroadcastTimer = 0;
                    std::string message = g_dbAnnouncerColor + g_dbAnnouncerMessages[g_dbAnnouncerCurrentIndex];
                    WorldSessionMgr::Instance()->SendServerMessage(SERVER_MSG_STRING, message.c_str());
                    LOG_INFO("module", "[자동 알림 DB] 메시지 전송: {}", message);
                    g_dbAnnouncerCurrentIndex = (g_dbAnnouncerCurrentIndex + 1) % g_dbAnnouncerMessages.size();
                }
            }
        }
    }
};

// --- 스크립트 추가 ---
void Addmod_auto_announcerScripts()
{
    new mod_auto_announcer_world();
}

