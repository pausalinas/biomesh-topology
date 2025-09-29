#include <gtest/gtest.h>
#include <biomesh/pdb_parser/pdb_parser.h>
#include <sstream>

using namespace biomesh;

class PDBParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(PDBParserTest, DefaultConstructor) {
    PDBParser parser;
    EXPECT_EQ(parser.getAtomCount(), 0);
}

TEST_F(PDBParserTest, ParseSimpleContent) {
    PDBParser parser;
    
    std::string pdbContent = 
        "ATOM      1  CA  ALA A   1      10.000  20.000  30.000  1.00 50.00           C  \n"
        "ATOM      2  CB  ALA A   1      11.000  21.000  31.000  1.00 50.00           C  \n";
    
    auto atoms = parser.parseContent(pdbContent);
    
    EXPECT_EQ(atoms.size(), 2);
    EXPECT_EQ(parser.getAtomCount(), 2);
    
    // Check first atom
    EXPECT_EQ(atoms[0]->getSerial(), 1);
    EXPECT_EQ(atoms[0]->getName(), "CA");
    EXPECT_EQ(atoms[0]->getElement(), "C");
    EXPECT_DOUBLE_EQ(atoms[0]->getX(), 10.000);
    EXPECT_DOUBLE_EQ(atoms[0]->getY(), 20.000);
    EXPECT_DOUBLE_EQ(atoms[0]->getZ(), 30.000);
    
    // Check second atom
    EXPECT_EQ(atoms[1]->getSerial(), 2);
    EXPECT_EQ(atoms[1]->getName(), "CB");
    EXPECT_EQ(atoms[1]->getElement(), "C");
    EXPECT_DOUBLE_EQ(atoms[1]->getX(), 11.000);
    EXPECT_DOUBLE_EQ(atoms[1]->getY(), 21.000);
    EXPECT_DOUBLE_EQ(atoms[1]->getZ(), 31.000);
}

TEST_F(PDBParserTest, ParseWithHETATM) {
    PDBParser parser;
    parser.setIncludeHetAtoms(true);
    
    std::string pdbContent = 
        "ATOM      1  CA  ALA A   1      10.000  20.000  30.000  1.00 50.00           C  \n"
        "HETATM    2  O   HOH S   1      15.000  25.000  35.000  1.00 60.00           O  \n";
    
    auto atoms = parser.parseContent(pdbContent);
    
    EXPECT_EQ(atoms.size(), 2);
    
    // Check HETATM record
    EXPECT_EQ(atoms[1]->getSerial(), 2);
    EXPECT_EQ(atoms[1]->getName(), "O");
    EXPECT_EQ(atoms[1]->getElement(), "O");
}

TEST_F(PDBParserTest, ExcludeHETATM) {
    PDBParser parser;
    parser.setIncludeHetAtoms(false);
    
    std::string pdbContent = 
        "ATOM      1  CA  ALA A   1      10.000  20.000  30.000  1.00 50.00           C  \n"
        "HETATM    2  O   HOH S   1      15.000  25.000  35.000  1.00 60.00           O  \n";
    
    auto atoms = parser.parseContent(pdbContent);
    
    EXPECT_EQ(atoms.size(), 1); // Only ATOM record should be included
    EXPECT_EQ(atoms[0]->getSerial(), 1);
}

TEST_F(PDBParserTest, ExcludeHydrogens) {
    PDBParser parser;
    parser.setIncludeHydrogens(false);
    
    std::string pdbContent = 
        "ATOM      1  CA  ALA A   1      10.000  20.000  30.000  1.00 50.00           C  \n"
        "ATOM      2  H   ALA A   1      11.000  21.000  31.000  1.00 50.00           H  \n"
        "ATOM      3  HA  ALA A   1      12.000  22.000  32.000  1.00 50.00           H  \n";
    
    auto atoms = parser.parseContent(pdbContent);
    
    EXPECT_EQ(atoms.size(), 1); // Only carbon atom should be included
    EXPECT_EQ(atoms[0]->getSerial(), 1);
    EXPECT_EQ(atoms[0]->getElement(), "C");
}

TEST_F(PDBParserTest, IncludeHydrogens) {
    PDBParser parser;
    parser.setIncludeHydrogens(true);
    
    std::string pdbContent = 
        "ATOM      1  CA  ALA A   1      10.000  20.000  30.000  1.00 50.00           C  \n"
        "ATOM      2  H   ALA A   1      11.000  21.000  31.000  1.00 50.00           H  \n";
    
    auto atoms = parser.parseContent(pdbContent);
    
    EXPECT_EQ(atoms.size(), 2); // Both atoms should be included
}

TEST_F(PDBParserTest, ParseStats) {
    PDBParser parser;
    parser.setIncludeHetAtoms(true);
    
    std::string pdbContent = 
        "HEADER    TITLE\n"
        "ATOM      1  CA  ALA A   1      10.000  20.000  30.000  1.00 50.00           C  \n"
        "HETATM    2  O   HOH S   1      15.000  25.000  35.000  1.00 60.00           O  \n"
        "REMARK    SOME REMARK\n"
        "END\n";
    
    auto atoms = parser.parseContent(pdbContent);
    
    const auto& stats = parser.getParseStats();
    EXPECT_EQ(stats.totalLines, 5);
    EXPECT_EQ(stats.atomRecords, 1);
    EXPECT_EQ(stats.hetatmRecords, 1);
    EXPECT_EQ(stats.skippedRecords, 3); // HEADER, REMARK, END
}

TEST_F(PDBParserTest, InvalidLines) {
    PDBParser parser;
    
    std::string pdbContent = 
        "ATOM      1  CA  ALA A   1      10.000  20.000  30.000  1.00 50.00           C  \n"
        "SHORT\n"  // Too short line
        "ATOM      X  CB  ALA A   1      XX.XXX  YY.YYY  ZZ.ZZZ  1.00 50.00           C  \n"; // Invalid numbers
    
    auto atoms = parser.parseContent(pdbContent);
    
    EXPECT_EQ(atoms.size(), 1); // Only the valid atom should be parsed
    const auto& stats = parser.getParseStats();
    EXPECT_GT(stats.errorRecords, 0);
}