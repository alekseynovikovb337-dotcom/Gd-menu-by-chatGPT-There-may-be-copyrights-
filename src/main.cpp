#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
using namespace geode::prelude;

class $modify(GDUltimateMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return true;
        auto spr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");
        if (!spr) return true;
        auto btn = CCMenuItemSpriteExtra::create(spr, this,
            menu_selector(GDUltimateMenuLayer::onOpenUltimateMenu));
        btn->setID("gd-ultimate-menu-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();
        return true;
    }
    void onOpenUltimateMenu(CCObject*) {
        auto speed = Mod::get()->getSettingValue<double>("speed-multiplier");
        auto gravity = Mod::get()->getSettingValue<bool>("low-gravity");
        auto jump = Mod::get()->getSettingValue<bool>("jump-effect");
        auto death = Mod::get()->getSettingValue<bool>("death-effect");
        auto msg = fmt::format("Starter menu loaded!\\nSpeed: {:.2f}x\\nLow gravity: {}\\nJump effect: {}\\nDeath effect: {}\\n\\nThese settings are not connected to gameplay yet.",
            speed, gravity ? "ON" : "OFF", jump ? "ON" : "OFF", death ? "ON" : "OFF");
        FLAlertLayer::create("GD Ultimate Menu", msg, "OK")->show();
    }
};
