#ifndef RADIUMENGINE_DRAWABLE_HPP
#define RADIUMENGINE_DRAWABLE_HPP

#include <string>
#include <mutex>
#include <memory>

#include <Core/Index/IndexedObject.hpp>
#include <Core/Math/Vector.hpp>
#include <Engine/Renderer/Material/Material.hpp>

namespace Ra { namespace Engine { class Light;         } }
namespace Ra { namespace Engine { class ShaderProgram; } }
namespace Ra { namespace Engine { class Component;     } }

namespace Ra { namespace Engine {

class RA_API Drawable : public Core::IndexedObject
{
public:
    // FIXME(Charly): Set component in the constructor ?
	explicit Drawable(const std::string& name);
    virtual ~Drawable();

    virtual void setComponent(Component* component) final;

	virtual const std::string& getName() const final;

    virtual void updateGL() final;

    virtual void draw(const Core::Matrix4& view,
                      const Core::Matrix4& proj,
                      ShaderProgram* shader) = 0;
    virtual void draw(const Core::Matrix4& view,
                      const Core::Matrix4& proj,
                      Light* light) = 0;

	virtual Drawable* clone() final;

    virtual void setVisible(bool visible) final;
    virtual bool isVisible() const final;

    virtual bool isDirty() const final;

	virtual void setMaterial(Material* material);
	virtual void setMaterial(std::shared_ptr<Material> material);
    virtual Material* getMaterial() const;

	/**
	 * @brief Get the bounding box of the drawable. 
	 * How the bbox is computed only depends on the reimplentation of the drawable.
	 * Hence, it is all the developer's responsibility to have it updated properly.
	 *
	 * @return The drawable bbox
	 */
	inline const Core::Aabb& getBoundingBox() const;
	Core::Aabb getBoundingBoxInWorld() const;

protected:
	virtual void updateGLInternal() = 0;
	virtual Drawable* cloneInternal() = 0;

protected:
    std::string m_name;
    Component* m_component;

	mutable std::mutex m_updateMutex;

    bool m_visible;
    mutable std::mutex m_visibleMutex;

    std::shared_ptr<Material> m_material;

    bool m_isDirty;

	Core::Aabb m_boundingBox;
};

} // namespace Engine
} // namespace Ra

#include <Engine/Renderer/Drawable/Drawable.inl>

#endif // RADIUMENGINE_DRAWABLE_HPP
