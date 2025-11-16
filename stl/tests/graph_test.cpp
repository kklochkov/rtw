#include "stl/graph.h"

#include <gtest/gtest.h>

namespace
{

struct VertexAttributes
{
  char label{};
};

struct EdgeAttributes : rtw::stl::graph::WeightedDirectedEdgeAttributes
{
  char label{};
};

using DirectedGraph = rtw::stl::graph::DirectedGraph;
using DirectedGraphWithAttributes = rtw::stl::graph::GenericDirectedGraph<VertexAttributes, EdgeAttributes>;

} // namespace

TEST(GraphTest, constructor)
{
  DirectedGraph graph{10U};
  EXPECT_EQ(graph.size(), 0U);
  EXPECT_EQ(graph.capacity(), 10U);
  EXPECT_TRUE(graph.empty());

  EXPECT_DEATH(DirectedGraph{0U}, ".*");
}

TEST(GraphTest, add_vertex)
{
  DirectedGraph graph{5U};
  EXPECT_EQ(graph.size(), 0U);
  EXPECT_EQ(graph.capacity(), 5U);
  EXPECT_TRUE(graph.empty());

  for (std::size_t i = 0U; i < graph.capacity(); ++i)
  {
    const auto vertex_id = graph.add_vertex();
    EXPECT_EQ(vertex_id, i);
    EXPECT_EQ(graph.size(), i + 1U);
    EXPECT_FALSE(graph.empty());
    EXPECT_EQ(graph.get_first_vertex_id(), 0U);
    EXPECT_EQ(graph.get_last_vertex_id(), i);
    EXPECT_TRUE(graph.get_edges(vertex_id).empty());
  }

  EXPECT_DEATH(graph.add_vertex(), ".*");
}

TEST(GraphTest, add_edge)
{
  DirectedGraph graph{3U};

  const auto v0 = graph.add_vertex();
  const auto v1 = graph.add_vertex();
  const auto v2 = graph.add_vertex();

  graph.add_edge(v0, v1);
  graph.add_edge(v0, v2);
  graph.add_edge(v1, v2);

  const auto& edges_v0 = graph.get_edges(v0);
  EXPECT_EQ(edges_v0.size(), 2U);
  EXPECT_EQ(edges_v0[0U].to, v1);
  EXPECT_EQ(edges_v0[1U].to, v2);

  const auto& edges_v1 = graph.get_edges(v1);
  EXPECT_EQ(edges_v1.size(), 1U);
  EXPECT_EQ(edges_v1[0U].to, v2);

  const auto& edges_v2 = graph.get_edges(v2);
  EXPECT_TRUE(edges_v2.empty());
}

TEST(GraphTest, fill_graph_with_attributes)
{
  DirectedGraphWithAttributes graph{2U};

  const auto v0 = graph.add_vertex('A');
  const auto v1 = graph.add_vertex('B');

  graph.add_edge(v0, v1, 1.5F, 'a');

  EXPECT_EQ(graph[v0].label, 'A');
  EXPECT_EQ(graph[v1].label, 'B');

  const auto& edges_v0 = graph.get_edges(v0);
  EXPECT_EQ(edges_v0.size(), 1U);
  EXPECT_EQ(edges_v0[0U].to, v1);
  EXPECT_EQ(edges_v0[0U].weight, 1.5);
  EXPECT_EQ(edges_v0[0U].label, 'a');
}

TEST(GraphTest, has_cycle)
{
  {
    DirectedGraph graph{3U};

    const auto v0 = graph.add_vertex();
    const auto v1 = graph.add_vertex();
    const auto v2 = graph.add_vertex();

    graph.add_edge(v0, v1);
    graph.add_edge(v1, v2);
    graph.add_edge(v2, v0); // Cycle

    EXPECT_TRUE(rtw::stl::graph::has_cycle_bfs(graph));
    EXPECT_TRUE(rtw::stl::graph::has_cycle_dfs_iterative(graph));
    EXPECT_TRUE(rtw::stl::graph::has_cycle_dfs_recursive(graph));
  }
  {
    DirectedGraph graph{3U};

    const auto v0 = graph.add_vertex();

    graph.add_edge(v0, v0); // Self-loop cycle

    EXPECT_TRUE(rtw::stl::graph::has_cycle_bfs(graph));
    EXPECT_TRUE(rtw::stl::graph::has_cycle_dfs_iterative(graph));
    EXPECT_TRUE(rtw::stl::graph::has_cycle_dfs_recursive(graph));
  }
  {
    rtw::stl::graph::GenericDirectedGraph<VertexAttributes> graph{6U};

    const auto u = graph.add_vertex('u');
    const auto v = graph.add_vertex('v');
    const auto w = graph.add_vertex('w');
    const auto x = graph.add_vertex('x');
    const auto y = graph.add_vertex('y');
    const auto z = graph.add_vertex('z');

    graph.add_edge(u, v);
    graph.add_edge(u, x);
    graph.add_edge(v, y);
    graph.add_edge(x, v);
    graph.add_edge(y, x);
    graph.add_edge(w, y);
    graph.add_edge(w, z);
    graph.add_edge(z, z); // Self-loop cycle

    {
      for (const auto& edge : graph.get_edges(u))
      {
        EXPECT_TRUE((edge.to == v) || (edge.to == x));
        EXPECT_TRUE(graph[edge.to].label == 'v' || graph[edge.to].label == 'x');
      }

      for (const auto& edge : graph.get_edges(v))
      {
        EXPECT_EQ(edge.to, y);
        EXPECT_EQ(graph[edge.to].label, 'y');
      }

      for (const auto& edge : graph.get_edges(x))
      {
        EXPECT_EQ(edge.to, v);
        EXPECT_EQ(graph[edge.to].label, 'v');
      }

      for (const auto& edge : graph.get_edges(y))
      {
        EXPECT_EQ(edge.to, x);
        EXPECT_EQ(graph[edge.to].label, 'x');
      }

      for (const auto& edge : graph.get_edges(w))
      {
        EXPECT_TRUE((edge.to == y) || (edge.to == z));
        EXPECT_TRUE(graph[edge.to].label == 'y' || graph[edge.to].label == 'z');
      }

      for (const auto& edge : graph.get_edges(z))
      {
        EXPECT_EQ(edge.to, z);
        EXPECT_EQ(graph[edge.to].label, 'z');
      }
    }

    EXPECT_TRUE(rtw::stl::graph::has_cycle_bfs(graph));
    EXPECT_TRUE(rtw::stl::graph::has_cycle_dfs_iterative(graph));
    EXPECT_TRUE(rtw::stl::graph::has_cycle_dfs_recursive(graph));
  }
  {
    DirectedGraph graph{3U};

    const auto v0 = graph.add_vertex();
    const auto v1 = graph.add_vertex();
    const auto v2 = graph.add_vertex();

    graph.add_edge(v0, v1);
    graph.add_edge(v1, v2);

    EXPECT_FALSE(rtw::stl::graph::has_cycle_bfs(graph));
    EXPECT_FALSE(rtw::stl::graph::has_cycle_dfs_iterative(graph));
    EXPECT_FALSE(rtw::stl::graph::has_cycle_dfs_recursive(graph));
  }
}
