#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Camera;
class Material;
class Light;

namespace Shader
{
    class Shader
    {
    public:
        Shader(std::string dir_path);
        inline GLuint program() const { return programId; }
        inline void use() const { glUseProgram(programId); }
        inline std::string shaderName() const { return m_dir; }
        static void update_uniform_blocks(const Camera& camera);
        void setSingleColor(const glm::vec3& color) const;
        void set_model(const glm::mat4& model) const;
        void set_viewpos(const glm::vec3& view_pos) const;
        void set_globals() const;
        void set_normal_mat(const glm::mat4& model) const;
        void set_material(const char* name, const Material& material) const;
    protected:
        std::string m_dir;
        static GLuint ubo_matrices, ubo_fn, ubo_light_matrices;
        GLuint programId;
    };

    class ObjectShader: public Shader
    {
    public:
        ObjectShader(std::string dir_path);
        virtual void set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const = 0;
    };

    class BlinnPhong: public ObjectShader
    {
    public:
        BlinnPhong();
        void set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const override;
    };

    class PBR: public ObjectShader
    {
    public:
        PBR();
        void set_uniforms(const Material& material, const Camera& camera, const glm::mat4& model) const override;
    };

    class SingleColor: public Shader
    {
    public:
        SingleColor();
        void set_uniforms(const glm::mat4& model, const glm::vec3& color) const;
    };

    class Normal: public Shader
    {
    public:
        Normal();
        void set_uniforms(const Camera& camera, const glm::mat4& model) const;
    };

    class TangentNormal: public Shader
    {
    public:
        TangentNormal();
        void set_uniforms(const Camera& camera, const glm::mat4& model) const;
    };

    class DepthShader: public Shader
    {
    public:
        DepthShader(std::string dir_path);
        virtual void set_uniforms(const glm::mat4& model, const Light& light) const = 0;
    };

    class CascadeMap: public DepthShader
    {
    public:
        CascadeMap();
        void set_uniforms(const glm::mat4& model, const Light& light) const override;
    };

    class DepthCubemap: public DepthShader
    {
    public:
        DepthCubemap();
        void set_uniforms(const glm::mat4& model, const Light& light) const override;
    };

    class SkyCube: public Shader
    {
    public:
        SkyCube();
        void set_uniforms(const Camera& camera, float intensity, GLuint texture_unit) const;
    };

    class BloomBlur: public Shader
    {
    public:
        void set_uniforms(GLuint image_unit, bool horizental) const;
        static BloomBlur* get_instance();
    private:
        BloomBlur();
        static BloomBlur* instance;
    };

    class SSAOBlur: public Shader
    {
    public:
        static SSAOBlur* get_instance();
        void set_uniforms() const;
    private:
        SSAOBlur();
        static SSAOBlur* instance;
    };

    class PostProc: public Shader
    {
    public:
        void set_uniforms(GLuint image_unit, GLuint blured_image_unit) const;
        static PostProc* get_instance();
    private:
        PostProc();
        static PostProc* instance;
    };

    class GBuffer: public Shader
    {
    public:
        void set_uniforms(const Material &material, const Camera &camera, const glm::mat4 &model) const;
    protected:
        GBuffer(std::string dir_path);
    };

    class GBufferBP: public GBuffer
    {
    public:
        static GBufferBP* get_instance();
        GBufferBP();
    private:
        static GBufferBP* instance;
    };

    class GBufferPBR: public GBuffer
    {
    public:
        static GBufferPBR* get_instance();
    private:
        GBufferPBR();
        static GBufferPBR* instance;
    };

    class TransparentWBPBR: public Shader
    {
    public:
        static TransparentWBPBR* getInstance();
        void setUniforms(const Material& material, const Camera& camera, const glm::mat4& model) const;
    private:
        TransparentWBPBR();
        static TransparentWBPBR* instance;
    };

    class Composite: public Shader
    {
    public:
        static Composite* getInstance();
        void setUniforms(GLuint accumTexUnit, GLuint revealageTexUnit) const;
    private:
        Composite();
        static Composite* instance;
    };

    class SSAO: public Shader
    {
    public:
        void set_uniforms() const;
        static SSAO* get_instance();
    private:
        SSAO();
        static SSAO* instance;
    };

    class LightingPass: public Shader
    {
    public:
        void set_uniforms(const Camera &camera) const;
    protected:
        LightingPass(std::string dir_path);
    };

    class LightingPassBP: public LightingPass
    {
    public:
        static LightingPassBP* get_instance();
    private:
        static LightingPassBP* instance;
        LightingPassBP();
    };

    class LightingPassPBR: public LightingPass
    {
    public:
        static LightingPassPBR* get_instance();
    private:
        static LightingPassPBR* instance;
        LightingPassPBR();
    };

    class HDRI2cubemap: public Shader
    {
    public:
        void set_uniforms(GLuint HDRI_map_unit, const glm::mat4& view) const;
        static HDRI2cubemap* get_instance();
    private:
        HDRI2cubemap();
        static HDRI2cubemap* instance;
    };

    class Cubemap2Irradiance: public Shader
    {
    public:
        void set_uniforms(const glm::mat4& view, GLuint cubemap_unit) const;
        static Cubemap2Irradiance* get_instance();
    private:
        Cubemap2Irradiance();
        static Cubemap2Irradiance* instance;
    };

    class CubemapPrefilter: public Shader
    {
    public:
        void set_uniforms(const glm::mat4& view, GLuint cubemap_unit, float roughness) const;
        static CubemapPrefilter* get_instance();
    private:
        CubemapPrefilter();
        static CubemapPrefilter* instance;
    };

    class CubemapBRDFIntergral: public Shader
    {
    public:
        void set_uniforms() const;
        static CubemapBRDFIntergral* getInstance();
    private:
        CubemapBRDFIntergral();
        static CubemapBRDFIntergral* instance;
    };

    class FBODebuger: public Shader
    {
    public:
        void set_uniforms(GLuint tex_unit, const glm::mat4& mode, bool alphal) const;
        static FBODebuger* getInstance();
    private:
        FBODebuger();
        static FBODebuger* instance;
    };

    class TextShader: public Shader
    {
    public:
        void set_uniforms(const glm::mat4& projection, const glm::vec3& color, GLuint bitmap_unit) const;
        static TextShader* get_instance();
    private:
        TextShader();
        static TextShader* instance;
    };
}