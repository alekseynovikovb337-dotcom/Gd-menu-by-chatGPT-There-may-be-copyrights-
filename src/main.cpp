#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/SliderTouchLogic.hpp>
#include <Geode/modify/LevelPage.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/RewardUnlockLayer.hpp>
#include <Geode/modify/SecretRewardsLayer.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <vector>
#include <cmath>


using namespace geode::prelude;

namespace {
    bool g_tpsBypassEnabled = false;
    double g_customTPS = 240.0;
    bool g_mainLevelBypassEnabled = false;
    bool g_sliderLimitBypassEnabled = false;
    bool g_towerLevelBypassEnabled = false;
    bool g_pauseBufferBypassEnabled = false;
    bool g_freeShoppingEnabled = false;
    bool g_practiceModeEnabled = false;
    bool g_ghostEffectEnabled = true;
    bool g_tpsInternalTick = false;
    double g_tpsAccumulator = 0.0;
    unsigned g_buttonColorIndex = 0;
    unsigned g_buttonOpacityIndex = 0;
    unsigned g_buttonScaleIndex = 1;
    bool g_hideInLevelButton = false;
    bool g_autoPracticeEnabled = false;
    bool g_hidePlayerEnabled = false;
    bool g_autoRetryEnabled = false;
    bool g_autoClickerEnabled = false;
    float g_autoClickerTimer = 0.f;
    bool g_autoClickerPressed = false;
    bool g_noTrailEnabled = false;
    bool g_noPlayerGlowEnabled = false;
    bool g_forcePlatformerEnabled = false;
    PlayerObject* g_platformerTrackedP1 = nullptr;
    PlayerObject* g_platformerTrackedP2 = nullptr;
    bool g_platformerOriginalP1 = false;
    bool g_platformerOriginalP2 = false;
    bool g_waveTrailEnabled = true;
    bool g_cheatIndicatorEnabled = true;
    bool g_tintOnDeathEnabled = false;
    bool g_noParticlesEnabled = false;
    bool g_noRotationEnabled = false;
    bool g_noCameraShakeEnabled = false;
    bool g_noOrbPulseEnabled = false;
    bool g_solidWaveTrailEnabled = false;
    bool g_noMirrorPortalEnabled = false;
    bool g_instantMirrorPortalEnabled = false;
    bool g_fastChestsEnabled = false;
    bool g_chestKeyBypassEnabled = false;
    bool g_autoCollectCoinsEnabled = false;
    bool g_noDashFireEnabled = false;
    bool g_noRobotFireEnabled = false;
    bool g_noShipFireEnabled = false;
    bool g_noRespawnBlinkEnabled = false;
    bool g_noWavePulseEnabled = false;
    bool g_noObjectGlowEnabled = false;
    std::vector<EffectGameObject*> g_levelCoins;
    CCLabelBMFont* g_cheatIndicatorLabel = nullptr;
    const GLubyte g_buttonOpacities[] = {255, 210, 165, 120};
    const float g_buttonScales[] = {0.55f, 0.72f, 0.88f, 1.0f};
    const ccColor3B g_buttonColors[] = {{255,255,255},{100,220,255},{140,255,140},{255,190,90},{230,130,255},{255,110,130}};
    PlayerObject* g_gravityTrackedP1 = nullptr;
    PlayerObject* g_gravityTrackedP2 = nullptr;
    double g_gravityBaseP1 = 0.0;
    double g_gravityBaseP2 = 0.0;
    double g_gravityLastP1 = 0.0;
    double g_gravityLastP2 = 0.0;

}

static void refreshButtonAppearance();
class UltimateMenuPopup : public geode::Popup {
protected:
    TextInput* m_speedInput = nullptr;
    CCMenuItemLabel* m_speedToggle = nullptr;
    CCMenuItemLabel* m_gravityToggle = nullptr;
    CCMenuItemLabel* m_jumpToggle = nullptr;
    CCMenuItemLabel* m_deathToggle = nullptr;
    CCMenuItemLabel* m_noclipToggle = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;
    CCMenu* m_itemsMenu = nullptr;
    CCLabelBMFont* m_tabHint = nullptr;
    CCMenuItemLabel* m_colorCycle = nullptr;
    CCMenuItemLabel* m_opacityCycle = nullptr;
    CCMenuItemLabel* m_sizeCycle = nullptr;
    CCMenuItemLabel* m_hideButton = nullptr;
    CCMenuItemLabel* m_tpsToggle = nullptr;
    CCMenuItemLabel* m_tpsApply = nullptr;
    TextInput* m_tpsInput = nullptr;
    CCNode* m_bypassControls = nullptr;
    CCMenuItemLabel* m_mainLevelBypassToggle = nullptr;
    CCMenuItemLabel* m_sliderLimitBypassToggle = nullptr;
    CCMenuItemLabel* m_towerLevelBypassToggle = nullptr;
    CCMenuItemLabel* m_pauseBufferBypassToggle = nullptr;
    CCMenuItemLabel* m_freeShoppingToggle = nullptr;
    CCMenuItemLabel* m_instantCompleteButton = nullptr;
    CCMenuItemLabel* m_practiceModeButton = nullptr;
    CCMenuItemLabel* m_ghostEffectButton = nullptr;
    CCMenuItemLabel* m_autoPracticeButton = nullptr;
    CCMenuItemLabel* m_hidePlayerButton = nullptr;
    CCMenuItemLabel* m_autoRetryButton = nullptr;
    CCMenuItemLabel* m_autoClickerButton = nullptr;
    CCMenuItemLabel* m_noTrailButton = nullptr;
    CCMenuItemLabel* m_noGlowButton = nullptr;
    CCMenuItemLabel* m_forcePlatformerButton = nullptr;
    CCMenuItemLabel* m_waveTrailButton = nullptr;
    CCMenuItemLabel* m_noParticlesButton = nullptr;
    CCMenuItemLabel* m_noRotationButton = nullptr;
    CCMenuItemLabel* m_noShakeButton = nullptr;
    CCMenuItemLabel* m_noOrbPulseButton = nullptr;
    CCMenuItemLabel* m_solidWaveButton = nullptr;
    CCMenuItemLabel* m_noMirrorButton = nullptr;
    CCMenuItemLabel* m_instantMirrorButton = nullptr;
    CCMenuItemLabel* m_fastChestsButton = nullptr;
    CCMenuItemLabel* m_chestBypassButton = nullptr;
    CCMenuItemLabel* m_autoCoinsButton = nullptr;
    CCMenuItemLabel* m_noDashFireButton = nullptr;
    CCMenuItemLabel* m_noRobotFireButton = nullptr;
    CCMenuItemLabel* m_noShipFireButton = nullptr;
    CCMenuItemLabel* m_noRespawnBlinkButton = nullptr;
    CCMenuItemLabel* m_noWavePulseButton = nullptr;
    CCMenuItemLabel* m_noObjectGlowButton = nullptr;
    CCMenuItemLabel* m_cheatIndicatorButton = nullptr;
    CCMenuItemLabel* m_tintOnDeathButton = nullptr;
    CCNode* m_playerOnlyControls = nullptr;

