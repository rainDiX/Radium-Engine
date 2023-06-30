// Include Radium base application and its simple Gui
#include "Core/Containers/VectorArray.hpp"
#include "Core/Types.hpp"
#include <Core/Asset/FileLoaderInterface.hpp>
#include <Engine/Data/DrawPrimitives.hpp>
#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Rendering/RenderObjectManager.hpp>
#include <Engine/Scene/Component.hpp>
#include <Engine/Scene/SystemDisplay.hpp>
#include <Gui/BaseApplication.hpp>
#include <Gui/RadiumWindow/SimpleWindowFactory.hpp>
#include <Gui/Utils/KeyMappingManager.hpp>
#include <Gui/Viewer/Viewer.hpp>

// include the Engine/entity/component interface
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <Engine/Scene/GeometrySystem.hpp>

// colors
#include <Core/Utils/Color.hpp>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QSettings>
#include <QTimer>
#include <iostream>
#include <memory>
#include <string>

void addBoundingBox( Ra::Core::VectorArray<Ra::Core::Aabb>& bbs,
                     const Ra::Core::Vector3Array& vertices,
                     int start,
                     unsigned short size ) {
    Ra::Core::Aabb aabb;
    for ( int i = 0; i < size; ++i ) {
        const auto& v = vertices[start + i];
        aabb.extend( v );
    }
    bbs.emplace_back( aabb );
}
Ra::Core::VectorArray<Ra::Core::Aabb>
computeKdTreeLeafAabb( const Ra::Core::Vector3Array& vertices,
                       const Ra::Core::VectorArray<Ra::Core::KdTreeNode>& nodes,
                       int leaf_count ) {
    auto bbs = Ra::Core::VectorArray<Ra::Core::Aabb>();

    bbs.reserve( leaf_count );

    for ( const auto& node : nodes ) {
        if ( node.is_leaf() ) { addBoundingBox( bbs, vertices, node.leaf.start, node.leaf.size ); }
    }
    return bbs;
}

// void addBoundingBoxForNode() {
//     Ra::Core::Aabb aabb;
//     // while () {
//     // aabb.extend(  );
//     // }
// }

// void r( const KdTreeNode& node,
//         int currentDepth,
//         int depth,
//         const VectorArray<KdTreeNode>& node_data,
//         VectorArray<Aabb>& bbs ) {
//     if ( currentDepth < depth ) {

//         if ( !node.is_leaf() ) {
//             r( node_data[node.inner.first_child_id], currentDepth + 1, depth, node_data, bbs );
//             // r(node_data[node.inner.], currentDepth + 1, depth, node_data);//TODO
//         }
//     }
// }

// Ra::Core::VectorArray<Ra::Core::Aabb> computeKdTreeAabb( int depth ) {
//     const auto& node_data = m_kdTree.node_data();
//     const auto& root      = node_data[0];
//     auto bbs              = Ra::Core::VectorArray<Ra::Core::Aabb>();

//     // bbs.reserve( m_kdTree->node_count() ); // to change
//     r( root, 0, depth, node_data, bbs );
//     return bbs;
// }

