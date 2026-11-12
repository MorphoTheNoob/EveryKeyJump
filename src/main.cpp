
#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#include <unordered_set>

using namespace geode::prelude;

static std::unordered_set<int> heldKeys;

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

            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                key, isKeyDown, isKeyRepeat, timestamp
            );
        }

        // Check whether the mod is enabled.
        bool enabled = Mod::get()->getSettingValue<bool>("enabled");

        if (!enabled) {
            if (!heldKeys.empty()) {
                playLayer->handleButton(false, 1, true);
                heldKeys.clear();
            }

            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                key, isKeyDown, isKeyRepeat, timestamp
            );
        }

        // Ignore unknown keys.
        if (key == KEY_Unknown) {
            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                key, isKeyDown, isKeyRepeat, timestamp
            );
        }

        int keyID = static_cast<int>(key);

        if (isKeyDown) {
            // Ignore keyboard auto-repeat.
            if (!isKeyRepeat) {
                bool wasEmpty = heldKeys.empty();

                heldKeys.insert(keyID);

                // First key pressed: hold jump.
                if (wasEmpty && !heldKeys.empty()) {
                    playLayer->handleButton(true, 1, true);
                }
            }
        }
        else {
            heldKeys.erase(keyID);

            // Last key released: release jump.
            if (heldKeys.empty()) {
                playLayer->handleButton(false, 1, true);
            }
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(
            key, isKeyDown, isKeyRepeat, timestamp
        );
    }
};
