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

using InplaceDirectedGraph = rtw::stl::graph::InplaceDirectedGraph<10U>;
using InplaceDirectedGraphWithAttributes =
    rtw::stl::graph::GenericInplaceDirectedGraph<VertexAttributes, EdgeAttributes, 10U>;

} // namespace

TEST(InplaceGraphTest, constructor)
{
  InplaceDirectedGraph graph;
  EXPECT_EQ(graph.size(), 0U);
  EXPECT_EQ(graph.capacity(), 10U);
  EXPECT_TRUE(graph.empty());
}

TEST(InplaceGraphTest, add_vertex)
{
  InplaceDirectedGraph graph;
  EXPECT_EQ(graph.size(), 0U);
  EXPECT_EQ(graph.capacity(), 10U);
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

TEST(InplaceGraphTest, add_edge)
{
  InplaceDirectedGraph graph;

  const auto v0 = graph.add_vertex();
  const auto v1 = graph.add_vertex();
  const auto v2 = graph.add_vertex();

  EXPECT_EQ(graph.size(), 3U);
  EXPECT_EQ(graph.capacity(), 10U);
  EXPECT_FALSE(graph.empty());

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

TEST(InplaceGraphTest, fill_graph_with_attributes)
{
  InplaceDirectedGraphWithAttributes graph;

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
