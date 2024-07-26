#include "light.h"

#include "globals.h"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "material.h"

#include <string>
#include <typeinfo>

Light::Light(const glm::vec3& positon,
    const glm::vec3& color,
    float intensity, 
    float diffuse, 
    float specular,
    float ambient):
m_intensity(intensity), m_diffuse(diffuse), m_specular(specular), m_ambient(ambient), m_color(color), m_position(positon),
m_near(.1f), m_far(10.f), m_depth_fbo(-1), m_depth_texture()
{
    get_lights().emplace_back(this);
}

std::vector<Light*>& Light::get_lights()
{
    static std::vector<Light*> all_lights;  
    return all_lights;
}

unsigned int& Light::point_lights_count()
{
    static unsigned int point_lights_count = 0;
    return point_lights_count;
}

unsigned int& Light::spot_lights_count()
{
    static unsigned int spot_lights_count = 0;
    return spot_lights_count;
}

unsigned int& Light::direction_lights_count()
{
    static unsigned int direction_lights_count = 0;
    return direction_lights_count;
}

unsigned int Light::lights_count()
{
    return direction_lights_count() + spot_lights_count() + point_lights_count();
}

void Light::set_uniforms(const char* name, GLuint program) const
{
    std::string prefix = std::string(name) + std::string("[") + std::to_string(m_index) + std::string("].");
    std::string light_color = (prefix + std::string("color"));
    std::string light_intensity = (prefix + std::string("intensity"));
    std::string light_diffuse = (prefix + std::string("diffuse"));
    std::string light_specular = (prefix + std::string("specular"));
    std::string light_ambient = (prefix + std::string("ambient"));
    util::set_floats(light_color.c_str(), m_color, program);
    util::set_float(light_intensity.c_str(), m_intensity, program);
    util::set_float(light_diffuse.c_str(), m_diffuse, program);
    util::set_float(light_specular.c_str(), m_specular, program);
    util::set_float(light_ambient.c_str(), m_ambient, program);
}

Point_light::Point_light(const glm::vec3& positon,
    const glm::vec3& color,
    float intensity,
    float diffuse,
    float specular,
    bool flag):
Light(positon, color, intensity, diffuse, specular), kc(1.0f), kl(0.35f), kq(0.44f)
{
    if(!flag)
    {
        unsigned int& c = Light::point_lights_count();
        m_index = c++;
        util::create_depthcubemap_framebuffer(m_depth_fbo, m_depth_texture, 
            ("light_depth_map" + std::to_string(Light::lights_count())).c_str());
    }
}

void Point_light::set_uniforms(const char* name, GLuint program) const
{
    Light::set_uniforms(name, program);
    std::string prefix = std::string(name) + std::string("[") + std::to_string(m_index) + std::string("].");
    std::string light_pos = (prefix + std::string("position"));
    std::string light_kc = (prefix + std::string("kc"));
    std::string light_kl = (prefix + std::string("kl"));
    std::string light_kq = (prefix + std::string("kq"));
    std::string light_near = (prefix + std::string("near"));
    std::string light_far = (prefix + std::string("far"));
    std::string light_depthcubemap = (prefix + std::string("depth_cubemap"));
    util::set_floats(light_pos.c_str(), glm::vec3(m_position), program);
    util::set_float(light_kc.c_str(), kc, program);
    util::set_float(light_kl.c_str(), kl, program);
    util::set_float(light_kq.c_str(), kq, program);
    util::set_float(light_near.c_str(), m_near, program);
    util::set_float(light_far.c_str(), m_far, program);
    util::set_int(light_depthcubemap.c_str(), m_depth_texture.texUnit, program);
}

