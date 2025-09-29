#include <gtest/gtest.h>
#include <biomesh/voxel_grid/voxel_grid.h>
#include <biomesh/atom/atom.h>

using namespace biomesh;

class VoxelGridTest : public ::testing::Test {
protected:
    void SetUp() override {
        bbox_ = BoundingBox(0.0, 0.0, 0.0, 10.0, 10.0, 10.0);
    }
    void TearDown() override {}
    
    BoundingBox bbox_;
};

TEST_F(VoxelGridTest, Constructor) {
    VoxelGrid grid(bbox_, 1.0);
    
    int nx, ny, nz;
    grid.getDimensions(nx, ny, nz);
    
    EXPECT_EQ(nx, 10); // 10.0 / 1.0
    EXPECT_EQ(ny, 10);
    EXPECT_EQ(nz, 10);
    EXPECT_EQ(grid.getTotalVoxels(), 1000);
    EXPECT_DOUBLE_EQ(grid.getVoxelSize(), 1.0);
}

TEST_F(VoxelGridTest, InvalidBoundingBox) {
    BoundingBox invalidBbox; // Invalid bounding box
    VoxelGrid grid(invalidBbox, 1.0);
    
    int nx, ny, nz;
    grid.getDimensions(nx, ny, nz);
    
    EXPECT_EQ(nx, 0);
    EXPECT_EQ(ny, 0);
    EXPECT_EQ(nz, 0);
}

TEST_F(VoxelGridTest, WorldToGrid) {
    VoxelGrid grid(bbox_, 2.0);
    
    int i, j, k;
    bool valid = grid.worldToGrid(1.0, 3.0, 5.0, i, j, k);
    
    EXPECT_TRUE(valid);
    EXPECT_EQ(i, 0); // (1.0 - 0.0) / 2.0 = 0.5 -> 0
    EXPECT_EQ(j, 1); // (3.0 - 0.0) / 2.0 = 1.5 -> 1
    EXPECT_EQ(k, 2); // (5.0 - 0.0) / 2.0 = 2.5 -> 2
}

TEST_F(VoxelGridTest, GridToWorld) {
    VoxelGrid grid(bbox_, 2.0);
    
    double x, y, z;
    grid.gridToWorld(1, 2, 3, x, y, z);
    
    EXPECT_DOUBLE_EQ(x, 3.0); // 0.0 + (1 + 0.5) * 2.0
    EXPECT_DOUBLE_EQ(y, 5.0); // 0.0 + (2 + 0.5) * 2.0
    EXPECT_DOUBLE_EQ(z, 7.0); // 0.0 + (3 + 0.5) * 2.0
}

TEST_F(VoxelGridTest, GetVoxel) {
    VoxelGrid grid(bbox_, 1.0);
    
    const Voxel& voxel = grid.getVoxel(2, 3, 4);
    EXPECT_EQ(voxel.i, 2);
    EXPECT_EQ(voxel.j, 3);
    EXPECT_EQ(voxel.k, 4);
    EXPECT_DOUBLE_EQ(voxel.x, 2.5); // Center of voxel at (2,3,4)
    EXPECT_DOUBLE_EQ(voxel.y, 3.5);
    EXPECT_DOUBLE_EQ(voxel.z, 4.5);
    EXPECT_FALSE(voxel.isOccupied); // Initially not occupied
}

TEST_F(VoxelGridTest, VoxelizeEmptyAtomList) {
    VoxelGrid grid(bbox_, 1.0);
    std::vector<std::unique_ptr<Atom>> atoms; // Empty
    
    grid.voxelize(atoms);
    
    EXPECT_EQ(grid.getOccupiedVoxelCount(), 0);
    
    auto occupied = grid.getOccupiedVoxels();
    EXPECT_TRUE(occupied.empty());
}

