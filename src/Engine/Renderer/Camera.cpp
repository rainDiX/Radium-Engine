#include <Engine/Renderer/Camera.hpp>

#include <cmath>

namespace Ra {

/// -------------------- ///
/// CONSTRUCTOR
/// -------------------- ///
Camera::Camera() :
    m_frame     ( Transform()           ),
    m_fov       ( M_PI * 50.0 / 180.0   ),
    m_zNear     ( 1.0                   ),
    m_zFar      ( 1000.0                ),
    m_focalPoint( 1.0                   ),
    m_zoomFactor( 1.0                   ),
    m_projType  ( ProjType::PERSPECTIVE ),
    m_viewMatrix( Matrix4()             ),
    m_projMatrix( Matrix4()             )
{
    updateViewMatrix();
}

Camera::Camera( const Camera& cam ) :
    m_frame     ( cam.m_frame      ),
    m_fov       ( cam.m_fov        ),
    m_zNear     ( cam.m_zNear      ),
    m_zFar      ( cam.m_zFar       ),
    m_focalPoint( cam.m_focalPoint ),
    m_zoomFactor( cam.m_zoomFactor ),
    m_projType  ( cam.m_projType   ),
    m_viewMatrix( cam.m_viewMatrix ),
    m_projMatrix( cam.m_projMatrix )
{}




/// -------------------- ///
/// DESTRUCTOR
/// -------------------- ///
Camera::~Camera() { }




/// -------------------- ///
/// FRAME
/// -------------------- ///
void Camera::applyTransform( const Transform& T,
                             const ModeType mode ) {
    Transform trans;
    switch( mode ) {
    case ModeType::FREE: {
        trans.translation() = -getPosition();
    } break;
    case ModeType::TARGET: {
        trans.translation() = -getTargetPoint();
    } break;
    default: break;
    }
    m_frame = trans.inverse( Eigen::Affine ) * T * trans * m_frame;

    if( mode == ModeType::TARGET ) {
        m_focalPoint = ( getPosition() - trans.translation() ).norm();
    }
}




/// -------------------- ///
/// VIEW MATRIX
/// -------------------- ///
void Camera::updateViewMatrix() {
    const Vector3 e  = getPosition();
    const Vector3 f  = getDirection().normalized();
    const Vector3 up = getUpVector().normalized();

    const Vector3 s = f.cross( up );
    const Vector3 u = s.cross( f  );

    Matrix4 T;
    T.block< 3, 1 >( 0, 3 ) = -e;

    m_viewMatrix.setIdentity();
    m_viewMatrix.block< 1, 3 >( 0, 0 ) =  s;
    m_viewMatrix.block< 1, 3 >( 1, 0 ) =  u;
    m_viewMatrix.block< 1, 3 >( 2, 0 ) = -f;

    m_viewMatrix = m_viewMatrix * T;
}




/// -------------------- ///
/// PROJECTION MATRIX
/// -------------------- ///
void Camera::updateProjMatrix( const Scalar& width, const Scalar& height ) {
    switch( m_projType ) {
        case ProjType::ORTHOGRAPHIC: {
            const Scalar dx = m_zoomFactor * 0.5f;
            const Scalar dy = height * dx / width;
            // ------------
            // Compute projection matrix as describe in the doc of gluPerspective()
            const Scalar l = -dx; // left
            const Scalar r =  dx; // right
            const Scalar t =  dy; // top
            const Scalar b = -dy; // bottom

            Vector3 tr;
            tr( 0 ) = -( r + l ) / ( r - l );
            tr( 1 ) = -( t + b ) / ( t - b );
            tr( 2 ) = -( ( m_zFar + m_zNear ) / ( m_zFar - m_zNear ) );

            m_projMatrix.setIdentity();

            m_projMatrix.coeffRef( 0, 0 ) = 2.0f / ( r - l );
            m_projMatrix.coeffRef( 1, 1 ) = 2.0f / ( t - b );
            m_projMatrix.coeffRef( 2, 2 ) = -2.0f / ( m_zFar - m_zNear );
            m_projMatrix.block< 3, 1 >( 0, 3 ) = tr;

        } break;

        case ProjType::PERSPECTIVE: {
            // Compute projection matrix as describe in the doc of gluPerspective()
            const Scalar f     = std::tan( ( M_PI * 0.5f ) - ( m_fov * 0.5f ) );
            const Scalar ratio = width / height;
            const Scalar diff  = m_zNear - m_zFar;

            m_projMatrix.setZero();

            m_projMatrix.coeffRef( 0, 0 ) = f / ratio;
            m_projMatrix.coeffRef( 1, 1 ) = f;
            m_projMatrix.coeffRef( 2, 2 ) = ( m_zFar + m_zNear ) / diff;
            m_projMatrix.coeffRef( 2, 3 ) = ( 2.0f * m_zFar * m_zNear ) / diff;
            m_projMatrix.coeffRef( 3, 2 ) = -1.0f;

        } break;

        default: break;
        }
}

} // End of Ra