void Point_light::resize_depthmap(int shadow_size) const
{
    glActiveTexture(GL_TEXTURE0 + m_depth_texture.texUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_depth_texture.texBuffer);
    for(unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, util::Globals::cubemap_size, 
            util::Globals::cubemap_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glActiveTexture(GL_TEXTURE0);
}

std::array<glm::mat4, 6> Point_light::light_space_mat() const
{
    glm::mat4 prerpective = glm::perspective(glm::radians(90.f), 1.f, m_near, m_far);
    std::array<glm::mat4, 6> ret;
    ret[0] = prerpective * glm::lookAt(m_position, m_position + glm::vec3(1.f, 0.f, 0.f), {0.f, -1.f, 0.f});
    ret[1] = prerpective * glm::lookAt(m_position, m_position + glm::vec3(-1.f, 0.f, 0.f), {0.f, -1.f, 0.f});
    ret[2] = prerpective * glm::lookAt(m_position, m_position + glm::vec3(0.f, 1.f, 0.f), {0.f, 0.f, 1.f});
    ret[3] = prerpective * glm::lookAt(m_position, m_position + glm::vec3(0.f, -1.f, 0.f), {0.f, 0.f, -1.f});
    ret[4] = prerpective * glm::lookAt(m_position, m_position + glm::vec3(0.f, 0.f, 1.f), {0.f, -1.f, 0.f});
    ret[5] = prerpective * glm::lookAt(m_position, m_position + glm::vec3(0.f, 0.f, -1.f), {0.f, -1.f, 0.f});
    return ret;
}

Direction_light::Direction_light(const glm::vec3& direction,
    const glm::vec3& positon,
    const glm::vec3& color,
    float intensity, 
    float diffuse, 
    float specular):
Light(positon, color, intensity, diffuse, specular), m_direction(glm::normalize(direction))
{
    unsigned int& c = Light::direction_lights_count();
    m_index = c++;
    // Bind depth map, create depth framebuffer
    util::create_cascademap_framebuffer(m_depth_fbo, m_depth_texture, 
        ("light_depth_map" + std::to_string(Light::lights_count())).c_str());
}

void Direction_light::set_uniforms(const char* name, GLuint program) const
{
    Light::set_uniforms(name, program);
    std::string prefix = std::string(name) + std::string("[") + std::to_string(m_index) + std::string("].");
    std::string light_direction = (prefix + std::string("direction"));
    std::string light_cascade_maps = (prefix + std::string("cascade_maps"));
    util::set_floats(light_direction.c_str(), m_direction, program);
    util::set_int(light_cascade_maps.c_str(), m_depth_texture.texUnit, program);
}

std::vector<glm::mat4> Direction_light::light_space_mat(const Camera& camera) const
{
    std::vector<glm::mat4> ret;
    for(int i = 0; i < util::Globals::cascade_levels.size() + 1; ++i)
    {
        std::array<glm::vec3, 8> corners;
        if(i == 0)
            corners = util::get_frustum_corner_world(camera, camera.near(), util::Globals::cascade_levels[i]);
        else if(i == util::Globals::cascade_levels.size())
            corners = util::get_frustum_corner_world(camera, util::Globals::cascade_levels[i - 1], camera.far());
        else
            corners = util::get_frustum_corner_world(camera, util::Globals::cascade_levels[i - 1], util::Globals::cascade_levels[i]);
        // for(int i = 0; i < 8; ++i) std::cout << corners[i].x << "," << corners[i].y << "," << corners[i].z << "\t";
        glm::vec3 center(0.f);
        for(glm::vec3& v: corners)
            center += v;
        center /= 8.f;
        glm::mat4 light_view = glm::lookAt(center - m_direction, center, camera.world_up());

        float max_x = std::numeric_limits<float>::lowest();
        float min_x = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::lowest();
        float min_y = std::numeric_limits<float>::max();
        float max_z = std::numeric_limits<float>::lowest();
        float min_z = std::numeric_limits<float>::max();
        for(glm::vec3& p : corners)
        {
            glm::vec3 p_light = glm::vec3(light_view * glm::vec4(p, 1.f));
            max_x = std::max(max_x, p_light.x);
            min_x = std::min(min_x, p_light.x);
            max_y = std::max(max_y, p_light.y);
            min_y = std::min(min_y, p_light.y);
            max_z = std::max(max_z, p_light.z);
            min_z = std::min(min_z, p_light.z);
        }
        float z_offset = 50.f / (max_z - min_z) ;
        max_z += z_offset;
        min_z -= z_offset;
        
        glm::mat4 ortho = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);
        ret.push_back(ortho * light_view);
    }
    return ret;
}

void Direction_light::resize_depthmap(int shadow_size) const
{
    glActiveTexture(GL_TEXTURE0 + m_depth_texture.texUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_depth_texture.texBuffer);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, util::Globals::cascade_size, util::Globals::cascade_size,
        util::Globals::cascade_levels.size() + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glActiveTexture(GL_TEXTURE0);
}

