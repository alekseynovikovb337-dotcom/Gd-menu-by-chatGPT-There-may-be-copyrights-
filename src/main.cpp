#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

class UltimateMenuPopup : public geode::Popup {
protected:
    TextInput* m_speedInput = nullptr;
    CCMenuItemLabel* m_speedToggle = nullptr;
    CCMenuItemLabel* m_gravityToggle = nullptr;
    CCMenuItemLabel* m_jumpToggle = nullptr;
    CCMenuItemLabel* m_deathToggle = nullptr;
    CCMenuItemLabel* m_noclipToggle = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;

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
        menu->setPosition({175.f, 195.f});
        content->addChild(menu);

        auto makeLabel = [](std::string const& text) {
            return CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        };

        m_speedToggle = CCMenuItemLabel::create(makeLabel("Speedhack: OFF"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_speedToggle->setTag(0);
        m_speedToggle->setScale(0.55f);
        m_speedToggle->setPosition({0.f, 155.f});
        menu->addChild(m_speedToggle);

        m_gravityToggle = CCMenuItemLabel::create(makeLabel("Low gravity: OFF"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_gravityToggle->setTag(1);
        m_gravityToggle->setScale(0.55f);
        m_gravityToggle->setPosition({0.f, 115.f});
        menu->addChild(m_gravityToggle);

        m_jumpToggle = CCMenuItemLabel::create(makeLabel("Jump effect: ON"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_jumpToggle->setTag(2);
        m_jumpToggle->setScale(0.55f);
        m_jumpToggle->setPosition({0.f, 75.f});
        menu->addChild(m_jumpToggle);

        m_deathToggle = CCMenuItemLabel::create(makeLabel("Death effect: ON"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_deathToggle->setTag(3);
        m_deathToggle->setScale(0.55f);
        m_deathToggle->setPosition({0.f, 35.f});
        menu->addChild(m_deathToggle);

        m_noclipToggle = CCMenuItemLabel::create(makeLabel("NoClip: OFF"), this,
            menu_selector(UltimateMenuPopup::onToggle));
        m_noclipToggle->setTag(4);
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
        apply->setScale(0.55f);
        apply->setPosition({0.f, -45.f});
        menu->addChild(apply);

        this->refreshLabels();
        return true;
    }

    void refreshLabels() {
        m_speedToggle->setString(Mod::get()->getSettingValue<bool>("speedhack-enabled") ? "Speedhack: ON" : "Speedhack: OFF");
        m_gravityToggle->setString(Mod::get()->getSettingValue<bool>("low-gravity") ? "Low gravity: ON" : "Low gravity: OFF");
        m_jumpToggle->setString(Mod::get()->getSettingValue<bool>("jump-effect") ? "Jump effect: ON" : "Jump effect: OFF");
        m_deathToggle->setString(Mod::get()->getSettingValue<bool>("death-effect") ? "Death effect: ON" : "Death effect: OFF");
        m_noclipToggle->setString(Mod::get()->getSettingValue<bool>("noclip-enabled") ? "NoClip: ON" : "NoClip: OFF");
    }

    void onToggle(CCObject* sender) {
        switch (sender->getTag()) {
            case 0: Mod::get()->setSettingValue("speedhack-enabled", !Mod::get()->getSettingValue<bool>("speedhack-enabled")); break;
            case 1: Mod::get()->setSettingValue("low-gravity", !Mod::get()->getSettingValue<bool>("low-gravity")); break;
            case 2: Mod::get()->setSettingValue("jump-effect", !Mod::get()->getSettingValue<bool>("jump-effect")); break;
            case 3: Mod::get()->setSettingValue("death-effect", !Mod::get()->getSettingValue<bool>("death-effect")); break;
            case 4: Mod::get()->setSettingValue("noclip-enabled", !Mod::get()->getSettingValue<bool>("noclip-enabled")); break;
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

class $modify(GDUltimatePlayerObject, PlayerObject) {
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
