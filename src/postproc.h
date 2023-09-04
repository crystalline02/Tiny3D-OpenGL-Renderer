#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Post_proc;
class Lighting_pass;
class Blur;
class Camera;

class Postproc_quad
{
public:
    static Postproc_quad* get_instance();
    void draw(const Post_proc &shader) const;
    void lighting_pass(const Lighting_pass &shader, const Camera &camera, GLuint fbo) const;
private:
    void run_blur(const Blur &shader, GLuint &blur_image_unit) const;
    static Postproc_quad* instance;
    Postproc_quad();
    GLuint quad_VAO, quad_VBO;
};