Spot_light::Spot_light(const glm::vec3& direction,
    const glm::vec3& position,
    const glm::vec3& color,
    float intensity, 
    float diffuse, 
    float specular):
Point_light(position, color, intensity, diffuse, specular, true),
m_dir(glm::normalize(direction)), 
m_innner(glm::radians(18.f)),
m_outer(glm::radians(35.5f))
{
    unsigned int& c = Light::spot_lights_count();
    m_index = c++;
    util::create_depthcubemap_framebuffer(m_depth_fbo, m_depth_texture, 
        ("light_depth_map" + std::to_string(Light::lights_count())).c_str());
}

void Spot_light::set_uniforms(const char* name, GLuint program) const
{
    Point_light::set_uniforms(name, program);
    std::string prefix = std::string(name) + std::string("[") + std::to_string(m_index) + std::string("].");
    std::string light_direction = prefix + std::string("direction");
    std::string light_outer_cutoff = prefix + std::string("outer_cutoff");
    std::string light_cutoff = prefix + std::string("cutoff");
    util::set_floats(light_direction.c_str(), glm::vec3(m_dir), program);
    util::set_float(light_cutoff.c_str(), glm::cos(m_innner), program);
    util::set_float(light_outer_cutoff.c_str(), glm::cos(m_outer), program);
}

Light_vertices::Light_vertices()
{
    m_vertices_circle = new float[108] {
		0.1500f, 0.0000f, 0.f,
		0.1477f, 0.0260f, 0.f,
		0.1410f, 0.0513f, 0.f,
		0.1299f, 0.0750f, 0.f,
		0.1149f, 0.0964f, 0.f,
		0.0964f, 0.1149f, 0.f,
		0.0750f, 0.1299f, 0.f,
		0.0513f, 0.1410f, 0.f,
		0.0260f, 0.1477f, 0.f,
		0.0000f, 0.1500f, 0.f,
		-0.0260f, 0.1477f, 0.f,
		-0.0513f, 0.1410f, 0.f,
		-0.0750f, 0.1299f, 0.f,
		-0.0964f, 0.1149f, 0.f,
		-0.1149f, 0.0964f, 0.f,
		-0.1299f, 0.0750f, 0.f,
		-0.1410f, 0.0513f, 0.f,
		-0.1477f, 0.0260f, 0.f,
		-0.1500f, 0.0000f, 0.f,
		-0.1477f, -0.0260f, 0.f,
		-0.1410f, -0.0513f, 0.f,
		-0.1299f, -0.0750f, 0.f,
		-0.1149f, -0.0964f, 0.f,
		-0.0964f, -0.1149f, 0.f,
		-0.0750f, -0.1299f, 0.f,
		-0.0513f, -0.1410f, 0.f,
		-0.0260f, -0.1477f, 0.f,
		-0.0000f, -0.1500f, 0.f,
		0.0260f, -0.1477f, 0.f,
		0.0513f, -0.1410f, 0.f,
		0.0750f, -0.1299f, 0.f,
		0.0964f, -0.1149f, 0.f,
		0.1149f, -0.0964f, 0.f,
		0.1299f, -0.0750f, 0.f,
		0.1410f, -0.0513f, 0.f,
		0.1477f, -0.0260f, 0.f,
    };
    m_size_vertices_circle = 108 * sizeof(float);
    m_verticies_bar = new float[6] { 0.f, 0.0f, 0.f, 0.f, -5.f, 0.f};
    m_size_verticies_bar = 6 * sizeof(float);
    m_vertices_radiant = new float[48] {
        0.0750f, 0.0000f, 0.f,
        0.3000f, 0.0000f, 0.f,
        0.0530f, 0.0530f, 0.f,
        0.2121f, 0.2121f, 0.f,
        0.0000f, 0.0750f, 0.f,
        0.0000f, 0.3000f, 0.f,
        -0.0530f, 0.0530f, 0.f,
        -0.2121f, 0.2121f, 0.f,
        -0.0750f, 0.0000f, 0.f,
        -0.3000f, 0.0000f, 0.f,
        -0.0530f, -0.0530f, 0.f,
        -0.2121f, -0.2121f, 0.f,
        -0.0000f, -0.0750f, 0.f,
        -0.0000f, -0.3000f, 0.f,
        0.0530f, -0.0530f, 0.f,
        0.2121f, -0.2121f, 0.f
    };
    m_size_vertices_radiant = 48 * sizeof(float);

    glGenVertexArrays(1, &m_circle_VAO);
    glGenVertexArrays(1, &m_bar_VAO);
    glGenVertexArrays(1, &m_radiant_VAO);
    m_circle_VBO = util::create_vbo_2_vao(m_vertices_circle, m_size_vertices_circle);
    m_bar_VBO = util::create_vbo_2_vao(m_verticies_bar, m_size_verticies_bar);
    m_radiant_VBO = util::create_vbo_2_vao(m_vertices_radiant, m_size_vertices_radiant);

    // VAO for cirlce
    bind_vbo_2_vao(m_circle_VAO, m_circle_VBO);
    // VAO for bar
    bind_vbo_2_vao(m_bar_VAO, m_bar_VBO);
    // VAO for radiant
    bind_vbo_2_vao(m_radiant_VAO, m_radiant_VBO);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Light_vertices::~Light_vertices()
{
    delete[] m_vertices_circle;
    delete[] m_verticies_bar;
    delete[] m_vertices_radiant;
    glDeleteBuffers(1, &m_circle_VBO);
    glDeleteBuffers(1, &m_bar_VBO);
    glDeleteBuffers(1, &m_radiant_VBO);
}

void Light_vertices::bind_vbo_2_vao(GLint VAO, GLint VBO) const
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

Light_vertices* Light_vertices::get_instance()
{
    if(!singleton) return singleton = new Light_vertices();
    else return singleton;
}

glm::mat4 Light_vertices::get_bar_dir_matrix(glm::vec3 direction, glm::vec3 postion)
{
    glm::vec3 mat_x = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), -direction));
    glm::vec3 mat_z = glm::normalize(glm::cross(mat_x, -direction));
    glm::mat4 model_bar_dir(glm::vec4(mat_x, 0.f), 
        glm::vec4(-direction, 0.f), 
        glm::vec4(mat_z, 0.f), 
        glm::vec4(postion, 1.f));
    return model_bar_dir;
}

