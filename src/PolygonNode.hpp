#pragma once
#include <clipper2/clipper.h>

class PolygonNode : public cocos2d::CCNode {
    ~PolygonNode();
public:
    static PolygonNode* create();
    bool init() override;

    // these should always stay in sync
    Clipper2Lib::PathsD m_tris;
    int m_vertexBufferCount;
    int m_outlineVertexBufferCount;
    GLuint m_vertexBuffer;
    GLuint m_outlineVertexBuffer;

    void draw() override;

    std::vector<float> flattenPathsD(const Clipper2Lib::PathsD& paths);
    void updateVertices(const std::vector<cocos2d::CCPoint>& points);
    bool intersectsNode(cocos2d::CCNode* node);
};
