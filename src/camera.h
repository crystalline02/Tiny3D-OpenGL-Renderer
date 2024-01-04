#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class ViewMode{ FLY, SPIN };

class Camera
{
public:
    // Camera(int width = 1920, int height = 1080, const glm::vec3& position = glm::vec3(0.f, 2.f, 6.f), float yaw = -90.f, float pitch = 0.f, float fov = 45.f, float near = 0.1f, float far = 100.f, const glm::vec3& world_up = glm::vec3(0.f, 1.f, 0.f));
    Camera(int width = 1920, 
        int height = 1080, 
        const glm::vec3& interest = glm::vec3(0.f), 
        float distance = 4.f, 
        float dis_pitch = 45.f, 
        float dis_yaw = -90.f, 
        const glm::vec3& world_up = glm::vec3(0.f, 1.f, 0.f));
    inline glm::mat4 lookat() const { return glm::lookAt(m_position, m_position + m_front, m_world_up); }
    inline glm::mat4 projection() const { return glm::perspective(glm::radians((float)m_fov), m_aspect_ratio, m_near, m_far); }
    inline float aspect_ratio() const { return m_aspect_ratio; }
    inline glm::mat4 camera_mat() const { return glm::mat4(m_right.x, m_right.y, m_right.z, 0.f,
        m_up.x, m_up.y, m_up.z, 0.f,
        m_direction.x, m_direction.y, m_direction.z, 0.f,
        0.f, 0.f, 0.f, 1.f); }
    inline glm::vec3 right() const { return m_right; }
    inline glm::vec3 up() const { return m_up; }
    inline glm::vec3 world_up() const { return m_world_up; }
    inline glm::vec3 front() const { return m_front; }
    inline glm::vec3 position() const { return m_position; }
    inline float fov() const { return m_fov; }
    void reaction_middle(double delta_x, double delta_y, bool react = true);
    void reaction_scroll(float delta);
    void offset_positon(glm::vec3 offset);
    void reaction_D(float delta);
    void reaction_A(float delta);
    void reaction_W(float delta);
    void reaction_S(float delta);
    void reaction_E(float delta);
    void reaction_Q(float delta);
    void reaction_shift_middle(float delta_x, float delta_y, bool react = true);
    void switch_mode(ViewMode mode);
    inline int width() const { return m_width; }
    inline int height() const { return m_height; }
    inline void set_width(int width) { m_width = width; }
    inline void set_height(int height) { m_height = height; }
    inline float near() const { return m_near; }
    inline float far() const { return m_far; }
    inline ViewMode mode() const { return m_v_mode; }
private:
    glm::vec3 m_world_up, m_position, m_front, m_right, m_direction, m_up, m_interest;
    float m_aspect_ratio, m_near, m_far, m_speed, m_dis_interest;
    double m_yaw, m_pitch, m_fov;
    int m_width, m_height;
    double m_mouse_senstivity, m_scroll_senstivity;
    ViewMode m_v_mode;
private:
    inline void update_interest() { m_interest = m_position + m_front * m_dis_interest; }
    inline void update_positon() { m_position = m_interest + m_direction * m_dis_interest; }
    void update_aixes_on_angles();
    void update_angles_on_aixes();
};