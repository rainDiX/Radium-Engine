#ifndef RADIUMENGINE_DIRLIGHT_HPP
#define RADIUMENGINE_DIRLIGHT_HPP

#include <Engine/Renderer/Light/Light.hpp>

namespace Ra { namespace Engine { class ShaderProgram; } }

namespace Ra { namespace Engine {

class RA_API DirectionalLight : public Light
{
public:
    RA_CORE_ALIGNED_NEW
    DirectionalLight();
    virtual ~DirectionalLight();

    virtual void bind(ShaderProgram* shader);

    inline void setDirection(const Core::Vector3& pos);
    inline const Core::Vector3& getDirection() const;

private:
    Core::Vector3 m_direction;
};

} // namespace Engine
} // namespace Ra

#include <Engine/Renderer/Light/DirLight.inl>

#endif // RADIUMENGINE_POINTLIGHT_HPP