void Light_vertices::draw(const Shader::Shader& shader, const Camera& camera)
{
    assert(shader.shaderName() == "./shader/singleColor");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    shader.use();
    shader.setSingleColor(glm::vec3(0.f));
    const std::vector<Light*>& all_lights = Light::get_lights();
    for(Light* light: all_lights)
    {
        glm::mat4 model_bar(1.f);
        model_bar = glm::translate(model_bar, light->position());
        switch (light->type())
        {
        case Light_type::POINT:
        {
            // Draw circle
            glm::mat4 model_circle = util::face_camera_model(util::Globals::camera.camera_mat(), light->position());
            glBindVertexArray(m_circle_VAO);
            glEnableVertexAttribArray(0);
            shader.set_model(model_circle); 
            glDrawArrays(GL_LINE_LOOP, 0, m_size_vertices_circle / (3 * sizeof(float)));
            break;
        }
        case Light_type::SUN:
        {
            // Draw circle
            glm::mat4 model_circle = util::face_camera_model(util::Globals::camera.camera_mat(), light->position());
            float zdis_l2c = glm::dot(light->position() - util::Globals::camera.position(), util::Globals::camera.front()); 
            float factor = 0.07f;
            model_circle = glm::scale(model_circle, glm::vec3(factor * zdis_l2c, factor * zdis_l2c, factor * zdis_l2c));
            glBindVertexArray(m_circle_VAO);
            glEnableVertexAttribArray(0);
            shader.set_model(model_circle);
            glDrawArrays(GL_LINE_LOOP, 0, m_size_vertices_circle / (3 * sizeof(float)));

            // Draw direction bar
            const Direction_light* light_d = static_cast<const Direction_light*>(light);  // 父类指针强转为子类指针
            glm::mat4 model_bar_dir = get_bar_dir_matrix(light_d->direction(), light_d->position());
            glBindVertexArray(m_bar_VAO);
            glEnableVertexAttribArray(0);
            shader.set_model(model_bar_dir);
            glDrawArrays(GL_LINE_LOOP, 0, m_size_vertices_circle / (3 * sizeof(float)));

            // Draw radiant
            glm::mat4 model_radiant(model_circle);
            glBindVertexArray(m_radiant_VAO);
            glEnableVertexAttribArray(0);
            shader.set_model(model_radiant);
            for(int i = 0; i < m_size_vertices_radiant / (3 * sizeof(float)); i += 2)
                glDrawArrays(GL_LINE_STRIP, i, 2);
            break;
        }
        case Light_type::SPOT:
        {
            // Draw Circle
            glm::mat4 model_circle = util::face_camera_model(util::Globals::camera.camera_mat(), light->position());
            float zdis_l2c = glm::dot(light->position() - util::Globals::camera.position(), util::Globals::camera.front());
            float factor = 0.07f;
            model_circle = glm::scale(model_circle, glm::vec3(factor * zdis_l2c));
            glBindVertexArray(m_circle_VAO);
            glEnableVertexAttribArray(0);
            shader.set_model(model_circle);
            glDrawArrays(GL_LINE_LOOP, 0, m_size_vertices_circle / (3 * sizeof(float)));
            
            // Draw direction bar
            const Spot_light* light_s = static_cast<const Spot_light*>(light);  // 父类指针强转为子类指针
            glm::mat4 model_bar_dir = get_bar_dir_matrix(light_s->direction(), light_s->position());
            glm::mat4 model_bar_dir_scaled = glm::scale(model_bar_dir, glm::vec3(0.5f, 0.5f, 0.5f));
            glBindVertexArray(m_bar_VAO);
            glEnableVertexAttribArray(0);
            shader.set_model(model_bar_dir_scaled);
            glDrawArrays(GL_LINE_LOOP, 0, m_size_vertices_circle / (3 * sizeof(float)));
            
            // Draw outer clone bars
            glm::vec3 clone_bar_dirs_outer[4] = {glm::vec3(-glm::sin(light_s->outer_cutoff()), -glm::cos(light_s->outer_cutoff()), 0.f),
                glm::vec3(glm::sin(light_s->outer_cutoff()), -glm::cos(light_s->outer_cutoff()), 0.f),
                glm::vec3(0.f, -glm::cos(light_s->outer_cutoff()), glm::sin(light_s->outer_cutoff())),
                glm::vec3(0.f, -glm::cos(light_s->outer_cutoff()), -glm::sin(light_s->outer_cutoff()))};
            for(int i = 0; i < 4; ++i)
            {
                glm::mat4 model_clone_bar = model_bar_dir * get_bar_dir_matrix(clone_bar_dirs_outer[i], glm::vec3(0.f));
                glBindVertexArray(m_bar_VAO);
                glEnableVertexAttribArray(0);
                shader.set_model(model_clone_bar);
                glDrawArrays(GL_LINE_STRIP, 0, m_size_verticies_bar / (3 * sizeof(float)));
            }

            // Draw 2 clone cirlce
            glm::mat4 model_clone_circles[2] = {glm::mat4(1.f), glm::mat4(1.f)};
            factor = 5.f * glm::sin(light_s->outer_cutoff()) / 0.15f;
            model_clone_circles[0] = model_bar_dir * glm::scale(glm::rotate(glm::translate(model_clone_circles[0], 
                        glm::vec3(0.f, -5 * glm::cos(light_s->outer_cutoff()), 0.f)),
                    glm::radians(90.f),
                    glm::vec3(1.f, 0.f, 0.f)),
                glm::vec3(factor));
            factor = 5.f * glm::cos(light_s->outer_cutoff()) * glm::tan(light_s->cutoff()) / 0.15f;
            model_clone_circles[1] = model_bar_dir * glm::scale(glm::rotate(glm::translate(model_clone_circles[1], 
                        glm::vec3(0.f, -5 * glm::cos(light_s->outer_cutoff()), 0.f)),
                    glm::radians(90.f),
                    glm::vec3(1.f, 0.f, 0.f)),
                glm::vec3(factor));
            for(int i = 0; i < 2; ++i)
            {
                glBindVertexArray(m_circle_VAO);
                glEnableVertexAttribArray(0);
                shader.set_model(model_clone_circles[i]);
                glDrawArrays(GL_LINE_LOOP, 0, m_size_vertices_circle / (3 * sizeof(float)));
            }
            break;
        }
        default:
            break;
        }

        // Draw bar
        glBindVertexArray(m_bar_VAO);
        glEnableVertexAttribArray(0);
        shader.set_model(model_bar);
        glDrawArrays(GL_LINE_STRIP, 0, m_size_verticies_bar);
    }
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}

Light_vertices* Light_vertices::singleton = nullptr;