    bool init() {
        if (!Popup::init(390.f, 310.f)) return false;
        this->setTitle("GD Ultimate Menu");

        auto layerSize = m_mainLayer->getContentSize();
        auto center = ccp(layerSize.width / 2.f, layerSize.height / 2.f);
        // Reserve a fixed footer for the numeric inputs; only the option rows scroll.
        m_scroll = geode::ScrollLayer::create(CCSize{350.f, 170.f});
        m_scroll->setPosition({20.f, 94.f});
        m_scroll->setID("ultimate-menu-scroll"_spr);
        m_mainLayer->addChild(m_scroll);
        auto content = m_scroll->m_contentLayer;
        content->setContentSize({350.f, 1000.f});
        auto menu = CCMenu::create();
        m_itemsMenu = menu;
        menu->setPosition({175.f, 650.f});
        content->addChild(menu);

        auto makeLabel = [](std::string const& text) {
            return CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        };

        m_speedToggle = CCMenuItemLabel::create(makeLabel("Speedhack: OFF"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_speedToggle->setTag(10);
        m_speedToggle->setScale(0.55f);
        m_speedToggle->setPosition({0.f, 155.f});
        menu->addChild(m_speedToggle);

        m_gravityToggle = CCMenuItemLabel::create(makeLabel("Low gravity: OFF"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_gravityToggle->setTag(11);
        m_gravityToggle->setScale(0.55f);
        m_gravityToggle->setPosition({0.f, 115.f});
        menu->addChild(m_gravityToggle);

        m_jumpToggle = CCMenuItemLabel::create(makeLabel("Jump effect: ON"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_jumpToggle->setTag(12);
        m_jumpToggle->setScale(0.55f);
        m_jumpToggle->setPosition({0.f, 75.f});
        menu->addChild(m_jumpToggle);

        m_deathToggle = CCMenuItemLabel::create(makeLabel("Death effect: ON"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_deathToggle->setTag(13);
        m_deathToggle->setScale(0.55f);
        m_deathToggle->setPosition({0.f, 35.f});
        menu->addChild(m_deathToggle);

        m_noclipToggle = CCMenuItemLabel::create(makeLabel("NoClip: OFF"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_noclipToggle->setTag(14);
        m_noclipToggle->setScale(0.55f);
        m_noclipToggle->setPosition({0.f, -5.f});
        menu->addChild(m_noclipToggle);

        m_playerOnlyControls = CCNode::create();
        m_mainLayer->addChild(m_playerOnlyControls, 7);
        m_speedInput = TextInput::create(130.f, "Speed multiplier");
        m_speedInput->setCommonFilter(CommonFilter::Float);
        m_speedInput->setString(fmt::format("{:.2f}", Mod::get()->getSettingValue<double>("speed-multiplier")));
        m_speedInput->setPosition({center.x, 66.f});
        m_speedInput->setScale(0.8f);
        m_playerOnlyControls->addChild(m_speedInput);

        auto apply = CCMenuItemLabel::create(makeLabel("APPLY SPEED"), this,
            menu_selector(UltimateMenuPopup::onApplySpeed));
        apply->setTag(15);
        apply->setScale(0.55f);
        apply->setPosition({0.f, -45.f});
        menu->addChild(apply);

        m_practiceModeButton = CCMenuItemLabel::create(makeLabel("PRACTICE MODE: OFF"), this,
            menu_selector(UltimateMenuPopup::onTogglePracticeMode));
        m_practiceModeButton->setTag(16);
        m_practiceModeButton->setScale(0.55f);
        menu->addChild(m_practiceModeButton);

        m_ghostEffectButton = CCMenuItemLabel::create(makeLabel("GHOST EFFECT: ON"), this,
            menu_selector(UltimateMenuPopup::onToggleGhostEffect));
        m_ghostEffectButton->setTag(17);
        m_ghostEffectButton->setScale(0.55f);
        menu->addChild(m_ghostEffectButton);

        m_colorCycle = CCMenuItemLabel::create(makeLabel("BUTTON COLOR"), this, menu_selector(UltimateMenuPopup::onCycleColor));
        m_colorCycle->setTag(50); m_colorCycle->setScale(0.55f); menu->addChild(m_colorCycle);
        m_opacityCycle = CCMenuItemLabel::create(makeLabel("BUTTON OPACITY"), this, menu_selector(UltimateMenuPopup::onCycleOpacity));
        m_opacityCycle->setTag(51); m_opacityCycle->setScale(0.55f); menu->addChild(m_opacityCycle);
        m_sizeCycle = CCMenuItemLabel::create(makeLabel("BUTTON SIZE"), this, menu_selector(UltimateMenuPopup::onCycleSize));
        m_sizeCycle->setTag(52); m_sizeCycle->setScale(0.55f); menu->addChild(m_sizeCycle);
        m_hideButton = CCMenuItemLabel::create(makeLabel("HIDE IN-LEVEL BUTTON: OFF"), this, menu_selector(UltimateMenuPopup::onToggleHideButton));
        m_hideButton->setTag(53); m_hideButton->setScale(0.55f); menu->addChild(m_hideButton);
        m_cheatIndicatorButton = CCMenuItemLabel::create(makeLabel("CHEAT INDICATOR: ON"), this, menu_selector(UltimateMenuPopup::onToggleCheatIndicator));
        m_cheatIndicatorButton->setTag(54); m_cheatIndicatorButton->setScale(0.55f); menu->addChild(m_cheatIndicatorButton);
        m_tintOnDeathButton = CCMenuItemLabel::create(makeLabel("TINT ON DEATH: OFF"), this, menu_selector(UltimateMenuPopup::onToggleTintOnDeath));
        m_tintOnDeathButton->setTag(55); m_tintOnDeathButton->setScale(0.55f); menu->addChild(m_tintOnDeathButton);
        m_autoPracticeButton = CCMenuItemLabel::create(makeLabel("AUTO PRACTICE: OFF"), this, menu_selector(UltimateMenuPopup::onToggleAutoPractice));
        m_autoPracticeButton->setTag(18); m_autoPracticeButton->setScale(0.55f); menu->addChild(m_autoPracticeButton);
        m_hidePlayerButton = CCMenuItemLabel::create(makeLabel("HIDE PLAYER: OFF"), this, menu_selector(UltimateMenuPopup::onToggleHidePlayer));
        m_hidePlayerButton->setTag(19); m_hidePlayerButton->setScale(0.55f); menu->addChild(m_hidePlayerButton);
        m_autoRetryButton = CCMenuItemLabel::create(makeLabel("AUTO RETRY: OFF"), this, menu_selector(UltimateMenuPopup::onToggleAutoRetry));
        m_autoRetryButton->setTag(20); m_autoRetryButton->setScale(0.55f); menu->addChild(m_autoRetryButton);
        m_autoClickerButton = CCMenuItemLabel::create(makeLabel("AUTO CLICKER: OFF"), this, menu_selector(UltimateMenuPopup::onToggleAutoClicker));
        m_autoClickerButton->setTag(21); m_autoClickerButton->setScale(0.55f); menu->addChild(m_autoClickerButton);
        m_noTrailButton = CCMenuItemLabel::create(makeLabel("NO TRAIL: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoTrail));
        m_noTrailButton->setTag(22); m_noTrailButton->setScale(0.55f); menu->addChild(m_noTrailButton);
        m_noGlowButton = CCMenuItemLabel::create(makeLabel("NO PLAYER GLOW: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoGlow));
        m_noGlowButton->setTag(23); m_noGlowButton->setScale(0.55f); menu->addChild(m_noGlowButton);
        m_forcePlatformerButton = CCMenuItemLabel::create(makeLabel("FORCE PLATFORMER: OFF"), this, menu_selector(UltimateMenuPopup::onToggleForcePlatformer));
        m_forcePlatformerButton->setTag(24); m_forcePlatformerButton->setScale(0.55f); menu->addChild(m_forcePlatformerButton);
        m_waveTrailButton = CCMenuItemLabel::create(makeLabel("WAVE TRAIL: ON"), this, menu_selector(UltimateMenuPopup::onToggleWaveTrail));
        m_waveTrailButton->setTag(25); m_waveTrailButton->setScale(0.55f); menu->addChild(m_waveTrailButton);
        m_noParticlesButton = CCMenuItemLabel::create(makeLabel("NO PARTICLES: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoParticles));
        m_noParticlesButton->setTag(26); m_noParticlesButton->setScale(0.55f); menu->addChild(m_noParticlesButton);
        m_noRotationButton = CCMenuItemLabel::create(makeLabel("NO ROTATION: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoRotation));
        m_noRotationButton->setTag(27); m_noRotationButton->setScale(0.55f); menu->addChild(m_noRotationButton);
        m_noShakeButton = CCMenuItemLabel::create(makeLabel("NO CAMERA SHAKE: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoShake));
        m_noShakeButton->setTag(28); m_noShakeButton->setScale(0.55f); menu->addChild(m_noShakeButton);
        m_noOrbPulseButton = CCMenuItemLabel::create(makeLabel("NO ORB PULSE: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoOrbPulse));
        m_noOrbPulseButton->setTag(29); m_noOrbPulseButton->setScale(0.55f); menu->addChild(m_noOrbPulseButton);
        m_solidWaveButton = CCMenuItemLabel::create(makeLabel("SOLID WAVE TRAIL: OFF"), this, menu_selector(UltimateMenuPopup::onToggleSolidWave));
        m_solidWaveButton->setTag(30); m_solidWaveButton->setScale(0.55f); menu->addChild(m_solidWaveButton);
        m_noMirrorButton = CCMenuItemLabel::create(makeLabel("NO MIRROR PORTAL: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoMirror));
        m_noMirrorButton->setTag(31); m_noMirrorButton->setScale(0.55f); menu->addChild(m_noMirrorButton);
        m_instantMirrorButton = CCMenuItemLabel::create(makeLabel("INSTANT MIRROR: OFF"), this, menu_selector(UltimateMenuPopup::onToggleInstantMirror));
        m_instantMirrorButton->setTag(32); m_instantMirrorButton->setScale(0.55f); menu->addChild(m_instantMirrorButton);
        m_fastChestsButton = CCMenuItemLabel::create(makeLabel("FAST CHESTS: OFF"), this, menu_selector(UltimateMenuPopup::onToggleFastChests));
        m_fastChestsButton->setTag(33); m_fastChestsButton->setScale(0.55f); menu->addChild(m_fastChestsButton);
        m_chestBypassButton = CCMenuItemLabel::create(makeLabel("CHEST KEY BYPASS: OFF"), this, menu_selector(UltimateMenuPopup::onToggleChestBypass));
        m_chestBypassButton->setTag(34); m_chestBypassButton->setScale(0.55f); menu->addChild(m_chestBypassButton);
        m_autoCoinsButton = CCMenuItemLabel::create(makeLabel("AUTO COLLECT COINS: OFF"), this, menu_selector(UltimateMenuPopup::onToggleAutoCoins));
        m_autoCoinsButton->setTag(35); m_autoCoinsButton->setScale(0.55f); menu->addChild(m_autoCoinsButton);
        m_noDashFireButton = CCMenuItemLabel::create(makeLabel("NO DASH FIRE: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoDashFire));
        m_noDashFireButton->setTag(36); m_noDashFireButton->setScale(0.55f); menu->addChild(m_noDashFireButton);
        m_noRobotFireButton = CCMenuItemLabel::create(makeLabel("NO ROBOT FIRE: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoRobotFire));
        m_noRobotFireButton->setTag(37); m_noRobotFireButton->setScale(0.55f); menu->addChild(m_noRobotFireButton);
        m_noShipFireButton = CCMenuItemLabel::create(makeLabel("NO SHIP FIRE: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoShipFire));
        m_noShipFireButton->setTag(38); m_noShipFireButton->setScale(0.55f); menu->addChild(m_noShipFireButton);
        m_noRespawnBlinkButton = CCMenuItemLabel::create(makeLabel("NO RESPAWN BLINK: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoRespawnBlink));
        m_noRespawnBlinkButton->setTag(39); m_noRespawnBlinkButton->setScale(0.55f); menu->addChild(m_noRespawnBlinkButton);
        m_noWavePulseButton = CCMenuItemLabel::create(makeLabel("NO WAVE PULSE: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoWavePulse));
        m_noWavePulseButton->setTag(40); m_noWavePulseButton->setScale(0.55f); menu->addChild(m_noWavePulseButton);
        m_noObjectGlowButton = CCMenuItemLabel::create(makeLabel("NO OBJECT GLOW: OFF"), this, menu_selector(UltimateMenuPopup::onToggleNoObjectGlow));
        m_noObjectGlowButton->setTag(41); m_noObjectGlowButton->setScale(0.55f); menu->addChild(m_noObjectGlowButton);

        m_tpsToggle = CCMenuItemLabel::create(makeLabel("CUSTOM TPS BYPASS: OFF"), this, menu_selector(UltimateMenuPopup::onToggleTpsBypass));
        m_tpsToggle->setTag(40); m_tpsToggle->setScale(0.55f); menu->addChild(m_tpsToggle);
        m_tpsApply = CCMenuItemLabel::create(makeLabel("APPLY TPS"), this, menu_selector(UltimateMenuPopup::onApplyTPS));
        m_tpsApply->setTag(43); m_tpsApply->setScale(0.55f); menu->addChild(m_tpsApply);
        m_bypassControls = CCNode::create();
        m_mainLayer->addChild(m_bypassControls, 7);
        m_tpsInput = TextInput::create(130.f, "TPS (30-360)");
        m_tpsInput->setCommonFilter(CommonFilter::Int);
        m_tpsInput->setString(fmt::format("{:.0f}", g_customTPS));
        m_tpsInput->setPosition({center.x, 66.f});
        m_tpsInput->setScale(0.8f);
        m_bypassControls->addChild(m_tpsInput);

        m_mainLevelBypassToggle = CCMenuItemLabel::create(makeLabel("MAIN LEVEL BYPASS: OFF"), this, menu_selector(UltimateMenuPopup::onToggleMainLevelBypass));
        m_mainLevelBypassToggle->setTag(41); m_mainLevelBypassToggle->setScale(0.55f); menu->addChild(m_mainLevelBypassToggle);
        m_sliderLimitBypassToggle = CCMenuItemLabel::create(makeLabel("SLIDER LIMIT BYPASS: OFF"), this, menu_selector(UltimateMenuPopup::onToggleSliderLimitBypass));
        m_sliderLimitBypassToggle->setTag(42); m_sliderLimitBypassToggle->setScale(0.55f); menu->addChild(m_sliderLimitBypassToggle);
        m_towerLevelBypassToggle = CCMenuItemLabel::create(makeLabel("TOWER LEVEL BYPASS: OFF"), this, menu_selector(UltimateMenuPopup::onToggleTowerLevelBypass));
        m_towerLevelBypassToggle->setTag(44); m_towerLevelBypassToggle->setScale(0.55f); menu->addChild(m_towerLevelBypassToggle);
        m_pauseBufferBypassToggle = CCMenuItemLabel::create(makeLabel("PAUSE BUFFER BYPASS: OFF"), this, menu_selector(UltimateMenuPopup::onTogglePauseBufferBypass));
        m_pauseBufferBypassToggle->setTag(45); m_pauseBufferBypassToggle->setScale(0.55f); menu->addChild(m_pauseBufferBypassToggle);
        m_freeShoppingToggle = CCMenuItemLabel::create(makeLabel("FREE SHOPPING: OFF"), this, menu_selector(UltimateMenuPopup::onToggleFreeShopping));
        m_freeShoppingToggle->setTag(46); m_freeShoppingToggle->setScale(0.55f); menu->addChild(m_freeShoppingToggle);
        m_instantCompleteButton = CCMenuItemLabel::create(makeLabel("INSTANT COMPLETE"), this, menu_selector(UltimateMenuPopup::onInstantComplete));
        m_instantCompleteButton->setTag(47); m_instantCompleteButton->setScale(0.55f); menu->addChild(m_instantCompleteButton);

        m_tabHint = CCLabelBMFont::create("Choose a tab", "bigFont.fnt");
        m_tabHint->setScale(0.42f);
        m_tabHint->setPosition({center.x, 43.f});
        m_mainLayer->addChild(m_tabHint, 5);

        auto tabs = CCMenu::create();
        tabs->setPosition({center.x, 22.f});
        m_mainLayer->addChild(tabs, 6);
        const char* tabNames[] = {"PLAYER", "CHEATS", "BYPASS", "INTERFACE"};
        for (int i = 0; i < 4; ++i) {
            auto label = CCLabelBMFont::create(tabNames[i], "bigFont.fnt");
            auto item = CCMenuItemLabel::create(label, this, menu_selector(UltimateMenuPopup::onTab));
            item->setTag(i);
            item->setScale(0.36f);
            item->setPosition({-135.f + i * 90.f, 0.f});
            tabs->addChild(item);
        }
        this->switchTab(0);
        this->refreshLabels();
        return true;
    }

    void switchTab(int tab) {
        if (!m_itemsMenu) return;
        m_itemsMenu->setVisible(true);
        m_playerOnlyControls->setVisible(tab == 0);
        if (m_bypassControls) m_bypassControls->setVisible(tab == 1);
        int row = 0;
        const float rowY[] = {100.f, 68.f, 36.f, 4.f, -28.f, -60.f, -92.f, -124.f, -156.f, -188.f, -220.f, -252.f, -284.f, -316.f, -348.f, -380.f, -412.f, -444.f, -476.f, -508.f, -540.f, -572.f, -604.f, -636.f};
        for (auto child : CCArrayExt<CCNode*>(m_itemsMenu->getChildren())) {
            auto item = typeinfo_cast<CCMenuItem*>(child);
            if (!item) continue;
            int tag = item->getTag();
            bool visible = (tab == 0 && tag >= 10 && tag <= 17) ||
                           (tab == 1 && tag >= 18 && tag <= 41) ||
                           (tab == 2 && tag >= 40 && tag <= 47) ||
                           (tab == 3 && tag >= 50 && tag <= 55);
            item->setVisible(visible);
            item->setEnabled(visible);
            if (visible) {
                item->setPosition({0.f, rowY[std::min(row, 23)]});
                ++row;
            }
        }
        const char* hints[] = {"Player settings", "Extra cheat options", "TPS, level and pause bypasses", "Floating button appearance"};
        m_tabHint->setString(hints[std::clamp(tab, 0, 3)]);
    }

    void onTab(CCObject* sender) {
        this->switchTab(sender->getTag());
    }

    void refreshLabels() {
        m_speedToggle->setString(Mod::get()->getSettingValue<bool>("speedhack-enabled") ? "Speedhack: ON" : "Speedhack: OFF");
        m_gravityToggle->setString(Mod::get()->getSettingValue<bool>("low-gravity") ? "Low gravity: ON" : "Low gravity: OFF");
        m_jumpToggle->setString(Mod::get()->getSettingValue<bool>("jump-effect") ? "Jump effect: ON" : "Jump effect: OFF");
        m_deathToggle->setString(Mod::get()->getSettingValue<bool>("death-effect") ? "Death effect: ON" : "Death effect: OFF");
        m_noclipToggle->setString(Mod::get()->getSettingValue<bool>("noclip-enabled") ? "NoClip: ON" : "NoClip: OFF");
        if (m_practiceModeButton) m_practiceModeButton->setString(g_practiceModeEnabled ? "PRACTICE MODE: ON" : "PRACTICE MODE: OFF");
        if (m_ghostEffectButton) m_ghostEffectButton->setString(g_ghostEffectEnabled ? "GHOST EFFECT: ON" : "GHOST EFFECT: OFF");
        if (m_hideButton) m_hideButton->setString(g_hideInLevelButton ? "HIDE IN-LEVEL BUTTON: ON" : "HIDE IN-LEVEL BUTTON: OFF");
        if (m_cheatIndicatorButton) m_cheatIndicatorButton->setString(g_cheatIndicatorEnabled ? "CHEAT INDICATOR: ON" : "CHEAT INDICATOR: OFF");
        if (m_tintOnDeathButton) m_tintOnDeathButton->setString(g_tintOnDeathEnabled ? "TINT ON DEATH: ON" : "TINT ON DEATH: OFF");
        if (m_autoPracticeButton) m_autoPracticeButton->setString(g_autoPracticeEnabled ? "AUTO PRACTICE: ON" : "AUTO PRACTICE: OFF");
        if (m_hidePlayerButton) m_hidePlayerButton->setString(g_hidePlayerEnabled ? "HIDE PLAYER: ON" : "HIDE PLAYER: OFF");
        if (m_autoRetryButton) m_autoRetryButton->setString(g_autoRetryEnabled ? "AUTO RETRY: ON" : "AUTO RETRY: OFF");
        if (m_autoClickerButton) m_autoClickerButton->setString(g_autoClickerEnabled ? "AUTO CLICKER: ON" : "AUTO CLICKER: OFF");
        if (m_noTrailButton) m_noTrailButton->setString(g_noTrailEnabled ? "NO TRAIL: ON" : "NO TRAIL: OFF");
        if (m_noGlowButton) m_noGlowButton->setString(g_noPlayerGlowEnabled ? "NO PLAYER GLOW: ON" : "NO PLAYER GLOW: OFF");
        if (m_forcePlatformerButton) m_forcePlatformerButton->setString(g_forcePlatformerEnabled ? "FORCE PLATFORMER: ON" : "FORCE PLATFORMER: OFF");
        if (m_waveTrailButton) m_waveTrailButton->setString(g_waveTrailEnabled ? "WAVE TRAIL: ON" : "WAVE TRAIL: OFF");
        if (m_noParticlesButton) m_noParticlesButton->setString(g_noParticlesEnabled ? "NO PARTICLES: ON" : "NO PARTICLES: OFF");
        if (m_noRotationButton) m_noRotationButton->setString(g_noRotationEnabled ? "NO ROTATION: ON" : "NO ROTATION: OFF");
        if (m_noShakeButton) m_noShakeButton->setString(g_noCameraShakeEnabled ? "NO CAMERA SHAKE: ON" : "NO CAMERA SHAKE: OFF");
        if (m_noOrbPulseButton) m_noOrbPulseButton->setString(g_noOrbPulseEnabled ? "NO ORB PULSE: ON" : "NO ORB PULSE: OFF");
        if (m_solidWaveButton) m_solidWaveButton->setString(g_solidWaveTrailEnabled ? "SOLID WAVE TRAIL: ON" : "SOLID WAVE TRAIL: OFF");
        if (m_noMirrorButton) m_noMirrorButton->setString(g_noMirrorPortalEnabled ? "NO MIRROR PORTAL: ON" : "NO MIRROR PORTAL: OFF");
        if (m_instantMirrorButton) m_instantMirrorButton->setString(g_instantMirrorPortalEnabled ? "INSTANT MIRROR: ON" : "INSTANT MIRROR: OFF");
        if (m_fastChestsButton) m_fastChestsButton->setString(g_fastChestsEnabled ? "FAST CHESTS: ON" : "FAST CHESTS: OFF");
        if (m_chestBypassButton) m_chestBypassButton->setString(g_chestKeyBypassEnabled ? "CHEST KEY BYPASS: ON" : "CHEST KEY BYPASS: OFF");
        if (m_autoCoinsButton) m_autoCoinsButton->setString(g_autoCollectCoinsEnabled ? "AUTO COLLECT COINS: ON" : "AUTO COLLECT COINS: OFF");
        if (m_noDashFireButton) m_noDashFireButton->setString(g_noDashFireEnabled ? "NO DASH FIRE: ON" : "NO DASH FIRE: OFF");
        if (m_noRobotFireButton) m_noRobotFireButton->setString(g_noRobotFireEnabled ? "NO ROBOT FIRE: ON" : "NO ROBOT FIRE: OFF");
        if (m_noShipFireButton) m_noShipFireButton->setString(g_noShipFireEnabled ? "NO SHIP FIRE: ON" : "NO SHIP FIRE: OFF");
        if (m_noRespawnBlinkButton) m_noRespawnBlinkButton->setString(g_noRespawnBlinkEnabled ? "NO RESPAWN BLINK: ON" : "NO RESPAWN BLINK: OFF");
        if (m_noWavePulseButton) m_noWavePulseButton->setString(g_noWavePulseEnabled ? "NO WAVE PULSE: ON" : "NO WAVE PULSE: OFF");
        if (m_noObjectGlowButton) m_noObjectGlowButton->setString(g_noObjectGlowEnabled ? "NO OBJECT GLOW: ON" : "NO OBJECT GLOW: OFF");
        if (m_tpsToggle) m_tpsToggle->setString(g_tpsBypassEnabled ? "CUSTOM TPS BYPASS: ON" : "CUSTOM TPS BYPASS: OFF");
        if (m_tpsApply) m_tpsApply->setString(fmt::format("APPLY TPS ({:.0f})", g_customTPS).c_str());
        if (m_mainLevelBypassToggle) m_mainLevelBypassToggle->setString(g_mainLevelBypassEnabled ? "MAIN LEVEL BYPASS: ON" : "MAIN LEVEL BYPASS: OFF");
        if (m_sliderLimitBypassToggle) m_sliderLimitBypassToggle->setString(g_sliderLimitBypassEnabled ? "SLIDER LIMIT BYPASS: ON" : "SLIDER LIMIT BYPASS: OFF");
        if (m_towerLevelBypassToggle) m_towerLevelBypassToggle->setString(g_towerLevelBypassEnabled ? "TOWER LEVEL BYPASS: ON" : "TOWER LEVEL BYPASS: OFF");
        if (m_pauseBufferBypassToggle) m_pauseBufferBypassToggle->setString(g_pauseBufferBypassEnabled ? "PAUSE BUFFER BYPASS: ON" : "PAUSE BUFFER BYPASS: OFF");
        if (m_freeShoppingToggle) m_freeShoppingToggle->setString(g_freeShoppingEnabled ? "FREE SHOPPING: ON" : "FREE SHOPPING: OFF");
        if (m_colorCycle) m_colorCycle->setString(fmt::format("BUTTON COLOR: {}", g_buttonColorIndex + 1).c_str());
        if (m_opacityCycle) m_opacityCycle->setString(fmt::format("OPACITY: {}%", static_cast<int>(g_buttonOpacities[g_buttonOpacityIndex]) * 100 / 255).c_str());
        if (m_sizeCycle) m_sizeCycle->setString(fmt::format("SIZE: {}%", static_cast<int>(g_buttonScales[g_buttonScaleIndex] * 100)).c_str());
    }

    void onToggle(CCObject* sender) {
        switch (sender->getTag()) {
            case 10: Mod::get()->setSettingValue("speedhack-enabled", !Mod::get()->getSettingValue<bool>("speedhack-enabled")); break;
            case 11: Mod::get()->setSettingValue("low-gravity", !Mod::get()->getSettingValue<bool>("low-gravity")); break;
            case 12: Mod::get()->setSettingValue("jump-effect", !Mod::get()->getSettingValue<bool>("jump-effect")); break;
            case 13: Mod::get()->setSettingValue("death-effect", !Mod::get()->getSettingValue<bool>("death-effect")); break;
            case 14: Mod::get()->setSettingValue("noclip-enabled", !Mod::get()->getSettingValue<bool>("noclip-enabled")); break;
        }
        this->refreshLabels();
    }

    void onToggleMainLevelBypass(CCObject*) {
        g_mainLevelBypassEnabled = !g_mainLevelBypassEnabled;
        this->refreshLabels();
    }

    void onToggleSliderLimitBypass(CCObject*) {
        g_sliderLimitBypassEnabled = !g_sliderLimitBypassEnabled;
        this->refreshLabels();
    }

    void onToggleTowerLevelBypass(CCObject*) {
        g_towerLevelBypassEnabled = !g_towerLevelBypassEnabled;
        this->refreshLabels();
    }

    void onTogglePauseBufferBypass(CCObject*) {
        g_pauseBufferBypassEnabled = !g_pauseBufferBypassEnabled;
        this->refreshLabels();
    }

    void onToggleFreeShopping(CCObject*) {
        g_freeShoppingEnabled = !g_freeShoppingEnabled;
        this->refreshLabels();
    }

    void onTogglePracticeMode(CCObject*) {
        auto layer = PlayLayer::get();
        if (!layer) {
            FLAlertLayer::create("Practice Mode", "Open a level first.", "OK")->show();
            return;
        }
        g_practiceModeEnabled = !g_practiceModeEnabled;
        layer->togglePracticeMode(g_practiceModeEnabled);
        this->refreshLabels();
    }

    void onToggleGhostEffect(CCObject*) {
        auto layer = PlayLayer::get();
        if (!layer) {
            FLAlertLayer::create("Ghost Effect", "Open a level first.", "OK")->show();
            return;
        }
        g_ghostEffectEnabled = !g_ghostEffectEnabled;
        layer->toggleGhostEffect(g_ghostEffectEnabled);
        this->refreshLabels();
    }

    void onInstantComplete(CCObject*) {
        auto layer = PlayLayer::get();
        if (!layer) {
            FLAlertLayer::create("Instant Complete", "Open a level first.", "OK")->show();
            return;
        }
        layer->levelComplete();
    }

    void onToggleTpsBypass(CCObject*) {
        g_tpsBypassEnabled = !g_tpsBypassEnabled;
        g_tpsAccumulator = 0.0;
        this->refreshLabels();
    }

    void onApplyTPS(CCObject*) {
        auto parsed = numFromString<int>(m_tpsInput->getString());
        if (!parsed || *parsed < 30 || *parsed > 360) {
            FLAlertLayer::create("Invalid TPS", "Enter a value from 30 to 360", "OK")->show();
            return;
        }
        g_customTPS = static_cast<double>(*parsed);
        g_tpsAccumulator = 0.0;
        this->refreshLabels();
        FLAlertLayer::create("TPS saved", fmt::format("Custom TPS set to {:.0f}", g_customTPS), "OK")->show();
    }

    void onCycleColor(CCObject*) {
        g_buttonColorIndex = (g_buttonColorIndex + 1) % (sizeof(g_buttonColors) / sizeof(g_buttonColors[0]));
        refreshButtonAppearance();
        this->refreshLabels();
    }

    void onCycleOpacity(CCObject*) {
        g_buttonOpacityIndex = (g_buttonOpacityIndex + 1) % (sizeof(g_buttonOpacities) / sizeof(g_buttonOpacities[0]));
        refreshButtonAppearance();
        this->refreshLabels();
    }

    void onCycleSize(CCObject*) {
        g_buttonScaleIndex = (g_buttonScaleIndex + 1) % (sizeof(g_buttonScales) / sizeof(g_buttonScales[0]));
        refreshButtonAppearance();
        this->refreshLabels();
    }

    void onToggleAutoPractice(CCObject*) {
        g_autoPracticeEnabled = !g_autoPracticeEnabled;
        if (auto layer = PlayLayer::get()) layer->togglePracticeMode(g_autoPracticeEnabled);
        this->refreshLabels();
    }

    void onToggleNoTrail(CCObject*) {
        g_noTrailEnabled = !g_noTrailEnabled;
        this->refreshLabels();
    }

    void onToggleNoGlow(CCObject*) {
        g_noPlayerGlowEnabled = !g_noPlayerGlowEnabled;
        this->refreshLabels();
    }

    void onToggleForcePlatformer(CCObject*) {
        g_forcePlatformerEnabled = !g_forcePlatformerEnabled;
        this->refreshLabels();
    }

    void onToggleWaveTrail(CCObject*) {
        g_waveTrailEnabled = !g_waveTrailEnabled;
        this->refreshLabels();
    }

    void onToggleNoParticles(CCObject*) { g_noParticlesEnabled = !g_noParticlesEnabled; this->refreshLabels(); }
    void onToggleNoRotation(CCObject*) { g_noRotationEnabled = !g_noRotationEnabled; this->refreshLabels(); }
    void onToggleNoShake(CCObject*) { g_noCameraShakeEnabled = !g_noCameraShakeEnabled; this->refreshLabels(); }
    void onToggleNoOrbPulse(CCObject*) { g_noOrbPulseEnabled = !g_noOrbPulseEnabled; this->refreshLabels(); }
    void onToggleSolidWave(CCObject*) { g_solidWaveTrailEnabled = !g_solidWaveTrailEnabled; this->refreshLabels(); }
    void onToggleNoMirror(CCObject*) { g_noMirrorPortalEnabled = !g_noMirrorPortalEnabled; this->refreshLabels(); }
    void onToggleInstantMirror(CCObject*) { g_instantMirrorPortalEnabled = !g_instantMirrorPortalEnabled; this->refreshLabels(); }
    void onToggleFastChests(CCObject*) { g_fastChestsEnabled = !g_fastChestsEnabled; this->refreshLabels(); }
    void onToggleChestBypass(CCObject*) { g_chestKeyBypassEnabled = !g_chestKeyBypassEnabled; this->refreshLabels(); }
    void onToggleAutoCoins(CCObject*) { g_autoCollectCoinsEnabled = !g_autoCollectCoinsEnabled; this->refreshLabels(); }
    void onToggleNoDashFire(CCObject*) { g_noDashFireEnabled = !g_noDashFireEnabled; this->refreshLabels(); }
    void onToggleNoRobotFire(CCObject*) { g_noRobotFireEnabled = !g_noRobotFireEnabled; this->refreshLabels(); }
    void onToggleNoShipFire(CCObject*) { g_noShipFireEnabled = !g_noShipFireEnabled; this->refreshLabels(); }
    void onToggleNoRespawnBlink(CCObject*) { g_noRespawnBlinkEnabled = !g_noRespawnBlinkEnabled; this->refreshLabels(); }
    void onToggleNoWavePulse(CCObject*) { g_noWavePulseEnabled = !g_noWavePulseEnabled; this->refreshLabels(); }
    void onToggleNoObjectGlow(CCObject*) { g_noObjectGlowEnabled = !g_noObjectGlowEnabled; this->refreshLabels(); }

    void onToggleAutoRetry(CCObject*) {
        g_autoRetryEnabled = !g_autoRetryEnabled;
        this->refreshLabels();
    }

    void onToggleAutoClicker(CCObject*) {
        g_autoClickerEnabled = !g_autoClickerEnabled;
        g_autoClickerTimer = 0.f;
        if (!g_autoClickerEnabled && g_autoClickerPressed) {
            if (auto pl = PlayLayer::get(); pl && pl->m_player1) pl->m_player1->releaseButton(PlayerButton::Jump);
            g_autoClickerPressed = false;
        }
        this->refreshLabels();
    }

    void onToggleHidePlayer(CCObject*) {
        g_hidePlayerEnabled = !g_hidePlayerEnabled;
        if (auto layer = PlayLayer::get()) {
            if (layer->m_player1) layer->m_player1->setVisible(!g_hidePlayerEnabled);
            if (layer->m_player2) layer->m_player2->setVisible(!g_hidePlayerEnabled);
        }
        this->refreshLabels();
    }

    void onToggleTintOnDeath(CCObject*) { g_tintOnDeathEnabled = !g_tintOnDeathEnabled; this->refreshLabels(); }

    void onToggleCheatIndicator(CCObject*) {
        g_cheatIndicatorEnabled = !g_cheatIndicatorEnabled;
        this->refreshLabels();
    }

    void onToggleHideButton(CCObject*) {
        g_hideInLevelButton = !g_hideInLevelButton;
        if (auto pl = PlayLayer::get()) {
            if (auto node = pl->getChildByID("gd-ultimate-game-menu"_spr)) node->setVisible(!g_hideInLevelButton);
        }
        this->refreshLabels();
    }

    void onApplySpeed(CCObject*) {
        auto parsed = numFromString<double>(m_speedInput->getString());
        if (!parsed || *parsed < 0.5 || *parsed > 3.0) {
            FLAlertLayer::create("Invalid speed", "Enter a number from 0.5 to 3.0", "OK")->show();
            return;
        }
        Mod::get()->setSettingValue("speed-multiplier", *parsed);
        FLAlertLayer::create("Speed saved", fmt::format("Speed multiplier set to {:.2f}x", *parsed), "OK")->show();
    }

public:
    static UltimateMenuPopup* create() {
        auto ret = new UltimateMenuPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

static void openUltimateMenu() {
    UltimateMenuPopup::create()->show();
}

static void applyButtonAppearance(CCNode* node) {
    if (!node) return;
    if (node->getID() == "gd-ultimate-menu-button"_spr) {
        if (auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(node)) {
            button->setColor(g_buttonColors[g_buttonColorIndex]);
            button->setOpacity(g_buttonOpacities[g_buttonOpacityIndex]);
            button->setScale(g_buttonScales[g_buttonScaleIndex]);
        }
    }
    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        applyButtonAppearance(child);
    }
}

static void refreshButtonAppearance() {
    if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
        applyButtonAppearance(scene);
    }
}

static CCMenuItemSpriteExtra* createUltimateButton(CCNode* target, SEL_MenuHandler selector) {
    auto spr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");
    if (!spr) return nullptr;
    auto button = CCMenuItemSpriteExtra::create(spr, target, selector);
    button->setID("gd-ultimate-menu-button"_spr);
    button->setColor(g_buttonColors[g_buttonColorIndex]);
    button->setOpacity(g_buttonOpacities[g_buttonOpacityIndex]);
    button->setScale(g_buttonScales[g_buttonScaleIndex]);
    return button;
}

class $modify(GDUltimateMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return true;
        auto button = createUltimateButton(this, menu_selector(GDUltimateMenuLayer::onOpenUltimateMenu));
        if (!button) return true;
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }
    void onOpenUltimateMenu(CCObject*) { openUltimateMenu(); }
};


class $modify(GDUltimatePauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        if (!menu) return;
        menu->setPosition({winSize.width - 38.f, winSize.height - 38.f});
        menu->setZOrder(10000);
        auto button = createUltimateButton(this, menu_selector(GDUltimatePauseLayer::onOpenUltimateMenu));
        if (!button) return;
        menu->addChild(button);
        this->addChild(menu, 10000);
    }
    void onOpenUltimateMenu(CCObject*) { openUltimateMenu(); }
};

class $modify(GDUltimatePlayerObject, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        return PlayerObject::pushButton(btn);
    }

    bool releaseButton(PlayerButton btn) {
        return PlayerObject::releaseButton(btn);
    }
    void update(float dt) {
        this->m_hasGroundParticles = Mod::get()->getSettingValue<bool>("jump-effect") && !g_noParticlesEnabled;
        this->m_hasShipParticles = Mod::get()->getSettingValue<bool>("jump-effect") && !g_noParticlesEnabled;
        if (this->m_ghostTrail) this->m_ghostTrail->setVisible(!g_noTrailEnabled);
        if (this->m_waveTrail) this->m_waveTrail->setVisible(!g_noTrailEnabled && g_waveTrailEnabled);
        this->m_hasGlow = !g_noPlayerGlowEnabled;
        if (auto pl = PlayLayer::get()) {
            bool isP1 = this == pl->m_player1;
            bool isP2 = this == pl->m_player2;
            if (isP1 || isP2) {
                auto& tracked = isP1 ? g_platformerTrackedP1 : g_platformerTrackedP2;
                auto& original = isP1 ? g_platformerOriginalP1 : g_platformerOriginalP2;
                if (tracked != this) {
                    tracked = this;
                    original = this->m_isPlatformer;
                }
                this->m_isPlatformer = g_forcePlatformerEnabled ? true : original;
            }
        }
        if (auto pl = PlayLayer::get()) {
            bool isP1 = this == pl->m_player1;
            bool isP2 = this == pl->m_player2;
            if (isP1 && g_autoClickerEnabled) {
                g_autoClickerTimer += dt;
                if (g_autoClickerPressed && g_autoClickerTimer >= 0.055f) {
                    this->releaseButton(PlayerButton::Jump);
                    g_autoClickerPressed = false;
                    g_autoClickerTimer = 0.f;
                } else if (!g_autoClickerPressed && g_autoClickerTimer >= 0.11f) {
                    this->pushButton(PlayerButton::Jump);
                    g_autoClickerPressed = true;
                    g_autoClickerTimer = 0.f;
                }
            } else if (isP1 && g_autoClickerPressed) {
                this->releaseButton(PlayerButton::Jump);
                g_autoClickerPressed = false;
                g_autoClickerTimer = 0.f;
            }
            if (isP1 || isP2) {
                this->setVisible(!g_hidePlayerEnabled);
                auto& tracked = isP1 ? g_gravityTrackedP1 : g_gravityTrackedP2;
                auto& base = isP1 ? g_gravityBaseP1 : g_gravityBaseP2;
                auto& last = isP1 ? g_gravityLastP1 : g_gravityLastP2;
                if (tracked != this) { tracked = this; base = this->m_gravity; last = this->m_gravity; }
                else if (this->m_gravity != last) base = this->m_gravity;
                this->m_gravity = Mod::get()->getSettingValue<bool>("low-gravity") ? base * 0.5 : base;
                last = this->m_gravity;
            }
        }
        PlayerObject::update(dt);
        if (g_noDashFireEnabled && this->m_dashFireSprite) this->m_dashFireSprite->setVisible(false);
        if (g_noRobotFireEnabled) {
            if (this->m_robotFire) this->m_robotFire->setVisible(false);
            if (this->m_robotBurstParticles) this->m_robotBurstParticles->setVisible(false);
        }
        if (g_noShipFireEnabled && this->m_shipStreak) this->m_shipStreak->setVisible(false);
        if (g_noRotationEnabled) {
            this->setRotation(0.f);
            if (this->m_iconSprite) this->m_iconSprite->setRotation(0.f);
        }
    }

    void playSpawnEffect() {
        if (g_noRespawnBlinkEnabled) return;
        PlayerObject::playSpawnEffect();
    }

    void playDeathEffect() {
        if (!Mod::get()->getSettingValue<bool>("death-effect")) return;
        PlayerObject::playDeathEffect();
    }
};

class $modify(GDUltimatePlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        g_cheatIndicatorLabel = CCLabelBMFont::create("GD MENU: CHEATS ACTIVE", "bigFont.fnt");
        if (g_cheatIndicatorLabel) {
            g_cheatIndicatorLabel->setScale(0.38f);
            g_cheatIndicatorLabel->setPosition({winSize.width / 2.f, winSize.height - 18.f});
            g_cheatIndicatorLabel->setVisible(false);
            g_cheatIndicatorLabel->setID("gd-ultimate-cheat-indicator"_spr);
            this->addChild(g_cheatIndicatorLabel, 1000);
        }
        if (g_autoPracticeEnabled) this->togglePracticeMode(true);
        // Optional in-level menu button.
        if (!g_hideInLevelButton) {
            auto menu = CCMenu::create();
            if (menu) {
                menu->setID("gd-ultimate-game-menu"_spr);
                menu->setPosition({36.f, winSize.height - 36.f});
                menu->setZOrder(999);
                auto button = createUltimateButton(this, menu_selector(GDUltimatePlayLayer::onOpenUltimateMenu));
                if (button) {
                    button->setScale(0.72f);
                    menu->addChild(button);
                    this->addChild(menu, 999);
                }
            }
        }

        return true;
    }
    void updateVisibility(float dt) {
        auto audio = FMODAudioEngine::get();
        float oldPulse = audio ? audio->m_pulse1 : 0.f;
        float oldScale = m_audioEffectsLayer ? m_audioEffectsLayer->m_audioScale : 0.f;
        if (g_noOrbPulseEnabled) {
            if (audio) audio->m_pulse1 = 0.7f;
            if (m_audioEffectsLayer) m_audioEffectsLayer->m_audioScale = 0.7f;
        }
        PlayLayer::updateVisibility(dt);
        if (g_noOrbPulseEnabled) {
            if (audio) audio->m_pulse1 = oldPulse;
            if (m_audioEffectsLayer) m_audioEffectsLayer->m_audioScale = oldScale;
        }
    }

    void addObject(GameObject* obj) {
        PlayLayer::addObject(obj);
        if (obj && (obj->m_objectType == GameObjectType::UserCoin || obj->m_objectType == GameObjectType::SecretCoin))
            g_levelCoins.push_back(static_cast<EffectGameObject*>(obj));
        if (obj && g_noObjectGlowEnabled) obj->m_hasNoGlow = true;
    }

    void update(float dt) {
        if (g_cheatIndicatorLabel) {
            bool active = Mod::get()->getSettingValue<bool>("speedhack-enabled") ||
                Mod::get()->getSettingValue<bool>("low-gravity") ||
                Mod::get()->getSettingValue<bool>("noclip-enabled") ||
                g_tpsBypassEnabled || g_practiceModeEnabled || g_autoPracticeEnabled ||
                g_autoRetryEnabled || g_autoClickerEnabled || g_noTrailEnabled ||
                g_noPlayerGlowEnabled || g_forcePlatformerEnabled || g_freeShoppingEnabled ||
                g_hidePlayerEnabled || g_waveTrailEnabled == false || g_mainLevelBypassEnabled ||
                g_sliderLimitBypassEnabled || g_towerLevelBypassEnabled || g_pauseBufferBypassEnabled;
            g_cheatIndicatorLabel->setVisible(g_cheatIndicatorEnabled && active);
        }
        // Fixed 240 Hz simulation ticks, capped to avoid a runaway catch-up loop.
        if (g_tpsBypassEnabled && !g_tpsInternalTick) {
            g_tpsAccumulator += std::max(0.0f, dt);
            const double fixedStep = 1.0 / g_customTPS;
            int ticks = 0;
            while (g_tpsAccumulator >= fixedStep && ticks < 8) {
                g_tpsAccumulator -= fixedStep;
                g_tpsInternalTick = true;
                this->update(static_cast<float>(fixedStep));
                g_tpsInternalTick = false;
                ++ticks;
            }
            return;
        }
        PlayLayer::update(dt);
    }
    void resetLevel() {
        PlayLayer::resetLevel();
        if (g_autoCollectCoinsEnabled) {
            for (auto coin : g_levelCoins) {
                if (coin && coin->isVisible()) {
                    this->pickupItem(coin);
                    this->destroyObject(coin);
                }
            }
        }
    }

    void retryAfterDeath(float) {
        if (g_autoRetryEnabled && PlayLayer::get() == this && !m_isPaused) this->resetLevel();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (Mod::get()->getSettingValue<bool>("noclip-enabled") &&
            player && (player == m_player1 || player == m_player2)) {
            return;
        }
        if (g_tintOnDeathEnabled && player && (player == m_player1 || player == m_player2)) player->setColor({255, 80, 80});
        PlayLayer::destroyPlayer(player, object);
        if (g_autoRetryEnabled && player && (player == m_player1 || player == m_player2)) {
            this->scheduleOnce(schedule_selector(GDUltimatePlayLayer::retryAfterDeath), 0.45f);
        }
    }

    void onOpenUltimateMenu(CCObject*) { openUltimateMenu(); }
};

class $modify(GDUltimateMainLevelBypass, LevelPage) {
    void onPlay(CCObject* sender) {
        if (!g_mainLevelBypassEnabled || !m_level) {
            LevelPage::onPlay(sender);
            return;
        }
        auto requiredCoins = m_level->m_requiredCoins;
        m_level->m_requiredCoins = 0;
        LevelPage::onPlay(sender);
        m_level->m_requiredCoins = requiredCoins;
    }
};

class $modify(GDUltimateTowerLevelBypass, LevelAreaInnerLayer) {
    bool init(bool p0) {
        if (!LevelAreaInnerLayer::init(p0)) return false;
        if (!g_towerLevelBypassEnabled) return true;
        auto container = this->getChildByType<CCNode>(1);
        if (!container) return true;
        auto menu = container->getChildByType<CCMenu>(0);
        if (!menu) return true;
        for (auto child : CCArrayExt<CCMenuItem*>(menu->getChildren())) {
            if (child) child->setEnabled(true);
        }
        return true;
    }
};

class $modify(GDUltimatePauseBufferBypass, UILayer) {
    void onPause(CCObject* sender) {
        if (g_pauseBufferBypassEnabled && m_gameLayer) m_gameLayer->m_gameState.m_pauseCounter = 0;
        UILayer::onPause(sender);
    }
    void handleKeypress(enumKeyCodes key, bool down, double timestamp) {
        if (g_pauseBufferBypassEnabled && m_gameLayer) m_gameLayer->m_gameState.m_pauseCounter = 0;
        UILayer::handleKeypress(key, down, timestamp);
    }
};

class $modify(GDUltimateSliderLimitBypass, SliderTouchLogic) {
    void ccTouchMoved(CCTouch* touch, CCEvent* event) {
        if (!g_sliderLimitBypassEnabled) {
            SliderTouchLogic::ccTouchMoved(touch, event);
            return;
        }
        if (!m_thumb || !m_slider || !touch) return;
        auto position = this->convertToNodeSpace(touch->getLocation());
        if (m_thumb->m_vertical) m_thumb->setPositionY(position.y);
        else m_thumb->setPositionX(position.x);
        m_slider->updateBar();
        m_thumb->activate();
    }
};

class $modify(GDUltimateFreeShopping, GameStatsManager) {
    void recountSpecialStats() {
        if (!g_chestKeyBypassEnabled) GameStatsManager::recountSpecialStats();
    }

    bool purchaseItem(int index) {
        if (!g_freeShoppingEnabled) return GameStatsManager::purchaseItem(index);
        auto item = this->getStoreItem(index);
        if (!item) return GameStatsManager::purchaseItem(index);
        auto originalPrice = static_cast<int>(item->m_price);
        item->m_price = 0;
        bool purchased = GameStatsManager::purchaseItem(index);
        item->m_price = originalPrice;
        return purchased;
    }
};

class $modify(GDUltimateSpeedhackScheduler, cocos2d::CCScheduler) {
    void update(float dt) {
        if (PlayLayer::get() && Mod::get()->getSettingValue<bool>("speedhack-enabled")) {
            auto speed = static_cast<float>(Mod::get()->getSettingValue<double>("speed-multiplier"));
            dt *= std::clamp(speed, 0.5f, 3.0f);
        }
        CCScheduler::update(dt);
    }
};


class $modify(GDUltimateBaseGameLayer, GJBaseGameLayer) {
    void toggleFlipped(bool p0, bool p1) {
        if (g_noMirrorPortalEnabled) return;
        if (g_instantMirrorPortalEnabled) p1 = true;
        GJBaseGameLayer::toggleFlipped(p0, p1);
    }

    void updateCamera(float delta) {
        if (g_noCameraShakeEnabled) {
            m_gameState.m_cameraShakeFactor = 0.f;
            m_gameState.m_cameraShakeEnabled = false;
        }
        GJBaseGameLayer::updateCamera(delta);
    }
};

class $modify(GDUltimateHardStreak, HardStreak) {
    void updateStroke(float dt) {
        if (g_noWavePulseEnabled) this->m_pulseSize = 1.f;
        if (g_solidWaveTrailEnabled) this->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        HardStreak::updateStroke(dt);
    }
};


class $modify(GDUltimateRewardUnlockLayer, RewardUnlockLayer) {
    bool showCollectReward(GJRewardItem* item) {
        auto result = RewardUnlockLayer::showCollectReward(item);
        if (g_fastChestsEnabled) this->showCloseButton();
        return result;
    }
};

class $modify(GDUltimateSecretRewardsLayer, SecretRewardsLayer) {
    void onSelectItem(CCObject* sender) {
        if (!g_chestKeyBypassEnabled) return SecretRewardsLayer::onSelectItem(sender);
        auto keys = GameStatsManager::sharedState()->getStat("21");
        GameStatsManager::sharedState()->setStat("21", 999999);
        SecretRewardsLayer::onSelectItem(sender);
        GameStatsManager::sharedState()->setStat("21", keys);
        if (m_keysLabel) m_keysLabel->setString(fmt::format("{}", keys).c_str());
    }
    void onGoldChest(CCObject* sender) {
        if (!g_chestKeyBypassEnabled) return SecretRewardsLayer::onGoldChest(sender);
        auto keys = GameStatsManager::sharedState()->getStat("43");
        GameStatsManager::sharedState()->setStat("43", 999999);
        SecretRewardsLayer::onGoldChest(sender);
        GameStatsManager::sharedState()->setStat("43", keys);
        if (m_goldKeysLabel) m_goldKeysLabel->setString(fmt::format("{}", keys).c_str());
    }
};
