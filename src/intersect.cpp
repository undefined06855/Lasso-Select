#include "intersect.hpp"

// cross-product of lines ab and ac (see what side of line ab line ac is on or vice versa)
float cross(cocos2d::CCPoint a, cocos2d::CCPoint b, cocos2d::CCPoint c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// see if p is inside by seeing if the cross products are either all negative or all positive
// so if the point is either to the left of all the lines or to the right of all the lines
// idk what kind of winding order clipper2d gives me so you can just shove whatever in
// sebastian lague's software rasteriser video has a good explanation for this
bool pointInTriangle(cocos2d::CCPoint p, cocos2d::CCPoint a, cocos2d::CCPoint b, cocos2d::CCPoint c) {
    float d1 = cross(p, a, b);
    float d2 = cross(p, b, c);
    float d3 = cross(p, c, a);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

// uhh im not sure how this works go ask your parents
bool lineIntersect(cocos2d::CCPoint a1, cocos2d::CCPoint a2, cocos2d::CCPoint b1, cocos2d::CCPoint b2) {
    float d1 = cross(a1, a2, b1);
    float d2 = cross(a1, a2, b2);
    float d3 = cross(b1, b2, a1);
    float d4 = cross(b1, b2, a2);

    return ((d1 * d2) <= 0.0f) && ((d3 * d4) <= 0.0f);
}

// do all the shit
bool triTriIntersect(cocos2d::CCPoint a1, cocos2d::CCPoint a2, cocos2d::CCPoint a3, cocos2d::CCPoint b1, cocos2d::CCPoint b2, cocos2d::CCPoint b3) {
    if (pointInTriangle(a1, b1, b2, b3)) return true;
    if (pointInTriangle(b1, a1, a2, a3)) return true;

    if (lineIntersect(a1, a2, b1, b2)) return true;
    if (lineIntersect(a1, a2, b2, b3)) return true;
    if (lineIntersect(a1, a2, b3, b1)) return true;

    if (lineIntersect(a2, a3, b1, b2)) return true;
    if (lineIntersect(a2, a3, b2, b3)) return true;
    if (lineIntersect(a2, a3, b3, b1)) return true;

    if (lineIntersect(a3, a1, b1, b2)) return true;
    if (lineIntersect(a3, a1, b2, b3)) return true;
    if (lineIntersect(a3, a1, b3, b1)) return true;

    return false;
}

bool lasso::math::quadTriIntersect(cocos2d::CCPoint q0, cocos2d::CCPoint q1, cocos2d::CCPoint q2, cocos2d::CCPoint q3, cocos2d::CCPoint t0, cocos2d::CCPoint t1, cocos2d::CCPoint t2) {
    if (triTriIntersect(q0, q1, q2, t0, t1, t2)) return true;
    if (triTriIntersect(q0, q2, q3, t0, t1, t2)) return true;

    return false;
}
