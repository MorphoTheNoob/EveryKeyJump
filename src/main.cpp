
#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <unordered_set>
#include <deque>
#include <string>
#include <sstream>

using namespace geode::prelude;

static std::unordered_set<int> heldKeys;
static std::deque<bool> inputQueue;
static PlayLayer* currentLayer = nullptr;
static bool jumpHeld = false;
static bool playerWasAlive = false;

static bool isBlacklisted(int key) {
    auto blacklist = Mod::get()->getSettingValue<std::string>("blacklist");
    std::stringstream stream(blacklist);
    std::string item;

    while (std::getline(stream, item, ',')) {
        try {
            if (std::stoi(item) == key) {
                return true;
            }
        }
        catch (...) {
        }
    }

    return false;
}

class $modify(EveryKeyJumpKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(
        enumKeyCodes key,
        bool isKeyDown,
        bool isKeyRepeat,
        double timestamp
    ) {
        if (key != KEY_Unknown) {
            int keyID = static_cast<int>(key);

            if (!isBlacklisted(keyID)) {
                if (isKeyDown && !isKeyRepeat) {
                    if (heldKeys.insert(keyID).second) {
                        inputQueue.push_back(true);
                        inputQueue.push_back(false);
                    }
                }
                else if (!isKeyDown) {
                    heldKeys.erase(keyID);
                }
            }
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(
            key, isKeyDown, isKeyRepeat, timestamp
        );
    }
};

class $modify(EveryKeyJumpPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool dontCreateObjects) {
        if (!PlayLayer::init(level, dontCreateObjects)) {
            return false;
        }

        currentLayer = this;
        jumpHeld = false;
        playerWasAlive = false;

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        if (currentLayer != this) {
            return;
        }

        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            inputQueue.clear();

            if (jumpHeld) {
                this->handleButton(false, 1, true);
                jumpHeld = false;
            }

            return;
        }

        auto* player = m_player1;

        if (!player || player->m_isDead) {
            playerWasAlive = false;
            jumpHeld = false;
            return;
        }

        if (!playerWasAlive) {
            playerWasAlive = true;

            if (!heldKeys.empty()) {
                this->handleButton(true, 1, true);
                jumpHeld = true;
            }
        }

        if (!inputQueue.empty()) {
            bool down = inputQueue.front();
            inputQueue.pop_front();

            this->handleButton(down, 1, true);
            jumpHeld = down;
            return;
        }

        bool shouldHold = !heldKeys.empty();

        if (shouldHold != jumpHeld) {
            this->handleButton(shouldHold, 1, true);
            jumpHeld = shouldHold;
        }
    }

    void onQuit() {
        if (currentLayer == this) {
            currentLayer = nullptr;
            jumpHeld = false;
            playerWasAlive = false;
            inputQueue.clear();
        }

        PlayLayer::onQuit();
    }
};
