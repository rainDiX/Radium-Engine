#pragma once

#include <Core/Containers/VectorArray.hpp>
#include <Core/CoreMacros.hpp>
#include <Core/Types.hpp>
#include <Ponca/SpatialPartitioning>

namespace Ra {
namespace Core {

class DataPoint
{
  public:
    enum { Dim = 3 };
    using VectorType = Vector3;
    using Scalar     = Scalar;

    PONCA_MULTIARCH inline DataPoint( const VectorType& _pos = VectorType::Zero() ) :
        m_pos( _pos ) {}

    PONCA_MULTIARCH inline const VectorType& pos() const { return m_pos; }

    PONCA_MULTIARCH inline VectorType& pos() { return m_pos; }

  private:
    VectorType m_pos;
};

using KdTreeNode = Ponca::KdTreeDefaultNode<int, Scalar, DataPoint::Dim>;

template <typename T>
struct KdTreeTraits {
    using DataPoint = T;

  private:
    using Scalar     = typename DataPoint::Scalar;
    using VectorType = typename DataPoint::VectorType;

  public:
    enum {
        MAX_DEPTH = 32,
    };

    using AabbType = Core::Aabb;

    // Containers
    using IndexType      = int; // must be signed
    using PointContainer = VectorArray<DataPoint>;
    using IndexContainer = VectorArray<IndexType>;

    using NodeContainer = VectorArray<Ponca::KdTreeDefaultNode<IndexType, Scalar, DataPoint::Dim>>;
};

using KdTree = Ponca::KdTreeBase<KdTreeTraits<DataPoint>>;

using KdTreeKNearestIndexQuery = Ponca::KdTreeKNearestIndexQuery<KdTreeTraits<DataPoint>>;
using KdTreeKNearestPointQuery = Ponca::KdTreeKNearestPointQuery<KdTreeTraits<DataPoint>>;
using KdTreeNearestIndexQuery = Ponca::KdTreeNearestIndexQuery<KdTreeTraits<DataPoint>>;
using KdTreeNearestPointQuery = Ponca::KdTreeNearestPointQuery<KdTreeTraits<DataPoint>>;
using KdTreeRangeIndexQuery = Ponca::KdTreeRangeIndexQuery<KdTreeTraits<DataPoint>>;
using KdTreeRangePointQuery = Ponca::KdTreeRangePointQuery<KdTreeTraits<DataPoint>>;
} // namespace Core
} // namespace Ra