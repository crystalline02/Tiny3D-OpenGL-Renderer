#include "camera.h"

// Camera::Camera(int width, int height, const glm::vec3& positon, float yaw, float pitch, float fov, float near, float far, const glm::vec3& world_up):
// m_width(width), m_height(height), m_position(positon), m_yaw(yaw), m_pitch(pitch), m_fov(fov), m_near(near), m_far(far), 
// m_v_mode(ViewMode::FLY), m_aspect_ratio((float)width / height), , 
// , m_dis_interest(3.f), m_world_up(glm::normalize(world_up))
// {
//     update_aixes_on_angles();
//     m_interest = m_position + m_front * m_dis_interest;
// }

Camera::Camera(int width, 
    int height, 
    const glm::vec3& interest, 
    float distance, 
    float dis_pitch, 
    float dis_yaw, 
    const glm::vec3& world_up):
m_width(width), m_height(height), m_aspect_ratio((float)width / height), m_world_up(world_up), m_near(0.1f), m_far(20.0f),
m_speed(0.0035f), m_dis_interest(distance), m_interest(interest), m_fov(45.f), m_pitch(-dis_pitch), m_yaw(dis_yaw), m_mouse_senstivity(0.16f),
m_scroll_senstivity(1.8f), m_v_mode(ViewMode::SPIN)
{
    // 注意C++中写的初始化列表，变量的初始化顺序是与变量在.h中定义的顺序一致的，而不是与初始化列表写出的初始化顺序一致
    update_aixes_on_angles();
    update_positon();
}

void Camera::reaction_middle(double delta_x, double delta_y, bool react)
{
    switch(m_v_mode)
    {
    case ViewMode::FLY:
    {
        m_yaw += delta_x * m_mouse_senstivity;
        m_pitch += delta_y * m_mouse_senstivity;
        if(m_pitch > 89.9f) m_pitch = 89.9f;
        if(m_pitch < -89.9f) m_pitch = -89.9f;
        // Update axies at each movement of the camera may not be a optimal approch, but I have no other ideal
        update_aixes_on_angles();
        update_interest();
        break;
    }
    case ViewMode::SPIN:
    {
        if(!react) return;
        m_pitch += delta_y * m_mouse_senstivity;
        m_yaw += delta_x * m_mouse_senstivity;
        if(m_pitch > 89.9f) m_pitch = 89.9f;
        if(m_pitch < -89.9f) m_pitch = -89.9f;
        update_aixes_on_angles();
        update_positon();
        break;
    }
    default:
        break;
    }
}

void Camera::reaction_shift_middle(float delta_x, float delta_y, bool react)
{
    switch(m_v_mode)
    {
    case ViewMode::SPIN:
    {
        if(!react) return;
        m_position += m_right * delta_x * m_speed;
        m_position += m_up * delta_y * m_speed;
        update_interest();
        break;
    }
    default:
        break;
    }
}

void Camera::update_aixes_on_angles()
{
    m_front = glm::vec3(glm::cos(glm::radians(m_pitch)) * glm::cos(glm::radians(m_yaw)), 
        glm::sin(glm::radians(m_pitch)), 
        glm::cos(glm::radians(m_pitch)) * glm::sin(glm::radians(m_yaw)));
    m_direction = -m_front;
    m_right = glm::normalize(glm::cross(m_world_up, m_direction));
    m_up = glm::normalize(glm::cross(m_direction, m_right));
}

void Camera::update_angles_on_aixes()
{
    m_yaw = glm::acos(m_front.x / glm::sqrt(m_front.x * m_front.x + m_front.z * m_front.z)),
    m_pitch = glm::acos(glm::sqrt(m_front.x * m_front.x + m_front.z * m_front.z) / 1.f);
}

void Camera::offset_positon(glm::vec3 offset)
{
    m_position += offset * m_speed;
    update_interest();
}

void Camera::reaction_D(float delta)
{
    switch(m_v_mode)
    {
        case ViewMode::FLY:
        {
            m_position += m_right * delta * m_speed;
            update_interest();
            break;
        }
        default:
            break;
    }
}

void Camera::reaction_A(float delta)
{
    switch(m_v_mode)
    {
        case ViewMode::FLY:
        {
            m_position -= m_right * delta * m_speed;
            update_interest();
            break;
        }
        default:
            break;
    }
}

void Camera::reaction_W(float delta)
{
    switch(m_v_mode)
    {
        case ViewMode::FLY:
        {
            m_position += m_front * delta * m_speed;
            update_interest();
            break;
        }
        default:
            break;
    }
}

void Camera::reaction_S(float delta)
{
    switch(m_v_mode)
    {
        case ViewMode::FLY:
        {
            m_position -= m_front * delta * m_speed;
            update_interest();
            break;
        }
        default:
            break;
    }
}

void Camera::reaction_E(float delta)
{
    switch(m_v_mode)
    {
        case ViewMode::FLY:
        {
            m_position += m_world_up * delta * m_speed;
            update_interest();
            break;
        }
        default:
            break;
    }
}

void Camera::reaction_Q(float delta)
{
    switch(m_v_mode)
    {
        case ViewMode::FLY:
        {
            m_position -= m_world_up * delta * m_speed;
            update_interest();
            break;
        }
        default:
            break;
    }
}

void Camera::reaction_scroll(float delta)
{
    switch(m_v_mode)
    {
        case ViewMode::FLY:
        {
            m_fov += delta * m_scroll_senstivity;
            if(m_fov > 100.f) m_fov = 100.f;
            if(m_fov < 10.f) m_fov = 10.f;
            break;
        }
        case ViewMode::SPIN:
        {
            m_dis_interest += delta * m_scroll_senstivity * glm::sin(m_dis_interest * 3.141592 / 150.f);
            if(m_dis_interest < .02f) m_dis_interest = .02f;
            if(m_dis_interest > 150.f) m_dis_interest = 150.f;
            update_positon();
            break;
        }
        default:
            break;
    }
}

void Camera::switch_mode(ViewMode mode)
{
    m_v_mode = mode;
    switch (m_v_mode)
    {
        case ViewMode::FLY:   
        {
            m_speed = 2.5f;
            m_mouse_senstivity = 0.07f;
            m_scroll_senstivity = 1.f;
            break;
        }
        case ViewMode::SPIN:
        {
            m_speed = 0.0035f;
            m_mouse_senstivity = 0.16f;
            m_scroll_senstivity = 1.8f;
            break;
        }
        default:
            break;
    }
}
