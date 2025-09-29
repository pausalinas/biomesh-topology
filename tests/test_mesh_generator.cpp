#include <gtest/gtest.h>
#include <biomesh/mesh_generator/mesh_generator.h>
#include <biomesh/voxel_grid/voxel_grid.h>
#include <biomesh/atom/atom.h>

using namespace biomesh;

class MeshGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        bbox_ = BoundingBox(0.0, 0.0, 0.0, 10.0, 10.0, 10.0);
        setupTestAtoms();
    }
    
    void TearDown() override {}
    
    void setupTestAtoms() {
        atoms_.clear();
        atoms_.push_back(std::make_unique<Atom>(1, "CA", "C", 5.0, 5.0, 5.0, 1.0));
        atoms_.push_back(std::make_unique<Atom>(2, "CB", "C", 6.0, 5.0, 5.0, 1.0));
        atoms_.push_back(std::make_unique<Atom>(3, "CG", "C", 5.0, 6.0, 5.0, 1.0));
    }
    
    BoundingBox bbox_;
    std::vector<std::unique_ptr<Atom>> atoms_;
};

// Mesh Tests
TEST_F(MeshGeneratorTest, MeshDefaultConstructor) {
    Mesh mesh;
    EXPECT_EQ(mesh.getNodeCount(), 0);
    EXPECT_EQ(mesh.getElementCount(), 0);
    EXPECT_TRUE(mesh.isValid()); // Empty mesh is valid
}

TEST_F(MeshGeneratorTest, MeshAddNode) {
    Mesh mesh;
    MeshNode node(1, 1.0, 2.0, 3.0);
    
    mesh.addNode(node);
    
    EXPECT_EQ(mesh.getNodeCount(), 1);
    const auto& nodes = mesh.getNodes();
    EXPECT_EQ(nodes[0].id, 1);
    EXPECT_DOUBLE_EQ(nodes[0].x, 1.0);
    EXPECT_DOUBLE_EQ(nodes[0].y, 2.0);
    EXPECT_DOUBLE_EQ(nodes[0].z, 3.0);
}

TEST_F(MeshGeneratorTest, MeshAddElement) {
    Mesh mesh;
    
    // Add nodes first
    mesh.addNode(MeshNode(1, 0.0, 0.0, 0.0));
    mesh.addNode(MeshNode(2, 1.0, 0.0, 0.0));
    mesh.addNode(MeshNode(3, 0.0, 1.0, 0.0));
    mesh.addNode(MeshNode(4, 0.0, 0.0, 1.0));
    
    // Add element
    MeshElement element(1, {1, 2, 3, 4});
    mesh.addElement(element);
    
    EXPECT_EQ(mesh.getElementCount(), 1);
    const auto& elements = mesh.getElements();
    EXPECT_EQ(elements[0].id, 1);
    EXPECT_EQ(elements[0].nodeIds.size(), 4);
    EXPECT_EQ(elements[0].nodeIds[0], 1);
    EXPECT_EQ(elements[0].nodeIds[1], 2);
    EXPECT_EQ(elements[0].nodeIds[2], 3);
    EXPECT_EQ(elements[0].nodeIds[3], 4);
}

TEST_F(MeshGeneratorTest, MeshValidation) {
    Mesh mesh;
    
    // Add nodes
    mesh.addNode(MeshNode(1, 0.0, 0.0, 0.0));
    mesh.addNode(MeshNode(2, 1.0, 0.0, 0.0));
    
    // Add valid element
    mesh.addElement(MeshElement(1, {1, 2}));
    EXPECT_TRUE(mesh.isValid());
    
    // Add invalid element (references non-existent node)
    mesh.addElement(MeshElement(2, {1, 3})); // Node 3 doesn't exist
    EXPECT_FALSE(mesh.isValid());
}

TEST_F(MeshGeneratorTest, MeshClear) {
    Mesh mesh;
    mesh.addNode(MeshNode(1, 0.0, 0.0, 0.0));
    mesh.addElement(MeshElement(1, {1}));
    
    EXPECT_GT(mesh.getNodeCount(), 0);
    EXPECT_GT(mesh.getElementCount(), 0);
    
    mesh.clear();
    
    EXPECT_EQ(mesh.getNodeCount(), 0);
    EXPECT_EQ(mesh.getElementCount(), 0);
}

