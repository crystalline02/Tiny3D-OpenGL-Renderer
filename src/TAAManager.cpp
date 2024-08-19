#include "TAAManager.h"

#include "camera.h"
#include "fboManager.h"

#include <algorithm>

FBOManager::FBOData TAAManager::historyFBO = {};

const std::array<glm::vec2, 16> TAAManager::HaltonSeq = 
{
    glm::vec2(0.500000f, 0.333333f),
    glm::vec2(0.250000f, 0.666667f),
    glm::vec2(0.750000f, 0.111111f),
    glm::vec2(0.125000f, 0.444444f),
    glm::vec2(0.625000f, 0.777778f),
    glm::vec2(0.375000f, 0.222222f),
    glm::vec2(0.875000f, 0.555556f),
    glm::vec2(0.062500f, 0.888889f),
    glm::vec2(0.562500f, 0.037037f),
    glm::vec2(0.312500f, 0.370370f),
    glm::vec2(0.812500f, 0.703704f),
    glm::vec2(0.187500f, 0.148148f),
    glm::vec2(0.687500f, 0.481481f),
    glm::vec2(0.437500f, 0.814815f),
    glm::vec2(0.937500f, 0.259259f),
    glm::vec2(0.031250f, 0.592593f)
};

std::array<glm::vec2, 16> TAAManager::jitterVec = 
{
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f),
    glm::vec2(0.f, 0.f)
};

void TAAManager::matchJitterVec(const Camera& camera)
{
    for(uint32_t i = 0; i < HaltonSeq.size(); ++i)
        jitterVec[i] = ((HaltonSeq[i] - glm::vec2(0.5f, 0.5f)) * 2.f) / glm::vec2(camera.width(), camera.height());
}   