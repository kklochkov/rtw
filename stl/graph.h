#pragma once

#include "stl/heap_array.h"
#include "stl/id.h"
#include "stl/static_queue.h"
#include "stl/static_stack.h"
#include "stl/static_vector.h"

#include <array>
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

} // namespace details

template <typename VertexAttributesT, typename EdgeAttributesT = BasicDirectedEdgeAttributes,
          typename... AdjacencyListTraitsArgsT>
class GenericDirectedGraph
{
  using AdjacencyListTraitsType =
      details::AdjacencyListTraits<VertexAttributesT, EdgeAttributesT, AdjacencyListTraitsArgsT...>;

public:
  static_assert(std::is_base_of_v<BasicDirectedEdgeAttributes, EdgeAttributesT>,
                "EdgeAttributesT must derive from BasicDirectedEdgeAttributes");

  using vertex_attributes_type = VertexAttributesT;
  using edge_attributes_type = EdgeAttributesT;
  using size_type = typename AdjacencyListTraitsType::adjacency_list_type::size_type;
  using edge_container_type = typename AdjacencyListTraitsType::edge_container_type;

  constexpr GenericDirectedGraph() noexcept = default;
  constexpr explicit GenericDirectedGraph(const size_type capacity) noexcept : vertex_attributes_{capacity} {}

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

    if constexpr (AdjacencyListTraitsType::HAS_COMPILE_TIME_CAPACITY)
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

private:
  typename AdjacencyListTraitsType::adjacency_list_type adjacency_list_;
  typename AdjacencyListTraitsType::vertex_attributes_type vertex_attributes_;
};

using DirectedGraph = GenericDirectedGraph<EmptyVertexAttributes>;

template <std::size_t CAPACITY, typename VertexAttributesT = EmptyVertexAttributes,
          typename EdgeAttributesT = BasicDirectedEdgeAttributes>
using GenericInplaceDirectedGraph =
    GenericDirectedGraph<VertexAttributesT, EdgeAttributesT, std::integral_constant<std::size_t, CAPACITY>>;

template <std::size_t CAPACITY>
using InplaceDirectedGraph = GenericInplaceDirectedGraph<CAPACITY>;

namespace details
{

template <typename VertexAttributesT, typename EdgeAttributesT, typename... ArgsT>
struct HasCycleAlgorithmTraits;

template <typename VertexAttributesT, typename EdgeAttributesT>
struct HasCycleAlgorithmTraits<VertexAttributesT, EdgeAttributesT>
{
  using graph_type = GenericDirectedGraph<VertexAttributesT, EdgeAttributesT>;

  template <typename T>
  using buffer_type = HeapArray<T>;

  template <typename T>
  constexpr static buffer_type<T> make_buffer(const graph_type& graph, const T& value) noexcept
  {
    return buffer_type<T>{graph.size(), value};
  }

  template <typename T>
  using bfs_queue_type = StaticQueue<T>;

  template <typename T>
  constexpr static bfs_queue_type<T> make_bfs_queue(const graph_type& graph) noexcept
  {
    return bfs_queue_type<T>{graph.size()};
  }

  template <typename T>
  using dfs_stack_type = StaticStack<T>;

  template <typename T>
  constexpr static dfs_stack_type<T> make_dfs_stack(const graph_type& graph) noexcept
  {
    return dfs_stack_type<T>{graph.size()};
  }
};

template <typename VertexAttributesT, typename EdgeAttributesT, std::size_t CAPACITY>
struct HasCycleAlgorithmTraits<VertexAttributesT, EdgeAttributesT, std::integral_constant<std::size_t, CAPACITY>>
{
  using graph_type =
      GenericDirectedGraph<VertexAttributesT, EdgeAttributesT, std::integral_constant<std::size_t, CAPACITY>>;

  template <typename T>
  using buffer_type = std::array<T, CAPACITY>;

  template <typename T>
  constexpr static buffer_type<T> make_buffer(const graph_type& /*graph*/, const T& value) noexcept
  {
    buffer_type<T> buffer{};
    std::fill(buffer.begin(), buffer.end(), value);
    return buffer;
  }

  template <typename T>
  using bfs_queue_type = InplaceStaticQueue<T, CAPACITY>;

  template <typename T>
  constexpr static bfs_queue_type<T> make_bfs_queue(const graph_type& /*graph*/) noexcept
  {
    return bfs_queue_type<T>{};
  }

  template <typename T>
  using dfs_stack_type = InplaceStaticStack<T, CAPACITY>;

  template <typename T>
  constexpr static dfs_stack_type<T> make_dfs_stack(const graph_type& /*graph*/) noexcept
  {
    return dfs_stack_type<T>{};
  }
};

} // namespace details

