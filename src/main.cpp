#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <vector>
#include <cmath>


using namespace geode::prelude;

namespace {
    struct Re4Input {
        uint64_t frame;
        uint8_t down;
        int32_t button;
        uint8_t player2;
    };
    struct Re4Physics {
        uint64_t frame;
        float x;
        float y;
        double yAccel;
        uint8_t player2;
    };
    std::vector<Re4Input> g_re4Inputs;
    std::vector<Re4Physics> g_re4Physics;
    uint64_t g_re4Frame = 0;
    bool g_re4Recording = false;
    bool g_stepperEnabled = false;
    bool g_stepRequested = false;
    bool g_tpsBypassEnabled = false;
    bool g_tpsInternalTick = false;
    double g_tpsAccumulator = 0.0;
    unsigned g_buttonColorIndex = 0;
    unsigned g_buttonOpacityIndex = 0;
    unsigned g_buttonScaleIndex = 1;
    bool g_hideInLevelButton = false;
    const GLubyte g_buttonOpacities[] = {255, 210, 165, 120};
    const float g_buttonScales[] = {0.55f, 0.72f, 0.88f, 1.0f};
    const ccColor3B g_buttonColors[] = {{255,255,255},{100,220,255},{140,255,140},{255,190,90},{230,130,255},{255,110,130}};
    std::vector<Re4Input> g_re4PlaybackInputs;
    size_t g_re4PlaybackCursor = 0;
    bool g_re4Playback = false;
    PlayerObject* g_gravityTrackedP1 = nullptr;
    PlayerObject* g_gravityTrackedP2 = nullptr;
    double g_gravityBaseP1 = 0.0;
    double g_gravityBaseP2 = 0.0;
    double g_gravityLastP1 = 0.0;
    double g_gravityLastP2 = 0.0;

    template <class T>
    void writeLE(std::ofstream& out, T value) {
        auto bytes = reinterpret_cast<const char*>(&value);
        out.write(bytes, sizeof(T));
    }

    bool saveRe4Replay() {
        auto path = Mod::get()->getSaveDir() / "last-replay.re4";
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        std::ofstream out(path, std::ios::binary);
        if (!out) return false;
        out.write("RE4", 3);
        writeLE<float>(out, 240.0f);
        writeLE<uint64_t>(out, static_cast<uint64_t>(g_re4Physics.size()));
        for (auto const& p : g_re4Physics) {
            writeLE<uint64_t>(out, p.frame);
            writeLE<float>(out, p.x);
            writeLE<float>(out, p.y);
            writeLE<double>(out, p.yAccel);
            writeLE<uint8_t>(out, p.player2);
        }
        writeLE<uint64_t>(out, static_cast<uint64_t>(g_re4Inputs.size()));
        for (auto const& i : g_re4Inputs) {
            writeLE<uint64_t>(out, i.frame);
            writeLE<uint8_t>(out, i.down);
            writeLE<int32_t>(out, i.button);
            writeLE<uint8_t>(out, i.player2);
        }
        return out.good();
    }

    template <class T>
    bool readLE(std::ifstream& in, T& value) {
        in.read(reinterpret_cast<char*>(&value), sizeof(T));
        return in.good();
    }

    bool loadRe4Replay() {
        auto path = Mod::get()->getSaveDir() / "last-replay.re4";
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        char magic[3]{};
        in.read(magic, 3);
        float fps = 0.f;
        uint64_t physicsCount = 0;
        if (!in || std::string(magic, 3) != "RE4" || !readLE(in, fps) ||
            !std::isfinite(fps) || fps < 1.f || fps > 1000.f ||
            !readLE(in, physicsCount) || physicsCount > 5000000) return false;
        for (uint64_t n = 0; n < physicsCount; ++n) {
            uint64_t frame; float x, y; double accel; uint8_t p2;
            if (!readLE(in, frame) || !readLE(in, x) || !readLE(in, y) ||
                !readLE(in, accel) || !readLE(in, p2) ||
                !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(accel) || p2 > 1) return false;
        }
        uint64_t count = 0;
        if (!readLE(in, count) || count == 0 || count > 5000000) return false;
        std::vector<Re4Input> parsed;
        parsed.reserve(static_cast<size_t>(count));
        uint64_t lastFrame = 0;
        for (uint64_t n = 0; n < count; ++n) {
            Re4Input event{};
            if (!readLE(in, event.frame) || !readLE(in, event.down) ||
                !readLE(in, event.button) || !readLE(in, event.player2) ||
                event.down > 1 || event.player2 > 1 || event.button < 0 ||
                event.button > 16 || (n && event.frame < lastFrame)) return false;
            lastFrame = event.frame;
            parsed.push_back(event);
        }
        if (in.peek() != std::ifstream::traits_type::eof()) return false;
        g_re4PlaybackInputs = std::move(parsed);
        g_re4PlaybackCursor = 0;
        return true;
    }

