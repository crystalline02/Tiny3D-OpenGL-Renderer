#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Post_proc;
class Blur;

class Postproc_quad
{
public:
    static Postproc_quad* get_instance();
    void draw(const Post_proc& shader);
private:
    void run_blur(const Blur &shader, GLuint &blur_image_unit);
    static Postproc_quad* instance;
    Postproc_quad();
    GLuint quad_VAO, quad_VBO;
};