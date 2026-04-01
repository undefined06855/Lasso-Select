#include "EditorUI.hpp"

HookedEditorUI::Fields::Fields()
    : m_swiping(false)
    , m_points() {}

    // TODO: shift to use normal selection
    // TODO: (and) mobile ui

bool HookedEditorUI::init(LevelEditorLayer* editor) {
    if (!EditorUI::init(editor)) return false;

    auto node = PolygonNode::create();
    node->setID("PolygonNode"_spr);
    this->addChild(node);

    m_fields->m_swipe = node;

    return true;
}

bool HookedEditorUI::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    if (!EditorUI::ccTouchBegan(touch, event)) return false;
    m_fields->m_mousePos = touch->getLocation();

    // TODO: the object you're hovering over seems to get selected?

    if (m_swipeActive) {
        this->swipeBegin();
    }

    m_swipeActive = false; // always stop gd swiping

    return true;
}

// called when hold to swipe gets triggered
void HookedEditorUI::triggerSwipeMode() {
    EditorUI::triggerSwipeMode();

    if (m_swipeActive) {
        this->swipeBegin();
    }

    m_swipeActive = false;
    m_swipeModeTriggered = false;
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

    fields->m_points.push_back(fields->m_mousePos);
    fields->m_swipe->updateVertices(fields->m_points);
}
