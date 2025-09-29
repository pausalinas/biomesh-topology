#include <gtest/gtest.h>
#include <biomesh/exporter/gid_exporter.h>
#include <biomesh/exporter/vtk_exporter.h>
#include <biomesh/mesh_generator/mesh_generator.h>
#include <fstream>
#include <filesystem>

using namespace biomesh;

class ExporterTest : public ::testing::Test {
protected:
    void SetUp() override {
        setupTestMesh();
        testDir_ = "/tmp/biomesh_test";
        std::filesystem::create_directories(testDir_);
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(testDir_);
    }
    
    void setupTestMesh() {
        testMesh_ = std::make_unique<Mesh>();
        
        // Add nodes
        testMesh_->addNode(MeshNode(1, 0.0, 0.0, 0.0));
        testMesh_->addNode(MeshNode(2, 1.0, 0.0, 0.0));
        testMesh_->addNode(MeshNode(3, 0.0, 1.0, 0.0));
        testMesh_->addNode(MeshNode(4, 0.0, 0.0, 1.0));
        testMesh_->addNode(MeshNode(5, 1.0, 1.0, 0.0));
        testMesh_->addNode(MeshNode(6, 0.0, 1.0, 1.0));
        
        // Add elements (tetrahedra)
        testMesh_->addElement(MeshElement(1, {1, 2, 3, 4}));
        testMesh_->addElement(MeshElement(2, {2, 3, 4, 5}));
    }
    
    bool fileExists(const std::string& filename) {
        return std::filesystem::exists(filename);
    }
    
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return "";
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
    
    std::unique_ptr<Mesh> testMesh_;
    std::string testDir_;
};

// GiD Exporter Tests
TEST_F(ExporterTest, GiDExporterConstructor) {
    GiDExporter exporter;
    EXPECT_TRUE(exporter.getLastError().empty());
}

TEST_F(ExporterTest, GiDExportValidMesh) {
    GiDExporter exporter;
    exporter.setPrecision(3);
    exporter.setIncludeStatistics(true);
    
    std::string outputPath = testDir_ + "/test_mesh";
    bool success = exporter.exportMesh(*testMesh_, outputPath);
    
    EXPECT_TRUE(success);
    EXPECT_TRUE(exporter.getLastError().empty());
    
    // Check that files were created
    EXPECT_TRUE(fileExists(outputPath + ".msh"));
    EXPECT_TRUE(fileExists(outputPath + ".crd"));
    EXPECT_TRUE(fileExists(outputPath + ".ele"));
}

TEST_F(ExporterTest, GiDExportInvalidMesh) {
    GiDExporter exporter;
    
    // Create invalid mesh (element references non-existent node)
    Mesh invalidMesh;
    invalidMesh.addNode(MeshNode(1, 0.0, 0.0, 0.0));
    invalidMesh.addElement(MeshElement(1, {1, 2})); // Node 2 doesn't exist
    
    std::string outputPath = testDir_ + "/invalid_mesh";
    bool success = exporter.exportMesh(invalidMesh, outputPath);
    
    EXPECT_FALSE(success);
    EXPECT_FALSE(exporter.getLastError().empty());
}

TEST_F(ExporterTest, GiDExportEmptyMesh) {
    GiDExporter exporter;
    Mesh emptyMesh;
    
    std::string outputPath = testDir_ + "/empty_mesh";
    bool success = exporter.exportMesh(emptyMesh, outputPath);
    
    EXPECT_FALSE(success);
    EXPECT_FALSE(exporter.getLastError().empty());
}

TEST_F(ExporterTest, GiDFileContent) {
    GiDExporter exporter;
    exporter.setPrecision(2);
    
    std::string outputPath = testDir_ + "/content_test";
    bool success = exporter.exportMesh(*testMesh_, outputPath);
    
    EXPECT_TRUE(success);
    
    // Check mesh file content
    std::string meshContent = readFile(outputPath + ".msh");
    EXPECT_FALSE(meshContent.empty());
    EXPECT_NE(meshContent.find("MESH"), std::string::npos);
    EXPECT_NE(meshContent.find("Coordinates"), std::string::npos);
    EXPECT_NE(meshContent.find("Elements"), std::string::npos);
    
    // Check coordinates file content
    std::string crdContent = readFile(outputPath + ".crd");
    EXPECT_FALSE(crdContent.empty());
    EXPECT_NE(crdContent.find("1 0.00 0.00 0.00"), std::string::npos); // First node
    
    // Check elements file content
    std::string eleContent = readFile(outputPath + ".ele");
    EXPECT_FALSE(eleContent.empty());
    EXPECT_NE(eleContent.find("1 1 2 3 4"), std::string::npos); // First element
}

