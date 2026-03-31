#include "PolygonNode.hpp"
#include "intersect.hpp"

PolygonNode::~PolygonNode() {
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_outlineVertexBuffer);
}

PolygonNode* PolygonNode::create() {
    auto ret = new PolygonNode;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool PolygonNode::init() {
    if (!CCNode::init()) return false;

    glGenBuffers(1, &m_vertexBuffer);
    glGenBuffers(1, &m_outlineVertexBuffer);

    m_vertexBufferCount = 0;
    m_outlineVertexBufferCount = 0;

    this->setShaderProgram(cocos2d::CCShaderCache::sharedShaderCache()->programForKey("lasso_shader"_spr));
    this->setAnchorPoint({ .5f, .5f });

    return true;
}

void PolygonNode::draw() {
    CC_NODE_DRAW_SETUP();

    cocos2d::ccGLEnableVertexAttribs(cocos2d::kCCVertexAttribFlag_Position);

    cocos2d::ccGLBlendFunc(CC_BLEND_SRC, CC_BLEND_DST);

    int isOutline = m_pShaderProgram->getUniformLocationForName("u_isOutline");

    // TODO: GL_DOUBLE is great and all but will it work on ios :broken_heart:

    m_pShaderProgram->setUniformLocationWith1i(isOutline, 1);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineVertexBuffer);
    glVertexAttribPointer(cocos2d::kCCVertexAttrib_Position, 2, GL_DOUBLE, GL_FALSE, 0, (void*)(0));
    glDrawArrays(GL_TRIANGLES, 0, m_outlineVertexBufferCount);

    m_pShaderProgram->setUniformLocationWith1i(isOutline, 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(cocos2d::kCCVertexAttrib_Position, 2, GL_DOUBLE, GL_FALSE, 0, (void*)(0));
    glDrawArrays(GL_TRIANGLES, 0, m_vertexBufferCount);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // *getNumberOfDraws() += 1;
}

std::vector<double> PolygonNode::flattenPathsD(const Clipper2Lib::PathsD& paths) {
    std::vector<double> solution;
    solution.reserve(paths.size() * 6);
    for (auto& tri : paths) {
        solution.push_back(tri[0].x);
        solution.push_back(tri[0].y);
        solution.push_back(tri[1].x);
        solution.push_back(tri[1].y);
        solution.push_back(tri[2].x);
        solution.push_back(tri[2].y);
    }

    return solution;
}

void PolygonNode::updateVertices(const std::vector<cocos2d::CCPoint>& points) {
    Clipper2Lib::PathD path;
    path.reserve(points.size());
    for (auto& point : points) { path.push_back({ point.x, point.y }); }

    Clipper2Lib::PathsD source = { path };
    Clipper2Lib::Triangulate(Clipper2Lib::Union(source, Clipper2Lib::FillRule::NonZero), /* decimal places */ 3, m_tris);

    // flatten PathsD into double vector then put into vertex buffer
    auto vertexCoords = this->flattenPathsD(m_tris);
    m_vertexBufferCount = vertexCoords.size() / 2;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.size() * sizeof(double), vertexCoords.data(), GL_DYNAMIC_DRAW);

    // there are 34 bugs to do with this path inflation code. google inflation r34
    auto inflated = Clipper2Lib::InflatePaths(m_tris, 10.0, Clipper2Lib::JoinType::Round, Clipper2Lib::EndType::Round);
    auto outlineCoords = this->flattenPathsD(inflated);
    m_outlineVertexBufferCount = outlineCoords.size() / 2;
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, outlineCoords.size() * sizeof(double), outlineCoords.data(), GL_DYNAMIC_DRAW);
}

bool PolygonNode::intersectsNode(cocos2d::CCNode* node) {
    auto bounding = node->boundingBox();
    auto parent = node->getParent();
    if (!parent) return false;

    auto topLeft = parent->convertToWorldSpace({ bounding.getMinX(), bounding.getMaxY() });
    auto topRight = parent->convertToWorldSpace({ bounding.getMaxX(), bounding.getMaxY() });
    auto bottomLeft = parent->convertToWorldSpace({ bounding.getMinX(), bounding.getMinY() });
    auto bottomRight = parent->convertToWorldSpace({ bounding.getMaxX(), bounding.getMinY() });

    // TODO: add initial aabb test? this is already so fast anyway....

    for (auto& tri : m_tris) {
        // have to convert from double back to float :(
        auto corner1 = cocos2d::CCPoint{ (float)tri[0].x, (float)tri[0].y };
        auto corner2 = cocos2d::CCPoint{ (float)tri[1].x, (float)tri[1].y };
        auto corner3 = cocos2d::CCPoint{ (float)tri[2].x, (float)tri[2].y };
        if (lasso::math::quadTriIntersect(topLeft, topRight, bottomLeft, bottomRight, corner1, corner2, corner3)) {
            return true;
        }
    }

    return false;
}
