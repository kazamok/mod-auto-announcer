#include "ScriptMgr.h"
#include "World.h"
#include "Player.h"
#include "Chat.h"
#include "Config.h"
#include "WorldSessionMgr.h"
#include "SharedDefines.h"
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <vector>
#include <set>
#include <ctime>

// 모듈 설정 값을 저장할 전역 변수
bool g_autoAnnouncerEnabled = false;
uint32 g_autoAnnouncerIntervalSeconds = 0;
std::vector<std::string> g_autoAnnouncerMessages;

// 타이머 관련 전역 변수
uint32 g_autoAnnouncerTimer = 0;
uint32 g_autoAnnouncerCurrentMessageIndex = 0;

// 문자열을 구분자로 분리하여 벡터로 반환하는 헬퍼 함수
static std::vector<std::string> SplitString(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        // 각 토큰의 앞뒤 공백 제거
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        tokens.push_back(token);
    }
    return tokens;
}

// 모듈 전용 설정 파일을 로드하고 파싱하는 함수
void LoadModuleSpecificConfig_AutoAnnouncer()
{
    std::string configFilePath = "./configs/modules/mod-auto-announcer.conf.dist";

    std::ifstream configFile;

    if (std::filesystem::exists(configFilePath))
    {
        configFile.open(configFilePath);
        LOG_INFO("module", "[자동 알림] 설정 파일 로드: {}", configFilePath);
    }
    else
    {
        LOG_ERROR("module", "[자동 알림] 설정 파일을 찾을 수 없습니다. 모듈이 비활성화됩니다.");
        g_autoAnnouncerEnabled = false;
        return;
    }

    if (!configFile.is_open())
    {
        LOG_ERROR("module", "[자동 알림] 설정 파일을 열 수 없습니다. 모듈이 비활성화됩니다.");
        g_autoAnnouncerEnabled = false;
        return;
    }

    // 기본값 설정
    g_autoAnnouncerEnabled = true;
    g_autoAnnouncerIntervalSeconds = 300; // 5분
    g_autoAnnouncerMessages.clear();

    std::string line;
    while (std::getline(configFile, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '='))
        {
            std::string value;
            if (std::getline(iss, value))
            {
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (key == "AutoAnnouncer.Enable")
                {
                    g_autoAnnouncerEnabled = (value == "1");
                }
                else if (key == "AutoAnnouncer.IntervalSeconds")
                {
                    try
                    {
                        g_autoAnnouncerIntervalSeconds = std::stoul(value);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR("module", "[자동 알림] 잘못된 간격 설정: {} ({})", value, e.what());
                        g_autoAnnouncerIntervalSeconds = 300; // 오류 시 기본값
                    }
                }
                else if (key == "AutoAnnouncer.Messages")
                {
                    g_autoAnnouncerMessages = SplitString(value, ';');
                }
            }
        }
    }
    configFile.close();

    // 메시지가 없으면 모듈 비활성화
    if (g_autoAnnouncerMessages.empty())
    {
        LOG_ERROR("module", "[자동 알림] 설정된 메시지가 없습니다. 모듈이 비활성화됩니다.");
        g_autoAnnouncerEnabled = false;
    }
}

// 월드 서버 이벤트를 처리하는 클래스
class mod_auto_announcer_world : public WorldScript
{
public:
    mod_auto_announcer_world() : WorldScript("mod_auto_announcer_world") { }

    void OnBeforeConfigLoad(bool reload) override
    {
        // 모듈 전용 설정 파일을 로드하고 파싱합니다。
        LoadModuleSpecificConfig_AutoAnnouncer();
    }

    void OnUpdate(uint32 diff) override
    {
        if (!g_autoAnnouncerEnabled || g_autoAnnouncerMessages.empty())
            return;

        // 타이머 업데이트
        g_autoAnnouncerTimer += diff; // diff는 밀리초 단위

        // 설정된 간격(초)에 도달하면 메시지 전송
        if (g_autoAnnouncerTimer >= (g_autoAnnouncerIntervalSeconds * 1000))
        {
            // 메시지 전송
            if (g_autoAnnouncerCurrentMessageIndex < g_autoAnnouncerMessages.size())
            {
                std::string message = g_autoAnnouncerMessages[g_autoAnnouncerCurrentMessageIndex];
                WorldSessionMgr::Instance()->SendServerMessage(SERVER_MSG_STRING, message);
                LOG_INFO("module", "[자동 알림] 메시지 전송: {}", message);
            }

            // 다음 메시지 인덱스 업데이트 (순환)
            g_autoAnnouncerCurrentMessageIndex = (g_autoAnnouncerCurrentMessageIndex + 1) % g_autoAnnouncerMessages.size();

            // 타이머 초기화
            g_autoAnnouncerTimer = 0;
        }
    }
};

// 모든 스크립트를 추가하는 함수 (모듈 로드 시 호출됨)
void Addmod_auto_announcerScripts()
{
    new mod_auto_announcer_world();
}