    void resetRe4Recorder() {
        g_re4Inputs.clear();
        g_re4Physics.clear();
        g_re4Frame = 0;
        g_re4PlaybackInputs.clear();
        g_re4PlaybackCursor = 0;
        g_re4Playback = false;
    }
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
    CCMenuItemLabel* m_recordToggle = nullptr;
    CCMenu* m_itemsMenu = nullptr;
    CCLabelBMFont* m_tabHint = nullptr;
    CCMenuItemLabel* m_stepperToggle = nullptr;
    CCMenuItemLabel* m_stepOnce = nullptr;
    CCMenuItemLabel* m_colorCycle = nullptr;
    CCMenuItemLabel* m_replayPlay = nullptr;
    CCMenuItemLabel* m_opacityCycle = nullptr;
    CCMenuItemLabel* m_sizeCycle = nullptr;
    CCMenuItemLabel* m_hideButton = nullptr;
    CCMenuItemLabel* m_tpsToggle = nullptr;
    CCNode* m_playerOnlyControls = nullptr;

    bool init() {
        if (!Popup::init(390.f, 310.f)) return false;
        this->setTitle("GD Ultimate Menu");

        auto layerSize = m_mainLayer->getContentSize();
        auto center = ccp(layerSize.width / 2.f, layerSize.height / 2.f);
        m_scroll = geode::ScrollLayer::create(CCSize{350.f, 220.f});
        m_scroll->setPosition({20.f, 58.f});
        m_scroll->setID("ultimate-menu-scroll"_spr);
        m_mainLayer->addChild(m_scroll);
        auto content = m_scroll->m_contentLayer;
        content->setContentSize({350.f, 220.f});
        auto menu = CCMenu::create();
        m_itemsMenu = menu;
        menu->setPosition({175.f, 110.f});
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
        content->addChild(m_playerOnlyControls);
        m_speedInput = TextInput::create(130.f, "Speed multiplier");
        m_speedInput->setCommonFilter(CommonFilter::Float);
        m_speedInput->setString(fmt::format("{:.2f}", Mod::get()->getSettingValue<double>("speed-multiplier")));
        m_speedInput->setPosition({175.f, 15.f});
        m_speedInput->setScale(0.8f);
        m_playerOnlyControls->addChild(m_speedInput);

        auto apply = CCMenuItemLabel::create(makeLabel("APPLY SPEED"), this,
            menu_selector(UltimateMenuPopup::onApplySpeed));
        apply->setTag(15);
        apply->setScale(0.55f);
        apply->setPosition({0.f, -45.f});
        menu->addChild(apply);

        m_recordToggle = CCMenuItemLabel::create(makeLabel("RE4 recording: OFF"), this,
            menu_selector(UltimateMenuPopup::onToggleRecording));
        m_recordToggle->setTag(20);
        m_recordToggle->setScale(0.55f);
        m_recordToggle->setPosition({0.f, -85.f});
        menu->addChild(m_recordToggle);

        auto saveReplay = CCMenuItemLabel::create(makeLabel("SAVE .RE4"), this,
            menu_selector(UltimateMenuPopup::onSaveReplay));
        saveReplay->setTag(21);
        saveReplay->setScale(0.55f);
        saveReplay->setPosition({0.f, -125.f});
        menu->addChild(saveReplay);

        auto loadReplay = CCMenuItemLabel::create(makeLabel("LOAD .RE4"), this, menu_selector(UltimateMenuPopup::onLoadReplay));
        loadReplay->setTag(22); loadReplay->setScale(0.55f); loadReplay->setPosition({0.f, -165.f}); menu->addChild(loadReplay);
        m_replayPlay = CCMenuItemLabel::create(makeLabel("PLAY REPLAY"), this, menu_selector(UltimateMenuPopup::onPlayReplay));
        m_replayPlay->setTag(23); m_replayPlay->setScale(0.55f); m_replayPlay->setPosition({0.f, -205.f}); menu->addChild(m_replayPlay);
        auto stopReplay = CCMenuItemLabel::create(makeLabel("STOP REPLAY"), this, menu_selector(UltimateMenuPopup::onStopReplay));
        stopReplay->setTag(24); stopReplay->setScale(0.55f); stopReplay->setPosition({0.f, -245.f}); menu->addChild(stopReplay);

        m_stepperToggle = CCMenuItemLabel::create(makeLabel("Frame Stepper: OFF"), this, menu_selector(UltimateMenuPopup::onToggleStepper));
        m_stepperToggle->setTag(30); m_stepperToggle->setScale(0.55f); m_stepperToggle->setPosition({0.f, 115.f}); menu->addChild(m_stepperToggle);
        m_stepOnce = CCMenuItemLabel::create(makeLabel("STEP +1 FRAME"), this, menu_selector(UltimateMenuPopup::onStepOnce));
        m_stepOnce->setTag(31); m_stepOnce->setScale(0.55f); m_stepOnce->setPosition({0.f, 65.f}); menu->addChild(m_stepOnce);
        m_colorCycle = CCMenuItemLabel::create(makeLabel("BUTTON COLOR"), this, menu_selector(UltimateMenuPopup::onCycleColor));
        m_colorCycle->setTag(50); m_colorCycle->setScale(0.55f); menu->addChild(m_colorCycle);
        m_opacityCycle = CCMenuItemLabel::create(makeLabel("BUTTON OPACITY"), this, menu_selector(UltimateMenuPopup::onCycleOpacity));
        m_opacityCycle->setTag(51); m_opacityCycle->setScale(0.55f); menu->addChild(m_opacityCycle);
        m_sizeCycle = CCMenuItemLabel::create(makeLabel("BUTTON SIZE"), this, menu_selector(UltimateMenuPopup::onCycleSize));
        m_sizeCycle->setTag(52); m_sizeCycle->setScale(0.55f); menu->addChild(m_sizeCycle);
        m_hideButton = CCMenuItemLabel::create(makeLabel("HIDE IN-LEVEL BUTTON: OFF"), this, menu_selector(UltimateMenuPopup::onToggleHideButton));
        m_hideButton->setTag(53); m_hideButton->setScale(0.55f); menu->addChild(m_hideButton);

        m_tpsToggle = CCMenuItemLabel::create(makeLabel("TPS BYPASS 240: OFF"), this, menu_selector(UltimateMenuPopup::onToggleTpsBypass));
        m_tpsToggle->setTag(40); m_tpsToggle->setScale(0.55f); menu->addChild(m_tpsToggle);

        m_tabHint = CCLabelBMFont::create("Choose a tab", "bigFont.fnt");
        m_tabHint->setScale(0.42f);
        m_tabHint->setPosition({center.x, 44.f});
        m_mainLayer->addChild(m_tabHint, 5);

        auto tabs = CCMenu::create();
        tabs->setPosition({center.x, 25.f});
        m_mainLayer->addChild(tabs, 6);
        const char* tabNames[] = {"PLAYER", "REPLAY", "STEPPER", "BYPASS", "INTERFACE"};
        for (int i = 0; i < 5; ++i) {
            auto label = CCLabelBMFont::create(tabNames[i], "bigFont.fnt");
            auto item = CCMenuItemLabel::create(label, this, menu_selector(UltimateMenuPopup::onTab));
            item->setTag(i);
            item->setScale(0.36f);
            item->setPosition({-142.f + i * 71.f, 0.f});
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
        m_playerOnlyControls->setEnabled(tab == 0);
        int row = 0;
        const float rowY[] = {96.f, 60.f, 24.f, -12.f, -48.f, -84.f};
        for (auto child : CCArrayExt<CCNode*>(m_itemsMenu->getChildren())) {
            auto item = typeinfo_cast<CCMenuItem*>(child);
            if (!item) continue;
            int tag = item->getTag();
            bool visible = (tab == 0 && tag >= 10 && tag <= 15) ||
                           (tab == 1 && tag >= 20 && tag <= 24) ||
                           (tab == 2 && tag >= 30 && tag <= 31) ||
                           (tab == 3 && tag == 40) ||
                           (tab == 4 && tag >= 50 && tag <= 53);
            item->setVisible(visible);
            item->setEnabled(visible);
            if (visible) {
                item->setPosition({0.f, rowY[std::min(row, 5)]});
                ++row;
            }
        }
        const char* hints[] = {
            "Player settings", "RE4: record, save, load and playback",
            "Toggle step mode, then tap STEP +1 FRAME",
            "TPS Bypass: fixed 240 Hz simulation ticks",
            "Cycle the floating button accent color"
        };
        m_tabHint->setString(hints[std::clamp(tab, 0, 4)]);
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
        if (m_recordToggle) m_recordToggle->setString(g_re4Recording ? "RE4 recording: ON" : "RE4 recording: OFF");
        if (m_stepperToggle) m_stepperToggle->setString(g_stepperEnabled ? "Frame Stepper: ON" : "Frame Stepper: OFF");
        if (m_replayPlay) m_replayPlay->setString(g_re4Playback ? "PLAYING .RE4" : "PLAY REPLAY");
        if (m_hideButton) m_hideButton->setString(g_hideInLevelButton ? "HIDE IN-LEVEL BUTTON: ON" : "HIDE IN-LEVEL BUTTON: OFF");
        if (m_tpsToggle) m_tpsToggle->setString(g_tpsBypassEnabled ? "TPS BYPASS 240: ON" : "TPS BYPASS 240: OFF");
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

    void onToggleRecording(CCObject*) {
        if (!g_re4Recording) {
            resetRe4Recorder();
            g_re4Recording = true;
            FLAlertLayer::create("RE4 Recorder", "Recording started. Play the level, then save the replay.", "OK")->show();
        } else {
            g_re4Recording = false;
            FLAlertLayer::create("RE4 Recorder", "Recording stopped. Press SAVE .RE4 to export.", "OK")->show();
        }
        this->refreshLabels();
    }

    void onSaveReplay(CCObject*) {
        if (g_re4Recording) {
            g_re4Recording = false;
            this->refreshLabels();
        }
        if (g_re4Inputs.empty()) {
            FLAlertLayer::create("RE4 Recorder", "No inputs recorded yet. Start recording and play first.", "OK")->show();
            return;
        }
        if (!saveRe4Replay()) {
            FLAlertLayer::create("RE4 export failed", "Could not write the replay file. Check the Geode log.", "OK")->show();
            return;
        }
        auto path = Mod::get()->getSaveDir() / "last-replay.re4";
        FLAlertLayer::create("RE4 saved", fmt::format("Saved {} inputs to:\n{}", g_re4Inputs.size(), path.string()), "OK")->show();
    }

    void onLoadReplay(CCObject*) {
        if (!loadRe4Replay()) {
            FLAlertLayer::create("RE4 import", "Could not load last-replay.re4 from the mod save folder. Check file format and try again.", "OK")->show();
            return;
        }
        FLAlertLayer::create("RE4 import", fmt::format("Loaded {} input events.", g_re4PlaybackInputs.size()), "OK")->show();
    }

    void onPlayReplay(CCObject*) {
        // Use the just-recorded macro directly; exporting a file is optional.
        if (!g_re4Inputs.empty()) {
            g_re4PlaybackInputs = g_re4Inputs;
        } else if (g_re4PlaybackInputs.empty() && !loadRe4Replay()) {
            FLAlertLayer::create("Replay", "Record a macro first or load last-replay.re4.", "OK")->show();
            return;
        }
        auto pl = PlayLayer::get();
        if (!pl) {
            FLAlertLayer::create("Replay", "Open a level before playback.", "OK")->show();
            return;
        }
        pl->resetLevel();
        g_re4Frame = 0;
        g_re4PlaybackCursor = 0;
        g_re4Playback = true;
        g_re4Recording = false;
        this->refreshLabels();
    }

    void onStopReplay(CCObject*) {
        g_re4Playback = false;
        this->refreshLabels();
    }

    void onToggleStepper(CCObject*) {
        g_stepperEnabled = !g_stepperEnabled;
        g_stepRequested = false;
        this->refreshLabels();
    }

    void onToggleTpsBypass(CCObject*) {
        g_tpsBypassEnabled = !g_tpsBypassEnabled;
        g_tpsAccumulator = 0.0;
        this->refreshLabels();
    }

    void onStepOnce(CCObject*) {
        if (g_stepperEnabled) g_stepRequested = true;
    }

    void onCycleColor(CCObject*) {
        g_buttonColorIndex = (g_buttonColorIndex + 1) % (sizeof(g_buttonColors) / sizeof(g_buttonColors[0]));
    }

    void onCycleOpacity(CCObject*) {
        g_buttonOpacityIndex = (g_buttonOpacityIndex + 1) % (sizeof(g_buttonOpacities) / sizeof(g_buttonOpacities[0]));
    }

    void onCycleSize(CCObject*) {
        g_buttonScaleIndex = (g_buttonScaleIndex + 1) % (sizeof(g_buttonScales) / sizeof(g_buttonScales[0]));
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
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        resetRe4Recorder();
        g_re4Recording = false;
        if (g_hideInLevelButton) return true;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        if (!menu) return true;
        menu->setID("gd-ultimate-game-menu"_spr);
        menu->setPosition({36.f, winSize.height - 36.f});
        menu->setZOrder(999);
        auto button = createUltimateButton(this, menu_selector(GDUltimatePlayLayer::onOpenUltimateMenu));
        if (!button) return true;
        button->setScale(0.72f);
        menu->addChild(button);
        this->addChild(menu, 999);
        return true;
    }
    void update(float dt) {
        if (g_stepperEnabled && !g_stepRequested) return;
        if (g_stepperEnabled) g_stepRequested = false;

        // Feed recorded input BEFORE the physics tick.
        if (g_re4Playback) {
            while (g_re4PlaybackCursor < g_re4PlaybackInputs.size() &&
                   g_re4PlaybackInputs[g_re4PlaybackCursor].frame <= g_re4Frame) {
                auto const& event = g_re4PlaybackInputs[g_re4PlaybackCursor++];
                this->handleButton(event.down != 0, event.button, event.player2 == 0);
            }
        }

        PlayLayer::update(dt);

        if (g_re4Playback) {
            ++g_re4Frame;
            if (g_re4PlaybackCursor >= g_re4PlaybackInputs.size()) g_re4Playback = false;
        }
        if (g_re4Recording) {
            ++g_re4Frame;
            auto recordPhysics = [this](PlayerObject* player, bool p2) {
                if (!player || player->m_isDead) return;
                g_re4Physics.push_back({g_re4Frame, player->getPositionX(), player->getPositionY(), 0.0, static_cast<uint8_t>(p2)});
            };
            recordPhysics(m_player1, false);
            recordPhysics(m_player2, true);
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

class $modify(GDUltimateBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (g_re4Recording && !g_re4Playback && static_cast<GJBaseGameLayer*>(PlayLayer::get()) == this) {
            g_re4Inputs.push_back({
                g_re4Frame,
                static_cast<uint8_t>(down),
                static_cast<int32_t>(button),
                static_cast<uint8_t>(!isPlayer1)
            });
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
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
