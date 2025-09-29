#pragma once

#include <string>
#include <vector>
#include <memory>
#include <biomesh/atom/atom.h>

namespace biomesh {

/**
 * @brief Parser for PDB (Protein Data Bank) files
 */
class PDBParser {
public:
    /**
     * @brief Default constructor
     */
    PDBParser();

    /**
     * @brief Destructor
     */
    ~PDBParser() = default;

    /**
     * @brief Parse PDB file and extract atom data
     * @param filename Path to the PDB file
     * @return Vector of parsed atoms
     * @throws std::runtime_error if file cannot be opened or parsed
     */
    std::vector<std::unique_ptr<Atom>> parseFile(const std::string& filename);

    /**
     * @brief Parse PDB content from string
     * @param content PDB file content as string
     * @return Vector of parsed atoms
     */
    std::vector<std::unique_ptr<Atom>> parseContent(const std::string& content);

    /**
     * @brief Set whether to include HETATM records
     * @param includeHetAtoms If true, include HETATM records
     */
    void setIncludeHetAtoms(bool includeHetAtoms) { includeHetAtoms_ = includeHetAtoms; }

    /**
     * @brief Set whether to include hydrogen atoms
     * @param includeHydrogens If true, include hydrogen atoms
     */
    void setIncludeHydrogens(bool includeHydrogens) { includeHydrogens_ = includeHydrogens; }

    /**
     * @brief Get number of atoms parsed in last operation
     */
    size_t getAtomCount() const { return atomCount_; }

    /**
     * @brief Get parsing statistics
     */
    struct ParseStats {
        size_t totalLines = 0;
        size_t atomRecords = 0;
        size_t hetatmRecords = 0;
        size_t skippedRecords = 0;
        size_t errorRecords = 0;
    };

    const ParseStats& getParseStats() const { return stats_; }

private:
    bool includeHetAtoms_;
    bool includeHydrogens_;
    size_t atomCount_;
    ParseStats stats_;

    /**
     * @brief Parse a single PDB line
     * @param line PDB line to parse
     * @return Parsed atom or nullptr if line should be skipped
     */
    std::unique_ptr<Atom> parseLine(const std::string& line);

    /**
     * @brief Check if record type should be processed
     * @param recordType Record type (ATOM or HETATM)
     * @return True if record should be processed
     */
    bool shouldProcessRecord(const std::string& recordType) const;

    /**
     * @brief Extract atom information from PDB line
     * @param line PDB line
     * @return Atom data or nullptr if parsing failed
     */
    std::unique_ptr<Atom> extractAtomData(const std::string& line);

    /**
     * @brief Reset parsing statistics
     */
    void resetStats();
};

} // namespace biomesh