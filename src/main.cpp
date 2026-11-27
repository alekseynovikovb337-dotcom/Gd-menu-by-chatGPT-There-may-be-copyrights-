#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <vector>


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

    void resetRe4Recorder() {
        g_re4Inputs.clear();
        g_re4Physics.clear();
        g_re4Frame = 0;
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
        content->setContentSize({350.f, 390.f});
        auto menu = CCMenu::create();
        m_itemsMenu = menu;
        menu->setPosition({175.f, 195.f});
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

        m_speedInput = TextInput::create(130.f, "Speed multiplier");
        m_speedInput->setCommonFilter(CommonFilter::Float);
        m_speedInput->setString(fmt::format("{:.2f}", Mod::get()->getSettingValue<double>("speed-multiplier")));
        m_speedInput->setPosition({center.x, 35.f});
        m_speedInput->setScale(0.8f);
        content->addChild(m_speedInput);

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
        bool hasItems = (tab == 0 || tab == 1);
        m_itemsMenu->setVisible(hasItems);
        m_speedInput->setVisible(tab == 0);
        for (auto child : CCArrayExt<CCNode*>(m_itemsMenu->getChildren())) {
            auto item = typeinfo_cast<CCMenuItem*>(child);
            if (!item) continue;
            int tag = item->getTag();
            bool visible = (tab == 0 && tag >= 10 && tag <= 15) ||
                           (tab == 1 && tag >= 20 && tag <= 21);
            item->setVisible(visible);
            item->setEnabled(visible);
        }
        const char* hints[] = {
            "Player settings", "Replay Bot: record and export .RE4",
            "Frame Stepper controls are being implemented",
            "Bypass controls are being implemented",
            "Interface: movable button and colors are being implemented"
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
        button->setScale(0.72f);
        menu->addChild(button);
        this->addChild(menu, 10000);
    }
    void onOpenUltimateMenu(CCObject*) { openUltimateMenu(); }
};

class $modify(GDUltimatePlayerObject, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        bool result = PlayerObject::pushButton(btn);
        if (g_re4Recording) {
            auto pl = PlayLayer::get();
            if (pl && (this == pl->m_player1 || this == pl->m_player2)) {
                g_re4Inputs.push_back({g_re4Frame, 1, static_cast<int32_t>(btn), static_cast<uint8_t>(this == pl->m_player2)});
            }
        }
        return result;
    }

    bool releaseButton(PlayerButton btn) {
        bool result = PlayerObject::releaseButton(btn);
        if (g_re4Recording) {
            auto pl = PlayLayer::get();
            if (pl && (this == pl->m_player1 || this == pl->m_player2)) {
                g_re4Inputs.push_back({g_re4Frame, 0, static_cast<int32_t>(btn), static_cast<uint8_t>(this == pl->m_player2)});
            }
        }
        return result;
    }

    void update(float dt) {
        this->m_hasGroundParticles = Mod::get()->getSettingValue<bool>("jump-effect");
        this->m_hasShipParticles = Mod::get()->getSettingValue<bool>("jump-effect");
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
        PlayLayer::update(dt);
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

class $modify(GDUltimateSpeedhackScheduler, cocos2d::CCScheduler) {
    void update(float dt) {
        if (PlayLayer::get() && Mod::get()->getSettingValue<bool>("speedhack-enabled")) {
            auto speed = static_cast<float>(Mod::get()->getSettingValue<double>("speed-multiplier"));
            dt *= std::clamp(speed, 0.5f, 3.0f);
        }
        CCScheduler::update(dt);
    }
};
