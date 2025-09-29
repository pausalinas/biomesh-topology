#include <gtest/gtest.h>
#include <biomesh/atom/atom.h>

using namespace biomesh;

class AtomTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(AtomTest, DefaultConstructor) {
    Atom atom;
    EXPECT_EQ(atom.getSerial(), 0);
    EXPECT_EQ(atom.getName(), "");
    EXPECT_EQ(atom.getElement(), "");
    EXPECT_DOUBLE_EQ(atom.getX(), 0.0);
    EXPECT_DOUBLE_EQ(atom.getY(), 0.0);
    EXPECT_DOUBLE_EQ(atom.getZ(), 0.0);
    EXPECT_DOUBLE_EQ(atom.getRadius(), 1.0);
}

TEST_F(AtomTest, ParameterizedConstructor) {
    Atom atom(1, "CA", "C", 1.5, 2.5, 3.5, 1.7);
    EXPECT_EQ(atom.getSerial(), 1);
    EXPECT_EQ(atom.getName(), "CA");
    EXPECT_EQ(atom.getElement(), "C");
    EXPECT_DOUBLE_EQ(atom.getX(), 1.5);
    EXPECT_DOUBLE_EQ(atom.getY(), 2.5);
    EXPECT_DOUBLE_EQ(atom.getZ(), 3.5);
    EXPECT_DOUBLE_EQ(atom.getRadius(), 1.7);
}

TEST_F(AtomTest, CopyConstructor) {
    Atom original(1, "CA", "C", 1.5, 2.5, 3.5, 1.7);
    Atom copy(original);
    
    EXPECT_EQ(copy.getSerial(), original.getSerial());
    EXPECT_EQ(copy.getName(), original.getName());
    EXPECT_EQ(copy.getElement(), original.getElement());
    EXPECT_DOUBLE_EQ(copy.getX(), original.getX());
    EXPECT_DOUBLE_EQ(copy.getY(), original.getY());
    EXPECT_DOUBLE_EQ(copy.getZ(), original.getZ());
    EXPECT_DOUBLE_EQ(copy.getRadius(), original.getRadius());
}

TEST_F(AtomTest, AssignmentOperator) {
    Atom original(1, "CA", "C", 1.5, 2.5, 3.5, 1.7);
    Atom assigned;
    assigned = original;
    
    EXPECT_EQ(assigned.getSerial(), original.getSerial());
    EXPECT_EQ(assigned.getName(), original.getName());
    EXPECT_EQ(assigned.getElement(), original.getElement());
    EXPECT_DOUBLE_EQ(assigned.getX(), original.getX());
    EXPECT_DOUBLE_EQ(assigned.getY(), original.getY());
    EXPECT_DOUBLE_EQ(assigned.getZ(), original.getZ());
    EXPECT_DOUBLE_EQ(assigned.getRadius(), original.getRadius());
}

TEST_F(AtomTest, SetPosition) {
    Atom atom;
    atom.setPosition(10.0, 20.0, 30.0);
    
    EXPECT_DOUBLE_EQ(atom.getX(), 10.0);
    EXPECT_DOUBLE_EQ(atom.getY(), 20.0);
    EXPECT_DOUBLE_EQ(atom.getZ(), 30.0);
}

TEST_F(AtomTest, DistanceCalculation) {
    Atom atom1(1, "CA", "C", 0.0, 0.0, 0.0);
    Atom atom2(2, "CB", "C", 3.0, 4.0, 0.0);
    
    double distance = atom1.distanceTo(atom2);
    EXPECT_DOUBLE_EQ(distance, 5.0); // 3-4-5 triangle
}

// AtomBuilder Tests
class AtomBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(AtomBuilderTest, BasicBuilder) {
    AtomBuilder builder;
    auto atom = builder.setSerial(42)
                      .setName("CA")
                      .setElement("C")
                      .setPosition(1.0, 2.0, 3.0)
                      .setRadius(1.7)
                      .build();
    
    EXPECT_EQ(atom->getSerial(), 42);
    EXPECT_EQ(atom->getName(), "CA");
    EXPECT_EQ(atom->getElement(), "C");
    EXPECT_DOUBLE_EQ(atom->getX(), 1.0);
    EXPECT_DOUBLE_EQ(atom->getY(), 2.0);
    EXPECT_DOUBLE_EQ(atom->getZ(), 3.0);
    EXPECT_DOUBLE_EQ(atom->getRadius(), 1.7);
}

TEST_F(AtomBuilderTest, EnrichWithStandardRadius) {
    AtomBuilder builder;
    auto atom = builder.setElement("C")
                      .enrichWithStandardRadius()
                      .build();
    
    EXPECT_DOUBLE_EQ(atom->getRadius(), 1.7); // Carbon radius
    
    // Test hydrogen
    auto hydrogenAtom = AtomBuilder()
                              .setElement("H")
                              .enrichWithStandardRadius()
                              .build();
    
    EXPECT_DOUBLE_EQ(hydrogenAtom->getRadius(), 1.2); // Hydrogen radius
}

TEST_F(AtomBuilderTest, UnknownElementDefaultRadius) {
    AtomBuilder builder;
    auto atom = builder.setElement("XX") // Unknown element
                      .enrichWithStandardRadius()
                      .build();
    
    EXPECT_DOUBLE_EQ(atom->getRadius(), 1.7); // Default to carbon radius
}