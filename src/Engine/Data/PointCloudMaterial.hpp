#pragma once

#include "Engine/Data/Mesh.hpp"
#include <Engine/RaEngine.hpp>

#include <map>
#include <string>

#include <Core/Utils/Color.hpp>
#include <Engine/Data/Material.hpp>

namespace Ra {
namespace Core {
namespace Asset {
class MaterialData;
}
} // namespace Core

namespace Engine {

namespace Data {
class ShaderProgram;

/**
 * WIP
 */
class RA_ENGINE_API PointCloudMaterial final : public Material, public ParameterSetEditingInterface
{
  public:
    /**
     * Construct a named @todo
     * @param instanceName The name of this instance of the material
     */
    explicit PointCloudMaterial( const std::string& instanceName );

    void updateGL() override;
    void updateFromParameters() override;

    /**
     * Register the material in the material library.
     * After registration, the material could be instantiated by any Radium system, renderer,
     * plugin, ...
     */
    static void registerMaterial();

    /**
     * Remove the material from the material library.
     * After removal, the material is no more available, ...
     */
    static void unregisterMaterial();

    /**
     * Get a json containing metadata about the parameters of the material.
     * @return the metadata in json format
     */
    inline nlohmann::json getParametersMetadata() const override;

    inline void setColoredByVertexAttrib( bool state ) override;

    inline bool isColoredByVertexAttrib() const override;

    /**
     * Update the rendering parameters for the Material
     */
    void updateRenderingParameters();

  public:
    Core::Utils::Color m_kd { 0.7, 0.7, 0.7, 1.0 };
    Core::Utils::Color m_ks { 0.3, 0.3, 0.3, 1.0 };
    Scalar m_ns { 64.0 };
    Scalar m_alpha { 1.0 };
    bool m_perVertexColor { false };
    bool m_renderAsSplat { false };

  private:
    static nlohmann::json s_parametersMetadata;
};

inline nlohmann::json PointCloudMaterial::getParametersMetadata() const {
    return s_parametersMetadata;
}

inline void PointCloudMaterial::setColoredByVertexAttrib( bool state ) {
    bool oldState    = m_perVertexColor;
    m_perVertexColor = state;
    if ( oldState != m_perVertexColor ) { needUpdate(); }
}

inline bool PointCloudMaterial::isColoredByVertexAttrib() const {
    return m_perVertexColor;
}

} // namespace Data
} // namespace Engine
} // namespace Ra
