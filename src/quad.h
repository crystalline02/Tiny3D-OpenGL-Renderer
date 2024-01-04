#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Shader
{
    class Post_proc;
    class Lighting_pass;
    class Bloom_blur;
    class SSAO;
    class SSAO_blur;
}
class Camera;

class Postproc_quad
{
public:
    static Postproc_quad* get_instance();
    void draw(const Shader::Post_proc &shader) const;
    void lighting_pass(const Shader::Lighting_pass &shader, const Camera &camera, GLuint fbo) const;
    void ssao_pass(const Shader::SSAO &shader, const Shader::SSAO_blur &blur_shader, GLuint ssao_fbo, GLuint ssao_blur_fbo) const;
private:
    void run_blur(const Shader::Bloom_blur &shader, GLuint &blur_image_unit) const;
    static Postproc_quad* instance;
    Postproc_quad();
    GLuint quad_VAO, quad_VBO;
};