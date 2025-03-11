#pragma once

#include <Eigen/Dense>
#include <nlohmann/json.hpp>

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
    bool operator!=(const MeshData& other) const
    {
        return !operator==(other);
    }

    // JSON serialization
    void to_json(nlohmann::json& j) const;
    void from_json(const nlohmann::json& j);

private:
    Eigen::MatrixXd m_vertices;
    Eigen::MatrixXi m_faces;
    Eigen::MatrixXd m_normals;
};
}  // namespace Model

// JSON serialization for MeshData
namespace nlohmann
{
template<>
struct adl_serializer<Model::MeshData>
{
    static void to_json(nlohmann::json& j, const Model::MeshData& meshData)
    {
        meshData.to_json(j);
    }
    static void from_json(const nlohmann::json& j, Model::MeshData& meshData)
    {
        meshData.from_json(j);
    }
};
}  // namespace nlohmann