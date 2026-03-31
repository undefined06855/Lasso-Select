#pragma once
#include <Geode/modify/EditorUI.hpp>
#include "../PolygonNode.hpp"

class $modify(HookedEditorUI, EditorUI) {
    struct Fields {
        Fields();

        bool m_swiping;
        std::vector<cocos2d::CCPoint> m_points;
        cocos2d::CCPoint m_mousePos;
        PolygonNode* m_swipe;
    };

    bool init(LevelEditorLayer* editor);

    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event);
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event);
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event);
    void triggerSwipeMode();

    void swipeBegin();
    void swipeContinue();
    void swipeEnd();
    void addSwipePoint();
};
