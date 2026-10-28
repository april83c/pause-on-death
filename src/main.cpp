#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Flow:
//   destroyPlayer  -> mark "holding this death", queue pauseGame() (next frame or after delay)
//   delayedResetLevel (GD's respawn timer) -> swallowed while holding, remembered as pending
//   resume         -> release the hold; if GD already tried to respawn, respawn now
//   restart / any level reset -> drop the hold, let GD do its thing
//   (practice toggle from the pause menu ends in either a reset or a resume, both covered)
class $modify(DeathPausePlayLayer, PlayLayer) {
    struct Fields {
        bool holding = false;      // current death is being held on the pause menu
        bool resetPending = false; // GD's respawn timer fired while we were holding
    };

    bool shouldPauseOnDeath() {
        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("enabled")) return false;
        return m_isPracticeMode
            ? mod->getSettingValue<bool>("practice-mode")
            : mod->getSettingValue<bool>("normal-mode");
    }

    void clearHold() {
        m_fields->holding = false;
        m_fields->resetPending = false;
    }

    void releaseHold() {
        if (!m_fields->holding) return;
        bool pending = m_fields->resetPending;
        clearHold();
        // Only respawn if GD's timer already fired and the player is still dead
        // (if vanilla resume() already reset, player won't be dead -> no double reset).
        if (pending && m_player1 && m_player1->m_isDead) {
            this->delayedResetLevel();
        }
    }

    void pauseAfterDeath() {
        if (!m_fields->holding) return;           // level was reset in the meantime
        if (!m_player1 || !m_player1->m_isDead) { // revived somehow, nothing to hold
            releaseHold();
            return;
        }
        if (!m_isPaused) this->pauseGame(false);
        // If GD refused to pause (end animation etc.), never leave the player stuck dead.
        if (!m_isPaused) releaseHold();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);

        if (object == m_anticheatSpike) return;             // level-start anticheat probe
        if (!player || !player->m_isDead) return;           // noclip / didn't actually die
        if (m_fields->holding) return;                      // 2-player: second death same frame
        if (m_hasCompletedLevel || m_levelEndAnimationStarted) return;
        if (!shouldPauseOnDeath()) return;

        m_fields->holding = true;
        m_fields->resetPending = false;

        auto delay = static_cast<float>(Mod::get()->getSettingValue<double>("delay"));
        this->runAction(CCSequence::create(
            CCDelayTime::create(delay),
            CCCallFunc::create(this, callfunc_selector(DeathPausePlayLayer::pauseAfterDeath)),
            nullptr
        ));
    }

    void delayedResetLevel() {
        if (m_fields->holding) {
            m_fields->resetPending = true;
            return;
        }
        PlayLayer::delayedResetLevel();
    }

    void resume() {
        PlayLayer::resume();
        releaseHold();
    }

    void resumeAndRestart(bool fromStart) {
        clearHold();
        PlayLayer::resumeAndRestart(fromStart);
    }

    void resetLevel() {
        clearHold();
        PlayLayer::resetLevel();
    }
};