TEST_F(VoxelGridTest, VoxelizeSingleAtom) {
    VoxelGrid grid(bbox_, 1.0);
    std::vector<std::unique_ptr<Atom>> atoms;
    
    // Place atom at center with radius 1.0
    atoms.push_back(std::make_unique<Atom>(1, "CA", "C", 5.0, 5.0, 5.0, 1.0));
    
    grid.voxelize(atoms);
    
    size_t occupiedCount = grid.getOccupiedVoxelCount();
    EXPECT_GT(occupiedCount, 0);
    
    // The central voxel should definitely be occupied
    const Voxel& centralVoxel = grid.getVoxel(5, 5, 5); // Grid index for (5,5,5)
    EXPECT_TRUE(centralVoxel.isOccupied);
    EXPECT_GT(centralVoxel.density, 0.0);
}

TEST_F(VoxelGridTest, VoxelizeMultipleAtoms) {
    VoxelGrid grid(bbox_, 1.0);
    std::vector<std::unique_ptr<Atom>> atoms;
    
    atoms.push_back(std::make_unique<Atom>(1, "CA", "C", 2.0, 2.0, 2.0, 1.0));
    atoms.push_back(std::make_unique<Atom>(2, "CB", "C", 8.0, 8.0, 8.0, 1.0));
    
    grid.voxelize(atoms);
    
    size_t occupiedCount = grid.getOccupiedVoxelCount();
    EXPECT_GT(occupiedCount, 0);
    
    auto occupiedVoxels = grid.getOccupiedVoxels();
    EXPECT_FALSE(occupiedVoxels.empty());
}

TEST_F(VoxelGridTest, CopyConstructor) {
    VoxelGrid original(bbox_, 2.0);
    std::vector<std::unique_ptr<Atom>> atoms;
    atoms.push_back(std::make_unique<Atom>(1, "CA", "C", 5.0, 5.0, 5.0, 1.0));
    original.voxelize(atoms);
    
    VoxelGrid copy(original);
    
    int nx, ny, nz;
    copy.getDimensions(nx, ny, nz);
    EXPECT_EQ(nx, 5); // 10.0 / 2.0
    EXPECT_EQ(ny, 5);
    EXPECT_EQ(nz, 5);
    
    EXPECT_EQ(copy.getOccupiedVoxelCount(), original.getOccupiedVoxelCount());
    EXPECT_DOUBLE_EQ(copy.getVoxelSize(), original.getVoxelSize());
}

TEST_F(VoxelGridTest, MoveConstructor) {
    VoxelGrid original(bbox_, 2.0);
    std::vector<std::unique_ptr<Atom>> atoms;
    atoms.push_back(std::make_unique<Atom>(1, "CA", "C", 5.0, 5.0, 5.0, 1.0));
    original.voxelize(atoms);
    
    size_t originalOccupied = original.getOccupiedVoxelCount();
    
    VoxelGrid moved(std::move(original));
    
    int nx, ny, nz;
    moved.getDimensions(nx, ny, nz);
    EXPECT_EQ(nx, 5);
    EXPECT_EQ(ny, 5);
    EXPECT_EQ(nz, 5);
    
    EXPECT_EQ(moved.getOccupiedVoxelCount(), originalOccupied);
    
    // Original should be in moved-from state
    int origNx, origNy, origNz;
    original.getDimensions(origNx, origNy, origNz);
    EXPECT_EQ(origNx, 0);
    EXPECT_EQ(origNy, 0);
    EXPECT_EQ(origNz, 0);
}

TEST_F(VoxelGridTest, InvalidVoxelAccess) {
    VoxelGrid grid(bbox_, 1.0);
    
    // Access out of bounds - should return invalid voxel
    const Voxel& invalidVoxel = grid.getVoxel(-1, 5, 5);
    EXPECT_EQ(invalidVoxel.i, 0);
    EXPECT_EQ(invalidVoxel.j, 0);
    EXPECT_EQ(invalidVoxel.k, 0);
    
    const Voxel& invalidVoxel2 = grid.getVoxel(15, 5, 5); // Out of bounds
    EXPECT_EQ(invalidVoxel2.i, 0);
    EXPECT_EQ(invalidVoxel2.j, 0);
    EXPECT_EQ(invalidVoxel2.k, 0);
}