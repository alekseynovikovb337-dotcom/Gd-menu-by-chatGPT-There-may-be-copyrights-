#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

static void openUltimateMenu() {
    auto speed = Mod::get()->getSettingValue<double>("speed-multiplier");
    auto gravity = Mod::get()->getSettingValue<bool>("low-gravity");
    auto jump = Mod::get()->getSettingValue<bool>("jump-effect");
    auto death = Mod::get()->getSettingValue<bool>("death-effect");

    auto msg = fmt::format(
        "Speed: {:.2f}x\nLow gravity: {}\nJump effect: {}\nDeath effect: {}\n\n"
        "These options are currently saved as settings; gameplay effects are still being implemented.",
        speed,
        gravity ? "ON" : "OFF",
        jump ? "ON" : "OFF",
        death ? "ON" : "OFF"
    );
    FLAlertLayer::create("GD Ultimate Menu", msg, "OK")->show();
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

    void onOpenUltimateMenu(CCObject*) {
        openUltimateMenu();
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

    void onOpenUltimateMenu(CCObject*) {
        openUltimateMenu();
    }
};
