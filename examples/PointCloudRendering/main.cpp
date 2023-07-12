// Include Radium base application and its simple Gui
#include "Core/Containers/KdTree.hpp"
#include "Core/Containers/VectorArray.hpp"
#include "Core/Types.hpp"
#include "Core/Utils/Index.hpp"
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
#include <Gui/Widgets/ControlPanel.hpp>

// colors
#include <Core/Utils/Color.hpp>

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMenuBar>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

#include <iostream>
#include <memory>
#include <qlayout.h>
#include <string>

void extendBoundingBoxForNode( const Ra::Core::KdTreeNode& node,
                               const Ra::Core::VectorArray<Ra::Core::KdTreeNode>& node_data,
                               const Ra::Core::Vector3Array& vertices,
                               Ra::Core::Aabb& aabb ) {
    if ( !node.is_leaf() ) {
        for ( int i = 0; i < 2; ++i ) {
            extendBoundingBoxForNode(
                node_data[node.inner.first_child_id + i], node_data, vertices, aabb );
        }
    }
    else {
        for ( int i = 0; i < node.leaf.size; ++i ) {
            const auto& v = vertices[node.leaf.start + i];
            aabb.extend( v );
        }
    }
}

void recursive( const Ra::Core::KdTreeNode& node,
                int currentDepth,
                int depth,
                const Ra::Core::VectorArray<Ra::Core::KdTreeNode>& node_data,
                const Ra::Core::Vector3Array& vertices,
                Ra::Core::VectorArray<std::pair<int, Ra::Core::Aabb>>& bbs ) {
    if ( currentDepth < depth ) {
        Ra::Core::Aabb aabb;
        extendBoundingBoxForNode( node, node_data, vertices, aabb );
        bbs.emplace_back( std::make_pair( currentDepth, aabb ) );

        if ( !node.is_leaf() ) {
            std::cout << "level: " << currentDepth << " dim: " << node.inner.dim << std::endl;
            for ( int i = 0; i < 2; ++i ) {
                recursive( node_data[node.inner.first_child_id + i],
                           currentDepth + 1,
                           depth,
                           node_data,
                           vertices,
                           bbs );
            }
        }
    }
}

Ra::Core::VectorArray<std::pair<int, Ra::Core::Aabb>>
computeKdTreeAabb( const Ra::Core::VectorArray<Ra::Core::KdTreeNode>& node_data,
                   const Ra::Core::Vector3Array& vertices,
                   int depth ) {
    const auto& root = node_data[0];
    auto bbs         = Ra::Core::VectorArray<std::pair<int, Ra::Core::Aabb>>();

    recursive( root, 0, depth, node_data, vertices, bbs );
    return bbs;
}

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

        // add the menu bar
        setMenuBar( menuBar );

        // add the dock with a MaterialParameterEditor
        m_dock = new QDockWidget( "Settings" );
        m_dock->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );

        addDockWidget( Qt::LeftDockWidgetArea, m_dock );

        connect(
            getViewer(), &Ra::Gui::Viewer::rightClickPicking, this, &DemoWindow::handlePicking );
    }

    void configure() override {
        //! [Adding a keybinding to show the bounding boxes]
        DEMO_TOGGLE = getViewer()->addCustomAction(
            "ShowBoundingBoxes",
            Ra::Gui::KeyMappingManager::createEventBindingFromStrings( "", "", "Key_B" ),
            [=]( QEvent* e ) {
                if ( e->type() == QEvent::KeyPress ) toggleBoundingBoxes();
            } );
        //! [Adding a keybinding to show the bounding boxes]
    }

    void setupControlPanel() {
        if ( m_controlPanel != nullptr ) { delete m_controlPanel; }
        m_controlPanel = new Ra::Gui::Widgets::ControlPanel( "Point Settings", false, this );
        m_controlPanel->addLabel( m_Ro->getName() );
        m_controlPanel->addPowerSliderInput(
            "Point Size",
            [this]( double value ) { std::cout << value << std::endl; },
            0.5,
            0.1,
            5.0 );
        m_controlPanel->addStretch();
        m_dock->setWidget( m_controlPanel );
    }

    void handlePicking( const Ra::Engine::Rendering::Renderer::PickingResult& pickingResult ) {
        if ( pickingResult.getRoIdx().isValid() ) {
            auto roManager = Ra::Engine::RadiumEngine::getInstance()->getRenderObjectManager();
            m_Ro           = roManager->getRenderObject( pickingResult.getRoIdx() );
            setupControlPanel();
        }
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
                    int counter    = 0;
                    auto& pcgeom   = pointcloud->getCoreGeometry();
                    auto& vertices = pcgeom.vertices();
                    int depth      = 7;
                    auto aabbs =
                        computeKdTreeAabb( pcgeom.getKdTreeNodeData(), pcgeom.vertices(), depth );
                    for ( auto aabb : aabbs ) {
                        auto bbEntity    = new Ra::Engine::Scene::Entity( entity->getName() + "BB" +
                                                                       std::to_string( counter ) );
                        auto bbComponent = new Ra::Engine::Scene::DebugComponent( bbEntity );
                        float hue        = float( aabb.first ) / depth;
                        engine->getRenderObjectManager()->addRenderObject(
                            Ra::Engine::Data::DrawPrimitives::Primitive(
                                bbComponent,
                                Ra::Engine::Data::DrawPrimitives::AABB(
                                    aabb.second, Ra::Core::Utils::Color::fromHSV( hue ) ) ) );
                        ++counter;
                    }
                }
            }
        }
    }

  private:
    bool m_bbsCalculated = false;
    std::shared_ptr<Ra::Engine::Rendering::RenderObject> m_Ro { nullptr };
    Ra::Core::VectorArray<Ra::Core::Aabb> m_aabbs;
    Ra::Core::VectorArray<Ra::Core::Utils::Index> m_indices;
    Ra::Gui::KeyMappingManager::KeyMappingAction DEMO_TOGGLE;
    Ra::Gui::Widgets::ControlPanel* m_controlPanel { nullptr };
    QDockWidget* m_dock { nullptr };
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
