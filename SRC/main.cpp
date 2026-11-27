#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(GDUltimateMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto bottomMenu = this->getChildByID("bottom-menu");
        if (!bottomMenu) {
            log::warn("GD Ultimate Menu: bottom-menu node was not found");
            return true;
        }

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        if (!sprite) sprite = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");
        if (!sprite) {
            log::warn("GD Ultimate Menu: couldn't create button sprite");
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(GDUltimateMenuLayer::onOpenUltimateMenu)
        );
        button->setID("gd-ultimate-menu-button"_spr);
        bottomMenu->addChild(button);
        bottomMenu->updateLayout();
        return true;
    }

    void onOpenUltimateMenu(CCObject*) {
        auto speed = Mod::get()->getSettingValue<double>("speed-multiplier");
        auto lowGravity = Mod::get()->getSettingValue<bool>("low-gravity");
        auto jumpEffect = Mod::get()->getSettingValue<bool>("jump-effect");
        auto deathEffect = Mod::get()->getSettingValue<bool>("death-effect");

        auto message = fmt::format(
            "<cg>GD Ultimate Menu</c> <cy>v0.1.0</c>\n\n"
            "Starter menu is loaded!\n\n"
            "Saved settings:\n"
            "Speed: <cy>{:.2f}x</c>\n"
            "Low gravity: <cy>{}</c>\n"
            "Jump effect: <cy>{}</c>\n"
            "Death effect: <cy>{}</c>\n\n"
            "Change these values in Geode > Mods > GD Ultimate Menu.\n"
            "Gameplay effects are not connected in this starter build yet.",
            speed,
            lowGravity ? "ON" : "OFF",
            jumpEffect ? "ON" : "OFF",
            deathEffect ? "ON" : "OFF"
        );
        FLAlertLayer::create("GD Ultimate Menu", message, "OK")->show();
    }
};

