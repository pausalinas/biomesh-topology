#include <gtest/gtest.h>
#include <biomesh/bounding_box/bounding_box.h>
#include <biomesh/atom/atom.h>

using namespace biomesh;

class BoundingBoxTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BoundingBoxTest, DefaultConstructor) {
    BoundingBox bbox;
    EXPECT_FALSE(bbox.isValid());
}

TEST_F(BoundingBoxTest, ParameterizedConstructor) {
    BoundingBox bbox(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    
    EXPECT_TRUE(bbox.isValid());
    EXPECT_DOUBLE_EQ(bbox.getMinX(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.getMinY(), 2.0);
    EXPECT_DOUBLE_EQ(bbox.getMinZ(), 3.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxX(), 4.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxY(), 5.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxZ(), 6.0);
}

TEST_F(BoundingBoxTest, InvalidBoundingBox) {
    // Max < Min should create invalid bounding box
    BoundingBox bbox(4.0, 5.0, 6.0, 1.0, 2.0, 3.0);
    EXPECT_FALSE(bbox.isValid());
}

TEST_F(BoundingBoxTest, Dimensions) {
    BoundingBox bbox(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    
    EXPECT_DOUBLE_EQ(bbox.getWidth(), 3.0);   // 4-1
    EXPECT_DOUBLE_EQ(bbox.getHeight(), 3.0);  // 5-2
    EXPECT_DOUBLE_EQ(bbox.getDepth(), 3.0);   // 6-3
    EXPECT_DOUBLE_EQ(bbox.getVolume(), 27.0); // 3*3*3
}

TEST_F(BoundingBoxTest, Center) {
    BoundingBox bbox(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    
    double centerX, centerY, centerZ;
    bbox.getCenter(centerX, centerY, centerZ);
    
    EXPECT_DOUBLE_EQ(centerX, 2.5); // (1+4)/2
    EXPECT_DOUBLE_EQ(centerY, 3.5); // (2+5)/2
    EXPECT_DOUBLE_EQ(centerZ, 4.5); // (3+6)/2
}

TEST_F(BoundingBoxTest, Contains) {
    BoundingBox bbox(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    
    EXPECT_TRUE(bbox.contains(2.5, 3.5, 4.5));  // Center point
    EXPECT_TRUE(bbox.contains(1.0, 2.0, 3.0));  // Min corner
    EXPECT_TRUE(bbox.contains(4.0, 5.0, 6.0));  // Max corner
    
    EXPECT_FALSE(bbox.contains(0.5, 3.5, 4.5)); // Outside X min
    EXPECT_FALSE(bbox.contains(4.5, 3.5, 4.5)); // Outside X max
    EXPECT_FALSE(bbox.contains(2.5, 1.5, 4.5)); // Outside Y min
    EXPECT_FALSE(bbox.contains(2.5, 5.5, 4.5)); // Outside Y max
    EXPECT_FALSE(bbox.contains(2.5, 3.5, 2.5)); // Outside Z min
    EXPECT_FALSE(bbox.contains(2.5, 3.5, 6.5)); // Outside Z max
}

TEST_F(BoundingBoxTest, Expand) {
    BoundingBox bbox(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    bbox.expand(1.0);
    
    EXPECT_DOUBLE_EQ(bbox.getMinX(), 0.0);
    EXPECT_DOUBLE_EQ(bbox.getMinY(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.getMinZ(), 2.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxX(), 5.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxY(), 6.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxZ(), 7.0);
}

TEST_F(BoundingBoxTest, Merge) {
    BoundingBox bbox1(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    BoundingBox bbox2(0.0, 1.0, 2.0, 5.0, 6.0, 7.0);
    
    bbox1.merge(bbox2);
    
    EXPECT_DOUBLE_EQ(bbox1.getMinX(), 0.0); // min(1.0, 0.0)
    EXPECT_DOUBLE_EQ(bbox1.getMinY(), 1.0); // min(2.0, 1.0)
    EXPECT_DOUBLE_EQ(bbox1.getMinZ(), 2.0); // min(3.0, 2.0)
    EXPECT_DOUBLE_EQ(bbox1.getMaxX(), 5.0); // max(4.0, 5.0)
    EXPECT_DOUBLE_EQ(bbox1.getMaxY(), 6.0); // max(5.0, 6.0)
    EXPECT_DOUBLE_EQ(bbox1.getMaxZ(), 7.0); // max(6.0, 7.0)
}

TEST_F(BoundingBoxTest, FromAtoms) {
    std::vector<std::unique_ptr<Atom>> atoms;
    
    atoms.push_back(std::make_unique<Atom>(1, "CA", "C", 1.0, 2.0, 3.0));
    atoms.push_back(std::make_unique<Atom>(2, "CB", "C", 4.0, 5.0, 6.0));
    atoms.push_back(std::make_unique<Atom>(3, "CG", "C", -1.0, 0.0, 1.0));
    
    BoundingBox bbox = BoundingBox::fromAtoms(atoms, 0.0);
    
    EXPECT_TRUE(bbox.isValid());
    EXPECT_DOUBLE_EQ(bbox.getMinX(), -1.0);
    EXPECT_DOUBLE_EQ(bbox.getMinY(), 0.0);
    EXPECT_DOUBLE_EQ(bbox.getMinZ(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxX(), 4.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxY(), 5.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxZ(), 6.0);
}

TEST_F(BoundingBoxTest, FromAtomsWithPadding) {
    std::vector<std::unique_ptr<Atom>> atoms;
    atoms.push_back(std::make_unique<Atom>(1, "CA", "C", 0.0, 0.0, 0.0));
    atoms.push_back(std::make_unique<Atom>(2, "CB", "C", 1.0, 1.0, 1.0));
    
    BoundingBox bbox = BoundingBox::fromAtoms(atoms, 0.5);
    
    EXPECT_DOUBLE_EQ(bbox.getMinX(), -0.5);
    EXPECT_DOUBLE_EQ(bbox.getMinY(), -0.5);
    EXPECT_DOUBLE_EQ(bbox.getMinZ(), -0.5);
    EXPECT_DOUBLE_EQ(bbox.getMaxX(), 1.5);
    EXPECT_DOUBLE_EQ(bbox.getMaxY(), 1.5);
    EXPECT_DOUBLE_EQ(bbox.getMaxZ(), 1.5);
}

TEST_F(BoundingBoxTest, FromAtomsWithRadii) {
    std::vector<std::unique_ptr<Atom>> atoms;
    
    // Create atom with radius 1.0 at origin
    atoms.push_back(std::make_unique<Atom>(1, "CA", "C", 0.0, 0.0, 0.0, 1.0));
    
    BoundingBox bbox = BoundingBox::fromAtomsWithRadii(atoms, 0.0);
    
    EXPECT_DOUBLE_EQ(bbox.getMinX(), -1.0); // 0.0 - 1.0
    EXPECT_DOUBLE_EQ(bbox.getMinY(), -1.0);
    EXPECT_DOUBLE_EQ(bbox.getMinZ(), -1.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxX(), 1.0);  // 0.0 + 1.0
    EXPECT_DOUBLE_EQ(bbox.getMaxY(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.getMaxZ(), 1.0);
}

TEST_F(BoundingBoxTest, EmptyAtomList) {
    std::vector<std::unique_ptr<Atom>> atoms; // Empty vector
    
    BoundingBox bbox = BoundingBox::fromAtoms(atoms);
    EXPECT_FALSE(bbox.isValid());
    
    BoundingBox bbox2 = BoundingBox::fromAtomsWithRadii(atoms);
    EXPECT_FALSE(bbox2.isValid());
}