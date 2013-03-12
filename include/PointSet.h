#pragma once

class PointSet {
private:
    std::vector<Vertex>   m_vertices;

public:
    PointSet(void);
    PointSet(const std::string& filename);
    ~PointSet(void);

    void LoadFromFile(const std::string& filename);
    void WriteToFile(const std::string& filename);
};
