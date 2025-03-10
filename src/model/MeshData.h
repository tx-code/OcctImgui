#pragma once

#include <Eigen/Dense>


namespace Model
{

/**
 * @class MeshData
 * @brief Represents a mesh data structure
 */
class MeshData
{
public:
    MeshData();
    ~MeshData() = default;

    MeshData(const Eigen::MatrixXd& vertices,
             const Eigen::MatrixXi& faces,
             const Eigen::MatrixXd& normals = Eigen::MatrixXd::Zero());

    void setValue(const Eigen::MatrixXd& vertices,
                  const Eigen::MatrixXi& faces,
                  const Eigen::MatrixXd& normals = Eigen::MatrixXd::Zero());

    void
    getValue(Eigen::MatrixXd& vertices, Eigen::MatrixXi& faces, Eigen::MatrixXd& normals) const;

    const Eigen::MatrixXd& getVertices() const;
    const Eigen::MatrixXi& getFaces() const;
    const Eigen::MatrixXd& getNormals() const;

    bool operator==(const MeshData& other) const; 
    bool operator!=(const MeshData& other) const { return !operator==(other); }
private:
    Eigen::MatrixXd m_vertices;
    Eigen::MatrixXi m_faces;
    Eigen::MatrixXd m_normals;
};
}  // namespace Model