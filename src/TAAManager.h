# pragma once
#include <array>
#include <glm/glm.hpp>

class Camera;
namespace FBOManager
{
    class FBOData;
}

class TAAManager 
{
public:
    static FBOManager::FBOData historyFBO;
    static const std::array<glm::vec2, 16> HaltonSeq;
    static std::array<glm::vec2, 16> jitterVec;
    
    static glm::mat4 modelPrev, viewPrev, projectionPrev;

    static void matchJitterVec(const Camera& camera); 
};
// jitter vector used in clip space