TEST_F(MeshGeneratorTest, MeshCopyConstructor) {
    Mesh original;
    original.addNode(MeshNode(1, 1.0, 2.0, 3.0));
    original.addElement(MeshElement(1, {1}));
    
    Mesh copy(original);
    
    EXPECT_EQ(copy.getNodeCount(), original.getNodeCount());
    EXPECT_EQ(copy.getElementCount(), original.getElementCount());
    
    const auto& copyNodes = copy.getNodes();
    const auto& origNodes = original.getNodes();
    EXPECT_EQ(copyNodes[0].id, origNodes[0].id);
    EXPECT_DOUBLE_EQ(copyNodes[0].x, origNodes[0].x);
}

// MeshGenerator Tests
TEST_F(MeshGeneratorTest, MeshGeneratorConstructor) {
    MeshGenerator generator;
    
    const auto& stats = generator.getMeshStats();
    EXPECT_EQ(stats.inputVoxels, 0);
    EXPECT_EQ(stats.generatedNodes, 0);
    EXPECT_EQ(stats.generatedElements, 0);
}

TEST_F(MeshGeneratorTest, SetQualityParameters) {
    MeshGenerator generator;
    
    generator.setQualityParameters(20.0, 150.0);
    // No direct way to test internal values, but call should succeed
    EXPECT_NO_THROW(generator.setQualityParameters(20.0, 150.0));
    
    // Test boundary conditions
    generator.setQualityParameters(-10.0, 200.0); // Should clamp values
    generator.setQualityParameters(70.0, 100.0);  // Should clamp min angle
}

TEST_F(MeshGeneratorTest, SetMeshDensity) {
    MeshGenerator generator;
    
    generator.setMeshDensity(1.5);
    EXPECT_NO_THROW(generator.setMeshDensity(1.5));
    
    // Test boundary conditions
    generator.setMeshDensity(-1.0); // Should clamp to 0.1
    generator.setMeshDensity(5.0);  // Should clamp to 2.0
}

TEST_F(MeshGeneratorTest, GenerateMeshFromEmptyVoxelGrid) {
    VoxelGrid grid(bbox_, 1.0);
    std::vector<std::unique_ptr<Atom>> emptyAtoms;
    grid.voxelize(emptyAtoms);
    
    MeshGenerator generator;
    auto mesh = generator.generateMesh(grid);
    
    EXPECT_EQ(mesh->getNodeCount(), 0);
    EXPECT_EQ(mesh->getElementCount(), 0);
    
    const auto& stats = generator.getMeshStats();
    EXPECT_EQ(stats.inputVoxels, 0);
    EXPECT_EQ(stats.generatedNodes, 0);
    EXPECT_EQ(stats.generatedElements, 0);
}

TEST_F(MeshGeneratorTest, GenerateMeshFromVoxelGrid) {
    VoxelGrid grid(bbox_, 2.0);
    grid.voxelize(atoms_);
    
    EXPECT_GT(grid.getOccupiedVoxelCount(), 0);
    
    MeshGenerator generator;
    auto mesh = generator.generateMesh(grid);
    
    EXPECT_GT(mesh->getNodeCount(), 0);
    EXPECT_TRUE(mesh->isValid());
    
    const auto& stats = generator.getMeshStats();
    EXPECT_GT(stats.inputVoxels, 0);
    EXPECT_GT(stats.generatedNodes, 0);
    EXPECT_EQ(stats.generatedNodes, mesh->getNodeCount());
}

TEST_F(MeshGeneratorTest, GenerateTetrahedralMesh) {
    VoxelGrid grid(bbox_, 2.0);
    grid.voxelize(atoms_);
    
    MeshGenerator generator;
    auto mesh = generator.generateTetrahedralMesh(grid);
    
    EXPECT_TRUE(mesh->isValid());
    
    // Check that elements are tetrahedral (4 nodes each)
    const auto& elements = mesh->getElements();
    for (const auto& element : elements) {
        EXPECT_EQ(element.nodeIds.size(), 4);
    }
}

TEST_F(MeshGeneratorTest, MeshStatisticsUpdate) {
    VoxelGrid grid(bbox_, 2.0);
    grid.voxelize(atoms_);
    
    MeshGenerator generator;
    auto mesh = generator.generateMesh(grid);
    
    const auto& stats = generator.getMeshStats();
    
    // Quality metrics should be in valid range [0, 1]
    EXPECT_GE(stats.minElementQuality, 0.0);
    EXPECT_LE(stats.minElementQuality, 1.0);
    EXPECT_GE(stats.avgElementQuality, 0.0);
    EXPECT_LE(stats.avgElementQuality, 1.0);
    EXPECT_GE(stats.maxElementQuality, 0.0);
    EXPECT_LE(stats.maxElementQuality, 1.0);
    
    // Average should be between min and max
    EXPECT_GE(stats.avgElementQuality, stats.minElementQuality);
    EXPECT_LE(stats.avgElementQuality, stats.maxElementQuality);
}