template <typename VertexAttributesT, typename EdgeAttributesT, typename... ArgsT>
inline bool has_cycle_bfs(const GenericDirectedGraph<VertexAttributesT, EdgeAttributesT, ArgsT...>& graph) noexcept
{
  using HasCycleTraits = details::HasCycleAlgorithmTraits<VertexAttributesT, EdgeAttributesT, ArgsT...>;

  auto in_degree = HasCycleTraits::template make_buffer<std::int32_t>(graph, 0);
  for (auto u = graph.get_first_vertex_id(); u <= graph.get_last_vertex_id(); ++u)
  {
    const auto& edges = graph.get_edges(u);
    for (const auto& edge : edges)
    {
      ++in_degree[edge.to];
    }
  }

  auto bfs_queue = HasCycleTraits::template make_bfs_queue<VertexId>(graph);
  for (auto u = graph.get_first_vertex_id(); u <= graph.get_last_vertex_id(); ++u)
  {
    if (in_degree[u] == 0)
    {
      bfs_queue.push(u);
    }
  }

  while (!bfs_queue.empty())
  {
    VertexId u{};
    bfs_queue.pop(u);

    const auto& edges = graph.get_edges(u);
    for (const auto& edge : edges)
    {
      --in_degree[edge.to];
      if (in_degree[edge.to] == 0)
      {
        bfs_queue.push(edge.to);
      }
    }
  }

  return std::any_of(in_degree.begin(), in_degree.end(), [](const auto degree) { return degree > 0; });
}

enum class VisitState : std::uint8_t
{
  UNVISITED = 0U,
  VISITING,
  VISITED
};

template <typename VertexAttributesT, typename EdgeAttributesT, typename... ArgsT>
inline bool
has_cycle_dfs_recursive(const GenericDirectedGraph<VertexAttributesT, EdgeAttributesT, ArgsT...>& graph) noexcept
{
  using HasCycleTraits = details::HasCycleAlgorithmTraits<VertexAttributesT, EdgeAttributesT, ArgsT...>;
  using Graph = typename HasCycleTraits::graph_type;
  using States = typename HasCycleTraits::template buffer_type<VisitState>;

  const auto has_cycle_dfs = [](States& states, const Graph& g, const VertexId u, const auto& has_cycle_dfs_ref) -> bool
  {
    states[u] = VisitState::VISITING;

    const auto& edges = g.get_edges(u);
    for (const auto& edge : edges)
    {
      const auto v = edge.to;
      if (states[v] == VisitState::VISITING)
      {
        return true;
      }

      if (states[v] == VisitState::UNVISITED)
      {
        if (has_cycle_dfs_ref(states, g, v, has_cycle_dfs_ref))
        {
          return true;
        }
      }
    }

    states[u] = VisitState::VISITED;
    return false;
  };

  auto visit_states = HasCycleTraits::template make_buffer<VisitState>(graph, VisitState::UNVISITED);
  for (auto u = graph.get_first_vertex_id(); u <= graph.get_last_vertex_id(); ++u)
  {
    if (visit_states[u] == VisitState::UNVISITED)
    {
      if (has_cycle_dfs(visit_states, graph, u, has_cycle_dfs))
      {
        return true;
      }
    }
  }

  return false;
}

template <typename VertexAttributesT, typename EdgeAttributesT, typename... ArgsT>
inline bool
has_cycle_dfs_iterative(const GenericDirectedGraph<VertexAttributesT, EdgeAttributesT, ArgsT...>& graph) noexcept
{
  using HasCycleTraits = details::HasCycleAlgorithmTraits<VertexAttributesT, EdgeAttributesT, ArgsT...>;

  struct VertexSearchAttributes
  {
    VisitState state{VisitState::UNVISITED};
    bool on_stack{false};
  };

  auto attributes = HasCycleTraits::template make_buffer<VertexSearchAttributes>(graph, VertexSearchAttributes{});
  {
    auto& vertex_attr = attributes[graph.get_first_vertex_id()];
    vertex_attr.state = VisitState::VISITING;
    vertex_attr.on_stack = true;
  }

  auto dfs_stack = HasCycleTraits::template make_dfs_stack<VertexId>(graph);
  dfs_stack.push(graph.get_first_vertex_id());

  while (!dfs_stack.empty())
  {
    const auto u = dfs_stack.top();
    auto& u_attr = attributes[u];

    bool unvisited_found = false;

    {
      const auto& edges = graph.get_edges(u);
      for (const auto& edge : edges)
      {
        const auto v = edge.to;
        auto& v_attr = attributes[v];
        if (v_attr.state == VisitState::VISITING)
        {
          if (v_attr.on_stack)
          {
            return true;
          }
        }

        if (v_attr.state == VisitState::UNVISITED)
        {
          // An unvisited neighbor found, push it to the stack and start exploring it.
          unvisited_found = true;
          v_attr.state = VisitState::VISITING;
          v_attr.on_stack = true;
          dfs_stack.push(v);
          break;
        }
      }
    }

    if (!unvisited_found)
    {
      // All neighbors have been visited, mark the vertex as fully visited.
      u_attr.state = VisitState::VISITED;
      u_attr.on_stack = false;
      dfs_stack.pop();
    }
  }

  return false;
}

} // namespace rtw::stl::graph
