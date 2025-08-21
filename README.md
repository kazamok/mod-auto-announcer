# Auto Announcer Module (mod-auto-announcer)

## 1. 모듈 소개

`mod-auto-announcer`는 아제로스코어(AzerothCore) 월드 서버에서 설정된 간격마다 미리 정의된 알림 메시지를 자동으로 서버 전역에 브로드캐스트하는 모듈입니다. 이 모듈은 서버 관리자가 플레이어들에게 중요한 공지사항, 규칙, 이벤트 정보 등을 주기적으로 상기시키거나, 서버의 분위기를 조성하는 데 매우 유용하게 활용될 수 있습니다.

이 모듈은 아제로스코어의 핵심 파일을 수정하지 않는 **완전한 플러그인 형태**로 제작되어, 기존 서버 환경에 영향을 주지 않으면서 독립적으로 작동하도록 설계되었습니다.

## 2. 주요 기능

*   **주기적인 알림**: 설정된 시간 간격마다 자동으로 알림 메시지를 서버 전역에 출력합니다.
*   **다중 메시지 순환**: 여러 개의 알림 메시지를 등록하여 순차적으로 브로드캐스트할 수 있습니다.
*   **서버 전역 브로드캐스트**: 모든 온라인 플레이어의 챗창에 메시지를 표시하여 높은 가시성을 확보합니다.
*   **독립적인 설정**: `worldserver.conf` 파일을 직접 수정하지 않고, 모듈 자체의 설정 파일(`mod-auto-announcer.conf.dist`)을 통해 모듈의 활성화 여부, 알림 간격, 메시지 목록 등을 유연하게 제어할 수 있습니다.
*   **콘솔 로깅**: 메시지 전송 및 모듈의 작동 상태를 서버 콘솔에 기록하여 운영자가 쉽게 모니터링할 수 있도록 돕습니다.

## 3. 설치 방법

1.  **모듈 파일 배치**: 이 모듈의 모든 파일을 아제로스코어 소스 코드의 `modules/mod-auto-announcer/` 경로에 배치합니다.
    (예시: `C:/azerothcore/modules/mod-auto-announcer/`)

2.  **아제로스코어 빌드**: 아제로스코어 프로젝트를 다시 빌드합니다. 이 과정에서 `mod-auto-announcer` 모듈이 함께 컴파일되고, 필요한 설정 파일이 설치 경로로 복사됩니다.
    (빌드 과정은 사용자의 환경에 따라 다를 수 있습니다. 일반적으로 `cmake --build .` 명령을 사용합니다.)

3.  **설정 파일 준비**: 빌드 완료 후, 아제로스코어 설치 디렉토리의 `configs/modules/` 폴더에 `mod-auto-announcer.conf.dist` 파일이 생성됩니다. 이 파일은 모듈의 설정 파일로 사용됩니다.

    *   **설정 파일 경로 예시**: `C:/BUILD/bin/RelWithDebInfo/configs/modules/mod-auto-announcer.conf.dist`

## 4. 설정 (Configuration)

`mod-auto-announcer.conf.dist` 파일에서 다음 옵션들을 설정할 수 있습니다.

```ini
#----------------------------------------------------------
# Auto Announcer Module Settings
#----------------------------------------------------------
#
# Enable Auto Announcer Module (0 = disabled, 1 = enabled)
AutoAnnouncer.Enable = 1

# Interval between announcements in seconds
AutoAnnouncer.IntervalSeconds = 300

# List of messages to announce (semicolon-separated)
# Use |cffXXXXXX|r for colors. Example: "|cff00ff00|r환영합니다!|r"
AutoAnnouncer.Messages = "|cff00ff00|r서버에 오신 것을 환영합니다!|r";"|cffff0000|r불법 프로그램 사용은 제재 대상입니다.|r";"|cffffff00|r서로 존중하는 언어를 사용합시다.|r"
```

*   `AutoAnnouncer.Enable`: 모듈의 기능을 전체적으로 켜거나 끕니다. 기본값은 `1` (활성화)입니다.
*   `AutoAnnouncer.IntervalSeconds`: 알림 메시지를 출력할 시간 간격을 초 단위로 설정합니다. (예: `300`은 5분마다 메시지 출력)
*   `AutoAnnouncer.Messages`: 서버 전역에 브로드캐스트할 알림 메시지 목록을 세미콜론(`;`)으로 구분하여 입력합니다. 각 메시지 내에서 `|cffXXXXXX|r` 형식의 색상 코드를 사용하여 텍스트 색상을 변경할 수 있습니다.

## 5. 사용 방법

1.  **월드 서버 실행**: 설정이 완료된 후 월드 서버를 실행합니다.

2.  **알림 메시지 확인**: `AutoAnnouncer.IntervalSeconds`에 설정된 시간 간격마다 게임 내 챗창에 `AutoAnnouncer.Messages`에 정의된 메시지들이 순환하며 표시됩니다.

3.  **콘솔 로그 확인**: 월드 서버 콘솔에서 모듈의 작동 상태 및 메시지 전송 정보를 확인할 수 있습니다.

## 6. 향후 개선 사항

*   특정 채널 또는 특정 그룹에게만 메시지를 보내는 기능.

## 👥 크레딧
- Kazamok
- Gemini
- 모든 기여자들

## 📄 라이선스

이 프로젝트는 GPL-3.0 라이선스 하에 배포됩니다.
*   메시지 목록을 동적으로 변경할 수 있는 인게임 명령어.
*   메시지 출력 시 사운드 또는 시각적 효과 추가.


---
