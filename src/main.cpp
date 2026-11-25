
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
static bool jumpHeld = false;

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
        auto* playLayer = GameManager::get()->m_playLayer;

        if (!playLayer) {
            heldKeys.clear();
            inputQueue.clear();
            jumpHeld = false;

            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                key, isKeyDown, isKeyRepeat, timestamp
            );
        }

        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            heldKeys.clear();
            inputQueue.clear();

            if (jumpHeld) {
                playLayer->handleButton(false, 1, true);
                jumpHeld = false;
            }

            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                key, isKeyDown, isKeyRepeat, timestamp
            );
        }

        int keyID = static_cast<int>(key);

        if (key == KEY_Unknown || isBlacklisted(keyID)) {
            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                key, isKeyDown, isKeyRepeat, timestamp
            );
        }

        if (isKeyDown && !isKeyRepeat) {
            if (heldKeys.insert(keyID).second) {
                inputQueue.push_back(true);
                inputQueue.push_back(false);
            }
        }
        else if (!isKeyDown) {
            heldKeys.erase(keyID);
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(
            key, isKeyDown, isKeyRepeat, timestamp
        );
    }
};

class $modify(EveryKeyJumpPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);

        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            inputQueue.clear();
            heldKeys.clear();

            if (jumpHeld) {
                this->handleButton(false, 1, true);
                jumpHeld = false;
            }

            return;
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
};
