#include <biomesh/pdb_parser/pdb_parser.h>
#include <biomesh/atom/atom.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace biomesh {

PDBParser::PDBParser() 
    : includeHetAtoms_(false), includeHydrogens_(true), atomCount_(0) {
    resetStats();
}

std::vector<std::unique_ptr<Atom>> PDBParser::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open PDB file: " + filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    return parseContent(content);
}

std::vector<std::unique_ptr<Atom>> PDBParser::parseContent(const std::string& content) {
    resetStats();
    std::vector<std::unique_ptr<Atom>> atoms;
    
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        stats_.totalLines++;
        auto atom = parseLine(line);
        if (atom) {
            atoms.push_back(std::move(atom));
        }
    }
    
    atomCount_ = atoms.size();
    return atoms;
}

std::unique_ptr<Atom> PDBParser::parseLine(const std::string& line) {
    if (line.length() < 6) {
        return nullptr;
    }
    
    std::string recordType = line.substr(0, 6);
    // Remove trailing spaces
    recordType.erase(std::find_if(recordType.rbegin(), recordType.rend(),
                     [](unsigned char ch) { return !std::isspace(ch); }).base(),
                     recordType.end());
    
    if (!shouldProcessRecord(recordType)) {
        stats_.skippedRecords++;
        return nullptr;
    }
    
    if (recordType == "ATOM") {
        stats_.atomRecords++;
    } else if (recordType == "HETATM") {
        stats_.hetatmRecords++;
    }
    
    return extractAtomData(line);
}

bool PDBParser::shouldProcessRecord(const std::string& recordType) const {
    if (recordType == "ATOM") {
        return true;
    }
    if (recordType == "HETATM") {
        return includeHetAtoms_;
    }
    return false;
}

std::unique_ptr<Atom> PDBParser::extractAtomData(const std::string& line) {
    try {
        if (line.length() < 78) {
            stats_.errorRecords++;
            return nullptr;
        }
        
        // Parse PDB format (fixed width columns)
        int serial = std::stoi(line.substr(6, 5));
        std::string atomName = line.substr(12, 4);
        std::string element = line.substr(76, 2);
        
        // Clean up strings
        atomName.erase(0, atomName.find_first_not_of(" "));
        atomName.erase(atomName.find_last_not_of(" ") + 1);
        element.erase(0, element.find_first_not_of(" "));
        element.erase(element.find_last_not_of(" ") + 1);
        
        // Check if we should skip hydrogen atoms
        if (!includeHydrogens_ && (element == "H" || atomName[0] == 'H')) {
            stats_.skippedRecords++;
            return nullptr;
        }
        
        // Parse coordinates
        double x = std::stod(line.substr(30, 8));
        double y = std::stod(line.substr(38, 8));
        double z = std::stod(line.substr(46, 8));
        
        // Create atom using builder for enrichment
        AtomBuilder builder;
        auto atom = builder.setSerial(serial)
                          .setName(atomName)
                          .setElement(element)
                          .setPosition(x, y, z)
                          .enrichWithStandardRadius()
                          .build();
        
        return atom;
        
    } catch (const std::exception&) {
        stats_.errorRecords++;
        return nullptr;
    }
}

void PDBParser::resetStats() {
    stats_.totalLines = 0;
    stats_.atomRecords = 0;
    stats_.hetatmRecords = 0;
    stats_.skippedRecords = 0;
    stats_.errorRecords = 0;
}

} // namespace biomesh