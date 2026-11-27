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
    bool g_statusLabelEnabled = false;
    bool g_labelCpsEnabled = true;
    bool g_labelTpsEnabled = true;
    bool g_labelSpeedEnabled = true;
    bool g_labelProgressEnabled = false;
    unsigned g_cpsPresses = 0;
    unsigned g_cpsValue = 0;
    unsigned g_tpsFrames = 0;
    unsigned g_tpsValue = 0;
    double g_labelSampleTime = 0.0;
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
static bool shouldShowHudLabels() {
    return g_statusLabelEnabled || g_labelCpsEnabled || g_labelTpsEnabled || g_labelSpeedEnabled || g_labelProgressEnabled;
}

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
    CCMenuItemLabel* m_statusLabelToggle = nullptr;
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
        content->setContentSize({350.f, 400.f});
        auto menu = CCMenu::create();
        m_itemsMenu = menu;
        menu->setPosition({175.f, 290.f});
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
        m_statusLabelToggle = CCMenuItemLabel::create(makeLabel("HUD STATUS LABEL: OFF"), this, menu_selector(UltimateMenuPopup::onToggleStatusLabel));
        m_statusLabelToggle->setTag(60); m_statusLabelToggle->setScale(0.55f); menu->addChild(m_statusLabelToggle);
        auto addLabelOption = [&](char const* text, int tag) {
            auto item = CCMenuItemLabel::create(makeLabel(text), this, menu_selector(UltimateMenuPopup::onToggleLabelOption));
            item->setTag(tag); item->setScale(0.55f); menu->addChild(item);
        };
        addLabelOption("CPS COUNTER: ON", 61);
        addLabelOption("TPS COUNTER: ON", 62);
        addLabelOption("SPEED LABEL: ON", 63);
        addLabelOption("PROGRESS LABEL: OFF", 64);

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
        const char* tabNames[] = {"PLAYER", "BYPASS", "INTERFACE", "LABELS"};
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
        const float rowY[] = {100.f, 68.f, 36.f, 4.f, -28.f, -60.f, -92.f, -124.f};
        for (auto child : CCArrayExt<CCNode*>(m_itemsMenu->getChildren())) {
            auto item = typeinfo_cast<CCMenuItem*>(child);
            if (!item) continue;
            int tag = item->getTag();
            bool visible = (tab == 0 && tag >= 10 && tag <= 17) ||
                           (tab == 1 && tag >= 40 && tag <= 47) ||
                           (tab == 2 && tag >= 50 && tag <= 53) ||
                           (tab == 3 && tag >= 60 && tag <= 64);
            item->setVisible(visible);
            item->setEnabled(visible);
            if (visible) {
                item->setPosition({0.f, rowY[std::min(row, 7)]});
                ++row;
            }
        }
        const char* hints[] = {"Player settings", "TPS, level and pause bypasses", "Floating button appearance", "In-level HUD labels"};
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
        if (m_statusLabelToggle) m_statusLabelToggle->setString(g_statusLabelEnabled ? "HUD LABEL: ON" : "HUD LABEL: OFF");
        if (auto item = typeinfo_cast<CCMenuItemLabel*>(m_itemsMenu->getChildByTag(61))) item->setString(g_labelCpsEnabled ? "CPS COUNTER: ON" : "CPS COUNTER: OFF");
        if (auto item = typeinfo_cast<CCMenuItemLabel*>(m_itemsMenu->getChildByTag(62))) item->setString(g_labelTpsEnabled ? "TPS COUNTER: ON" : "TPS COUNTER: OFF");
        if (auto item = typeinfo_cast<CCMenuItemLabel*>(m_itemsMenu->getChildByTag(63))) item->setString(g_labelSpeedEnabled ? "SPEED LABEL: ON" : "SPEED LABEL: OFF");
        if (auto item = typeinfo_cast<CCMenuItemLabel*>(m_itemsMenu->getChildByTag(64))) item->setString(g_labelProgressEnabled ? "PROGRESS LABEL: ON" : "PROGRESS LABEL: OFF");
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

    void onToggleLabelOption(CCObject* sender) {
        switch (sender->getTag()) {
            case 61: g_labelCpsEnabled = !g_labelCpsEnabled; break;
            case 62: g_labelTpsEnabled = !g_labelTpsEnabled; break;
            case 63: g_labelSpeedEnabled = !g_labelSpeedEnabled; break;
            case 64: g_labelProgressEnabled = !g_labelProgressEnabled; break;
        }
        if (auto pl = PlayLayer::get()) {
            if (auto label = pl->getChildByID("gd-ultimate-status-label"_spr)) label->setVisible(shouldShowHudLabels());
        }
        this->refreshLabels();
    }

    void onToggleStatusLabel(CCObject*) {
        g_statusLabelEnabled = !g_statusLabelEnabled;
        if (auto pl = PlayLayer::get()) {
            if (auto label = pl->getChildByID("gd-ultimate-status-label"_spr)) label->setVisible(shouldShowHudLabels());
        }
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
        this->m_hasGroundParticles = Mod::get()->getSettingValue<bool>("jump-effect");
        this->m_hasShipParticles = Mod::get()->getSettingValue<bool>("jump-effect");
        if (auto pl = PlayLayer::get()) {
            bool isP1 = this == pl->m_player1;
            bool isP2 = this == pl->m_player2;
            if (isP1 || isP2) {
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
    }

    void playDeathEffect() {
        if (!Mod::get()->getSettingValue<bool>("death-effect")) return;
        PlayerObject::playDeathEffect();
    }
};

class $modify(GDUltimatePlayLayer, PlayLayer) {
    void pushButton(PlayerButton button, bool player1) {
        ++g_cpsPresses;
        PlayLayer::pushButton(button, player1);
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto statusLabel = CCLabelBMFont::create("CPS: 0", "bigFont.fnt");
        if (statusLabel) {
            statusLabel->setID("gd-ultimate-status-label"_spr);
            statusLabel->setAnchorPoint({0.f, 1.f});
            statusLabel->setPosition({12.f, winSize.height - 80.f});
            statusLabel->setScale(0.55f);
            statusLabel->setColor({255, 255, 255});
            statusLabel->setVisible(true);
            statusLabel->setZOrder(10000);
            this->addChild(statusLabel, 10000);
        }
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
    void update(float dt) {
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
        g_labelSampleTime += std::max(0.0f, dt);
        ++g_tpsFrames;
        if (g_labelSampleTime >= 1.0) {
            g_cpsValue = g_cpsPresses;
            g_cpsPresses = 0;
            g_tpsValue = g_tpsFrames;
            g_tpsFrames = 0;
            g_labelSampleTime -= 1.0;
        }
        if (auto label = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("gd-ultimate-status-label"_spr))) {
            std::string text;
            if (g_labelCpsEnabled) text += fmt::format("CPS: {}\n", g_cpsValue);
            if (g_labelTpsEnabled) text += fmt::format("TPS: {}\n", g_tpsBypassEnabled ? static_cast<unsigned>(g_customTPS) : g_tpsValue);
            if (g_labelSpeedEnabled) text += fmt::format("SPEED: {:.2f}x\n", Mod::get()->getSettingValue<double>("speed-multiplier"));
            if (g_labelProgressEnabled && m_level) text += fmt::format("PROGRESS: {}%", static_cast<int>(m_gameState.m_levelTime * 100.f / std::max(1, m_level->m_levelLength)));
            label->setString(text.empty() ? "Labels enabled" : text.c_str());
            label->setVisible(shouldShowHudLabels());
            label->setZOrder(10000);
        }
    }
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (Mod::get()->getSettingValue<bool>("noclip-enabled") &&
            player && (player == m_player1 || player == m_player2)) {
            return;
        }
        PlayLayer::destroyPlayer(player, object);
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
