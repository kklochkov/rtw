#pragma once

#include "stl/id.h"
#include "stl/static_vector.h"

#include <vector>

namespace rtw::stl::graph
{

struct VertexId : stl::Id
{};

struct BasicDirectedEdgeAttributes
{
  VertexId to{};
};

struct WeightedDirectedEdgeAttributes : BasicDirectedEdgeAttributes
{
  float weight{1.0F};
};

struct EmptyVertexAttributes
{};

namespace details
{

template <typename, typename... ArgsT>
struct ContainerSelector;

template <typename T>
struct ContainerSelector<T>
{
  // Usage of the std::vector allows to use StaticVector as dependent type,
  // because it is not default constructible without a capacity parameter.
  using vector_type = std::vector<T>;
  using static_vector_type = StaticVector<T>;
  constexpr static bool HAS_COMPILE_TIME_CAPACITY = false;
};

template <typename T, std::size_t CAPACITY>
struct ContainerSelector<T, std::integral_constant<std::size_t, CAPACITY>>
{
  using vector_type = InplaceStaticVector<T, CAPACITY>;
  using static_vector_type = vector_type;
  constexpr static bool HAS_COMPILE_TIME_CAPACITY = true;
};

template <typename VertexAttributesT, typename EdgeAttributesT, typename... ArgsT>
struct AdjacencyListTraits
{
  template <typename T>
  using vector_type = typename ContainerSelector<T, ArgsT...>::vector_type;
  template <typename T>
  using static_vector_type = typename ContainerSelector<T, ArgsT...>::static_vector_type;

  using edge_container_type = static_vector_type<EdgeAttributesT>;
  using adjacency_list_type = vector_type<edge_container_type>;
  using vertex_attributes_type = static_vector_type<VertexAttributesT>;

  constexpr static bool HAS_COMPILE_TIME_CAPACITY =
      ContainerSelector<EdgeAttributesT, ArgsT...>::HAS_COMPILE_TIME_CAPACITY;
};

template <typename VertexAttributesT, typename EdgeAttributesT,
          typename AdjacencyListTraitsT = AdjacencyListTraits<VertexAttributesT, EdgeAttributesT>>
class GenericDirectedGraph
{
  using AdjacencyListStorageType = typename AdjacencyListTraitsT::adjacency_list_type;
  using VertexAttributesStorageType = typename AdjacencyListTraitsT::vertex_attributes_type;

public:
  static_assert(std::is_base_of_v<BasicDirectedEdgeAttributes, EdgeAttributesT>,
                "EdgeAttributesT must derive from BasicDirectedEdgeAttributes");

  using vertex_attributes_type = VertexAttributesT;
  using edge_attributes_type = EdgeAttributesT;
  using size_type = typename AdjacencyListStorageType::size_type;
  using edge_container_type = typename AdjacencyListTraitsT::edge_container_type;

  constexpr GenericDirectedGraph() noexcept = default;

  constexpr size_type size() const noexcept { return vertex_attributes_.size(); }
  constexpr bool empty() const noexcept { return vertex_attributes_.empty(); }
  constexpr size_type capacity() const noexcept { return vertex_attributes_.capacity(); }

  constexpr VertexId get_first_vertex_id() const noexcept { return VertexId{0U}; }
  constexpr VertexId get_last_vertex_id() const noexcept
  {
    return VertexId{static_cast<VertexId::ID_TYPE>(size() - 1U)};
  }

  template <typename... ArgsT>
  constexpr VertexId add_vertex(ArgsT&&... args) noexcept
  {
    assert(size() < capacity());
    const VertexId vertex_id{static_cast<VertexId::ID_TYPE>(size())};

    if constexpr (AdjacencyListTraitsT::HAS_COMPILE_TIME_CAPACITY)
    {
      adjacency_list_.emplace_back();
    }
    else
    {
      adjacency_list_.emplace_back(capacity());
    }

    vertex_attributes_.emplace_back(std::forward<ArgsT>(args)...);
    return vertex_id;
  }

  template <typename U = vertex_attributes_type, typename = std::enable_if_t<!std::is_same_v<U, EmptyVertexAttributes>>>
  constexpr const vertex_attributes_type& operator[](const VertexId vertex_id) const noexcept
  {
    return vertex_attributes_[vertex_id];
  }

  template <typename U = vertex_attributes_type, typename = std::enable_if_t<!std::is_same_v<U, EmptyVertexAttributes>>>
  constexpr vertex_attributes_type& operator[](const VertexId vertex_id) noexcept
  {
    return vertex_attributes_[vertex_id];
  }

  constexpr void add_edge(const VertexId from, const VertexId to) noexcept { adjacency_list_[from].emplace_back(to); }

  template <typename... ArgsT>
  constexpr void add_edge(const VertexId from, const VertexId to, ArgsT&&... args) noexcept
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
    adjacency_list_[from].emplace_back(edge_attributes_type{to, std::forward<ArgsT>(args)...});
#pragma clang diagnostic pop
  }

  constexpr const edge_container_type& get_edges(const VertexId vertex_id) const noexcept
  {
    return adjacency_list_[vertex_id];
  }

protected:
  constexpr explicit GenericDirectedGraph(const size_type capacity) noexcept : vertex_attributes_{capacity} {}

private:
  AdjacencyListStorageType adjacency_list_;
  VertexAttributesStorageType vertex_attributes_;
};

} // namespace details

template <typename VertexAttributesT, typename EdgeAttributesT,
          typename BaseT = details::GenericDirectedGraph<VertexAttributesT, EdgeAttributesT>>
class GenericDirectedGraph : public BaseT
{
public:
  constexpr explicit GenericDirectedGraph(const typename BaseT::size_type capacity) noexcept : BaseT{capacity} {}
};

using DirectedGraph = GenericDirectedGraph<EmptyVertexAttributes, BasicDirectedEdgeAttributes>;

template <typename VertexAttributesT, typename EdgeAttributesT, std::size_t CAPACITY>
using GenericInplaceDirectedGraph = details::GenericDirectedGraph<
    VertexAttributesT, EdgeAttributesT,
    details::AdjacencyListTraits<VertexAttributesT, EdgeAttributesT, std::integral_constant<std::size_t, CAPACITY>>>;

template <std::size_t CAPACITY>
using InplaceDirectedGraph = GenericInplaceDirectedGraph<EmptyVertexAttributes, BasicDirectedEdgeAttributes, CAPACITY>;

} // namespace rtw::stl::graph