// VTK Exporter Tests
TEST_F(ExporterTest, VTKExporterConstructor) {
    VTKExporter exporter;
    EXPECT_TRUE(exporter.getLastError().empty());
}

TEST_F(ExporterTest, VTKExportValidMesh) {
    VTKExporter exporter;
    exporter.setFormat(VTKExporter::Format::ASCII);
    exporter.setPrecision(3);
    exporter.setTitle("Test Mesh");
    
    std::string outputPath = testDir_ + "/test_mesh.vtk";
    bool success = exporter.exportMesh(*testMesh_, outputPath);
    
    EXPECT_TRUE(success);
    EXPECT_TRUE(exporter.getLastError().empty());
    EXPECT_TRUE(fileExists(outputPath));
}

TEST_F(ExporterTest, VTKExportInvalidMesh) {
    VTKExporter exporter;
    
    // Create invalid mesh
    Mesh invalidMesh;
    invalidMesh.addNode(MeshNode(1, 0.0, 0.0, 0.0));
    invalidMesh.addElement(MeshElement(1, {1, 2})); // Node 2 doesn't exist
    
    std::string outputPath = testDir_ + "/invalid_mesh.vtk";
    bool success = exporter.exportMesh(invalidMesh, outputPath);
    
    EXPECT_FALSE(success);
    EXPECT_FALSE(exporter.getLastError().empty());
}

TEST_F(ExporterTest, VTKExportEmptyMesh) {
    VTKExporter exporter;
    Mesh emptyMesh;
    
    std::string outputPath = testDir_ + "/empty_mesh.vtk";
    bool success = exporter.exportMesh(emptyMesh, outputPath);
    
    EXPECT_FALSE(success);
    EXPECT_FALSE(exporter.getLastError().empty());
}

TEST_F(ExporterTest, VTKFileContent) {
    VTKExporter exporter;
    exporter.setFormat(VTKExporter::Format::ASCII);
    exporter.setPrecision(2);
    exporter.setTitle("Test VTK Export");
    
    std::string outputPath = testDir_ + "/vtk_content_test.vtk";
    bool success = exporter.exportMesh(*testMesh_, outputPath);
    
    EXPECT_TRUE(success);
    
    std::string content = readFile(outputPath);
    EXPECT_FALSE(content.empty());
    
    // Check VTK header
    EXPECT_NE(content.find("# vtk DataFile Version 3.0"), std::string::npos);
    EXPECT_NE(content.find("Test VTK Export"), std::string::npos);
    EXPECT_NE(content.find("ASCII"), std::string::npos);
    EXPECT_NE(content.find("DATASET UNSTRUCTURED_GRID"), std::string::npos);
    
    // Check points section
    EXPECT_NE(content.find("POINTS"), std::string::npos);
    EXPECT_NE(content.find("0.00 0.00 0.00"), std::string::npos); // First point
    
    // Check cells section
    EXPECT_NE(content.find("CELLS"), std::string::npos);
    EXPECT_NE(content.find("CELL_TYPES"), std::string::npos);
}

TEST_F(ExporterTest, VTKScalarData) {
    VTKExporter exporter;
    
    // Add scalar data for nodes
    std::vector<double> nodeData = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    exporter.addNodeScalarData("Temperature", nodeData);
    
    // Add scalar data for elements
    std::vector<double> elementData = {10.0, 20.0};
    exporter.addElementScalarData("Pressure", elementData);
    
    std::string outputPath = testDir_ + "/vtk_scalar_test.vtk";
    bool success = exporter.exportMesh(*testMesh_, outputPath);
    
    EXPECT_TRUE(success);
    
    std::string content = readFile(outputPath);
    
    // Check point data
    EXPECT_NE(content.find("POINT_DATA"), std::string::npos);
    EXPECT_NE(content.find("SCALARS Temperature"), std::string::npos);
    
    // Check cell data
    EXPECT_NE(content.find("CELL_DATA"), std::string::npos);
    EXPECT_NE(content.find("SCALARS Pressure"), std::string::npos);
}

TEST_F(ExporterTest, VTKClearData) {
    VTKExporter exporter;
    
    std::vector<double> nodeData = {1.0, 2.0, 3.0};
    exporter.addNodeScalarData("Temperature", nodeData);
    
    exporter.clearData();
    
    std::string outputPath = testDir_ + "/vtk_clear_test.vtk";
    bool success = exporter.exportMesh(*testMesh_, outputPath);
    
    EXPECT_TRUE(success);
    
    std::string content = readFile(outputPath);
    
    // Should not contain point data after clearing
    EXPECT_EQ(content.find("POINT_DATA"), std::string::npos);
    EXPECT_EQ(content.find("Temperature"), std::string::npos);
}