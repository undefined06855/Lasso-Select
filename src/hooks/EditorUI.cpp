#include "EditorUI.hpp"

HookedEditorUI::Fields::Fields()
    : m_swiping(false)
    , m_points()
    , m_mousePos(cocos2d::CCPoint{ 0.f, 0.f })
    , m_swipe(nullptr)

    , m_useLasso(geode::Mod::get()->getSavedValue<bool>("use-lasso", true))
    , m_alt(nullptr) {}

void HookedEditorUI::onModify(auto self) {
    (void)self.setHookPriorityPost("EditorUI::init", geode::Priority::Late);
}

bool HookedEditorUI::init(LevelEditorLayer* editor) {
    if (!EditorUI::init(editor)) return false;

    auto fields = m_fields.self();

    auto onSprite = geode::BasedButtonSprite::create(
        cocos2d::CCSprite::create("lasso.png"_spr),
        geode::BaseType::Editor,
        (int)geode::EditorBaseSize::Normal,
        (int)geode::EditorBaseColor::Cyan
    );
    onSprite->setScale(.75f);

    fields->m_alt = cocos2d::CCSprite::create("alt.png"_spr);
    fields->m_alt->setScale(.6f);
    fields->m_alt->setPosition({ 22.f, 9.f });
    fields->m_alt->setVisible(false);
    onSprite->addChild(fields->m_alt);

    auto offSprite = geode::BasedButtonSprite::create(
        cocos2d::CCSprite::create("lasso.png"_spr),
        geode::BaseType::Editor,
        (int)geode::EditorBaseSize::Normal,
        (int)geode::EditorBaseColor::Green
    );
    offSprite->setScale(.75f);

    auto toggler = geode::cocos::CCMenuItemExt::createToggler(
        onSprite, offSprite,
        [this](CCMenuItemToggler* toggler) {
            bool isLasso = !toggler->isOn(); // it's inverted because the callback gets called at a weird time
            m_fields->m_useLasso = isLasso;
            geode::Mod::get()->setSavedValue<bool>("use-lasso", isLasso);
        }
    );
    toggler->setID("lasso-button-toggler"_spr);

    toggler->toggle(fields->m_useLasso);

    auto pad = cocos2d::CCMenu::create();
    pad->setID("lasso-button-menu"_spr);
    pad->setContentSize({ 40.f, 40.f });
    pad->addChildAtPosition(toggler, geode::Anchor::Center, { 0.f, -2.f });

    auto buttonsMenu = this->getChildByID("editor-buttons-menu");
    if (!buttonsMenu) return true;

    buttonsMenu->addChild(pad);
    buttonsMenu->updateLayout();

    m_uiItems->addObject(pad);

    auto node = PolygonNode::create();
    node->setID("PolygonNode"_spr);
    this->addChild(node);

    fields->m_swipe = node;

    this->addEventListener(geode::KeyboardInputEvent(), [this, toggler](geode::KeyboardInputData data) {
        if (data.key != cocos2d::enumKeyCodes::KEY_LeftMenu) return geode::ListenerResult::Propagate;

        auto fields = m_fields.self();

        // alt pressed
        if (data.action == geode::KeyboardInputData::Action::Press && !fields->m_useLasso) {
            fields->m_useLasso = true;
            fields->m_alt->setVisible(true);
            toggler->toggle(true);
            return geode::ListenerResult::Stop;
        }

        // alt released
        if (data.action == geode::KeyboardInputData::Action::Release && fields->m_alt->isVisible()) {
            fields->m_alt->setVisible(false);
            fields->m_useLasso = false;
            toggler->toggle(false);
            return geode::ListenerResult::Stop;
        }

        return geode::ListenerResult::Propagate;
    });

    return true;
}

bool HookedEditorUI::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    if (!EditorUI::ccTouchBegan(touch, event)) return false;
    auto fields = m_fields.self();
    fields->m_mousePos = touch->getLocation();

    // m_snapObjectExists just is whether you're free moving or not?
    if (m_swipeActive && !m_snapObjectExists && fields->m_useLasso) {
        m_swipeActive = false; // always stop gd swiping
        this->swipeBegin();
    }

    return true;
}

// called when hold to swipe gets triggered
void HookedEditorUI::triggerSwipeMode() {
    EditorUI::triggerSwipeMode();

    if (m_swipeActive && !m_snapObjectExists && m_fields->m_useLasso) {
        m_swipeActive = false;
        this->swipeBegin();
    }
}

void HookedEditorUI::ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    auto fields = m_fields.self();
    fields->m_mousePos = touch->getLocation();

    if (!fields->m_swiping) {
        EditorUI::ccTouchMoved(touch, event);
        return;
    }

    this->stopActionByTag(123); // hold to swipe action number
    this->swipeContinue();
}

void HookedEditorUI::ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    this->stopActionByTag(123);

    EditorUI::ccTouchEnded(touch, event); // hope this doesnt break much

    auto fields = m_fields.self();
    fields->m_mousePos = touch->getLocation();

    if (fields->m_swiping) {
        this->swipeEnd();
    }
}

void HookedEditorUI::swipeBegin() {
    auto fields = m_fields.self();
    fields->m_swiping = true;
    this->addSwipePoint();
}

void HookedEditorUI::swipeContinue() {
    auto fields = m_fields.self();
    this->addSwipePoint();
}

void HookedEditorUI::swipeEnd() {
    auto fields = m_fields.self();
    fields->m_swiping = false;
    this->addSwipePoint();

    if (fields->m_points.size() < 3) {
        // not enough points
        return;
    }

    // do the shit
    auto objects = cocos2d::CCArray::create();
    for (int i = 0; i < m_editorLayer->m_activeObjectsCount; i++) {
        auto obj = m_editorLayer->m_activeObjects[i];
        if (fields->m_swipe->intersectsNode(obj)) {
            objects->addObject(obj);
        }
    }

    this->selectObjects(objects, false);
    this->updateEditMenu();
    this->updateButtons();
    this->updateObjectInfoLabel();

    fields->m_points.clear();
    fields->m_swipe->updateVertices(fields->m_points);
}

void HookedEditorUI::addSwipePoint() {
    auto fields = m_fields.self();

    if (fields->m_points.size() != 0 && fields->m_points.back().getDistanceSq(fields->m_mousePos) < 5.f) {
        // too short of a distance
        return;
    }

    if (std::find(fields->m_points.begin(), fields->m_points.end(), fields->m_mousePos) != fields->m_points.end()) {
        // point already exists, this can break triangulation
        geode::log::warn("duplicate point at {}", fields->m_mousePos);
        return;
    }

    fields->m_points.push_back(fields->m_mousePos);
    fields->m_swipe->updateVertices(fields->m_points);
}
