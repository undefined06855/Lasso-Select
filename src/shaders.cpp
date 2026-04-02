#include "shaders.hpp"

const GLchar* g_lassoShaderVertex = R"(
attribute vec4 a_position;

void main() {
    gl_Position = CC_MVPMatrix * a_position;
}
)";

const GLchar* g_lassoShaderFragment = R"(
#ifdef GL_ES
precision lowp float;
#endif

uniform bool u_isOutline;

void main() {
    if (u_isOutline) {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        gl_FragColor = vec4(0.17, 0.77, 0.28, 0.7);
    }
}
)";

$on_game(TexturesLoaded) {
    auto program = new cocos2d::CCGLProgram;
    bool ret = program->initWithVertexShaderByteArray(g_lassoShaderVertex, g_lassoShaderFragment);
    if (!ret) {
        geode::log::warn("shader failed to load!!!");
        geode::log::warn("{}", program->fragmentShaderLog());
        return;
    }

    program->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);

    program->link();
    program->updateUniforms();

    geode::log::info("heyyyyy hi hi hi it's the mod im doing the lasso yeah hi");

    cocos2d::CCShaderCache::sharedShaderCache()->addProgram(program, "lasso_shader"_spr);
}