class DemoWindow : public Ra::Gui::SimpleWindow
{
  public:
    DemoWindow() : Ra::Gui::SimpleWindow() {

        // Build the demo interface (menus ...)
        auto menuBar        = new QMenuBar( this );
        auto fileMenu       = menuBar->addMenu( "&File" );
        auto fileOpenAction = new QAction( "&Open..." );
        fileOpenAction->setShortcuts( QKeySequence::Open );
        fileOpenAction->setStatusTip( "Open a file." );
        fileMenu->addAction( fileOpenAction );

        // Connect the menu
        auto openFile = [this]() {
            QString filter;
            QString allexts;
            auto engine = Ra::Engine::RadiumEngine::getInstance();
            for ( const auto& loader : engine->getFileLoaders() ) {
                QString exts;
                if ( loader->name() == "TinyPly" ) {
                    for ( const auto& e : loader->getFileExtensions() ) {
                        exts.append( QString::fromStdString( e ) + " " );
                    }
                    allexts.append( exts + " " );
                    filter.append( QString::fromStdString( loader->name() ) + " (" + exts + ");;" );
                }
            }
            // add a filter concetenatting all the supported extensions
            filter.prepend( "Supported files (" + allexts + ");;" );

            // remove the last ";;" of the string
            filter.remove( filter.size() - 2, 2 );

            QSettings settings;
            auto path     = settings.value( "files/load", QDir::homePath() ).toString();
            auto pathList = QFileDialog::getOpenFileNames( this, "Open Files", path, filter );

            if ( !pathList.empty() ) {
                engine->getEntityManager()->deleteEntities();
                settings.setValue( "files/load", pathList.front() );
                engine->loadFile( pathList.front().toStdString() );
                engine->releaseFile();
                // Tell the window that something is to be displayed
                this->prepareDisplay();
                emit this->getViewer()->needUpdate();
            }
        };
        connect( fileOpenAction, &QAction::triggered, openFile );

        //! [Adding a keybinding to show the bounding boxes]
        auto viewer = getViewer();

        viewer->addCustomAction(
            "ShowBoundingBoxes",
            Ra::Gui::KeyMappingManager::createEventBindingFromStrings( "", "", "Key_B" ),
            [=]( QEvent* e ) {
                if ( e->type() == QEvent::KeyPress ) toggleBoundingBoxes();
            } );
        //! [Adding a keybinding to show the bounding boxes]

        // add the menu bar
        setMenuBar( menuBar );
    }

    void toggleBoundingBoxes() {
        auto engine   = Ra::Engine::RadiumEngine::getInstance();
        auto entities = engine->getEntityManager()->getEntities();
        for ( auto& entity : entities ) {
            if ( entity->getName() == "System Display Entity" ) { continue; }
            for ( auto& component : entity->getComponents() ) {
                auto pointcloud =
                    dynamic_cast<Ra::Engine::Scene::PointCloudComponent*>( component.get() );
                if ( pointcloud ) {
                    auto aabb        = pointcloud->computeAabb();
                    auto bbEntity    = new Ra::Engine::Scene::Entity( entity->getName() + "BB" );
                    auto bbComponent = new Ra::Engine::Scene::DebugComponent( bbEntity );
                    engine->getRenderObjectManager()->addRenderObject(
                        Ra::Engine::Data::DrawPrimitives::Primitive(
                            bbComponent,
                            Ra::Engine::Data::DrawPrimitives::AABB(
                                aabb, Ra::Core::Utils::Color::Cyan() ) ) );
                    int counter    = 0;
                    auto& pcgeom   = pointcloud->getCoreGeometry();
                    auto& vertices = pcgeom.vertices();
                    auto aabs      = computeKdTreeLeafAabb( pcgeom.vertices(),
                                                       pcgeom.getKdTreeNodeData(),
                                                       pcgeom.getKdTreeLeafCount() );
                    for ( auto aabb : aabs ) {
                        auto bbEntity    = new Ra::Engine::Scene::Entity( entity->getName() + "BB" +
                                                                       std::to_string( counter ) );
                        auto bbComponent = new Ra::Engine::Scene::DebugComponent( bbEntity );
                        float hue        = float( counter ) / aabs.size();
                        engine->getRenderObjectManager()->addRenderObject(
                            Ra::Engine::Data::DrawPrimitives::Primitive(
                                bbComponent,
                                Ra::Engine::Data::DrawPrimitives::AABB(
                                    aabb, Ra::Core::Utils::Color::fromHSV( hue ) ) ) );
                        ++counter;
                    }
                }
            }
        }
    }

  private:
    Ra::Core::VectorArray<Ra::Core::Aabb> m_aabbs;
};

class DemoWindowFactory : public Ra::Gui::BaseApplication::WindowFactory
{
  public:
    inline Ra::Gui::MainWindowInterface* createMainWindow() const override {
        auto window = new DemoWindow();
        return window;
    }
};

int main( int argc, char* argv[] ) {
    //! [Creating the application]
    Ra::Gui::BaseApplication app( argc, argv );
    //! [Initializing the application]
    app.initialize( DemoWindowFactory() );

    //! [Creating the application]

    return app.exec();
}
