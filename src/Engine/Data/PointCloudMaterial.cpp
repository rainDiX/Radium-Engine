#include "Engine/Data/Material.hpp"
#include <Engine/Data/PointCloudMaterial.hpp>

#include <Engine/Data/ShaderConfigFactory.hpp>
#include <Engine/Data/ShaderProgramManager.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Rendering/RenderTechnique.hpp>

namespace Ra {
namespace Engine {
namespace Data {
static const std::string materialName { "PointCloud" };

nlohmann::json PointCloudMaterial::s_parametersMetadata = {};

PointCloudMaterial::PointCloudMaterial( const std::string& instanceName ) :
    Material( instanceName, materialName, Material::MaterialAspect::MAT_OPAQUE ) {}

void PointCloudMaterial::registerMaterial() {
    // Get the Radium Resource location on the filesystem
    auto resourcesRootDir { RadiumEngine::getInstance()->getResourcesDir() };
    auto shaderProgramManager = RadiumEngine::getInstance()->getShaderProgramManager();

    shaderProgramManager->addNamedString(
        "/PointCloud.glsl", resourcesRootDir + "Shaders/Materials/PointCloud/PointCloud.glsl" );
    // registering re-usable shaders
    Data::ShaderConfiguration lpconfig(
        "PointCloud",
        resourcesRootDir + "Shaders/Materials/PointCloud/PointCloud.vert.glsl",
        resourcesRootDir + "Shaders/Materials/PointCloud/PointCloud.frag.glsl" );

    Data::ShaderConfigurationFactory::addConfiguration( lpconfig );

    Data::ShaderConfiguration zprepassconfig(
        "ZprepassPointCloud",
        resourcesRootDir + "Shaders/Materials/PointCloud/PointCloud.vert.glsl",
        resourcesRootDir + "Shaders/Materials/PointCloud/PointCloudZPrepass.frag.glsl" );
    Data::ShaderConfigurationFactory::addConfiguration( zprepassconfig );

    // Registering technique
    Rendering::EngineRenderTechniques::registerDefaultTechnique(
        materialName, []( Rendering::RenderTechnique& rt, bool ) {
            // Lighting pass
            auto lightpass = Data::ShaderConfigurationFactory::getConfiguration( "PointCloud" );
            rt.setConfiguration( *lightpass, Rendering::DefaultRenderingPasses::LIGHTING_OPAQUE );
            // Z prepass
            auto zprepass =
                Data::ShaderConfigurationFactory::getConfiguration( "ZprepassPointCloud" );
            rt.setConfiguration( *zprepass, Rendering::DefaultRenderingPasses::Z_PREPASS );
        } );

    // Registering parameters metadata
    // @todo
    // ParameterSetEditingInterface::loadMetaData( materialName, s_parametersMetadata );
}

void PointCloudMaterial::unregisterMaterial() {
    Rendering::EngineRenderTechniques::removeDefaultTechnique( materialName );
}

void PointCloudMaterial::updateRenderingParameters() {
    auto& renderParameters = getParameters();
    // update the rendering paramaters
    // @todo
    // renderParameters.addParameter( "material.color", m_color );
}

void PointCloudMaterial::updateGL() {
    if ( !m_isDirty ) { return; }

    m_isDirty = false;

    updateRenderingParameters();
}

void PointCloudMaterial::updateFromParameters() {
    auto& renderParameters = getParameters();
    // @todo
}

} // namespace Data
} // namespace Engine
} // namespace Ra