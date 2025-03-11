#include "MeshData.h"

#include <gsl/assert>
#include <igl/per_face_normals.h>

using namespace Model;

MeshData::MeshData()
    : m_vertices(Eigen::MatrixXd::Zero(0, 0))
    , m_faces(Eigen::MatrixXi::Zero(0, 0))
    , m_normals(Eigen::MatrixXd::Zero(0, 0))
{}

MeshData::MeshData(const Eigen::MatrixXd& vertices,
                   const Eigen::MatrixXi& faces,
                   const Eigen::MatrixXd& normals)
{
    setValue(vertices, faces, normals);
}

void MeshData::setValue(const Eigen::MatrixXd& vertices,
                        const Eigen::MatrixXi& faces,
                        const Eigen::MatrixXd& normals)
{

    m_vertices = vertices;
    m_faces = faces;
    if (m_normals.rows() == 0) {
        igl::per_face_normals(vertices, faces, m_normals);
    }
    else {
        m_normals = normals;
    }
}

void MeshData::getValue(Eigen::MatrixXd& vertices,
                        Eigen::MatrixXi& faces,
                        Eigen::MatrixXd& normals) const
{
    vertices = m_vertices;
    faces = m_faces;
    normals = m_normals;
}

const Eigen::MatrixXd& MeshData::getVertices() const
{
    return m_vertices;
}

const Eigen::MatrixXi& MeshData::getFaces() const
{
    return m_faces;
}

const Eigen::MatrixXd& MeshData::getNormals() const
{
    return m_normals;
}

bool MeshData::operator==(const MeshData& other) const
{
    if (this == &other) {
        return true;
    }
    return m_vertices == other.m_vertices && m_faces == other.m_faces
        && m_normals == other.m_normals;
}

void MeshData::to_json(nlohmann::json& j) const
{
    using namespace gsl;
    if (m_faces.rows() > 0) {
        Expects(m_faces.cols() == 3);
    }

    if (m_vertices.rows() > 0) {
        Expects(m_vertices.cols() == 3);
    }

    j["vertices"]["data"] =
        std::vector<double>(m_vertices.data(), m_vertices.data() + m_vertices.size());

    j["faces"]["data"] = std::vector<int>(m_faces.data(), m_faces.data() + m_faces.size());

    if (m_normals.rows() > 0) {
        Expects(m_normals.cols() == 3);
    }
    j["normals"]["data"] =
        std::vector<double>(m_normals.data(), m_normals.data() + m_normals.size());
}

void MeshData::from_json(const nlohmann::json& j)
{
    using namespace gsl;

    m_vertices.resize(j["vertices"]["data"].size() / 3, 3);
    m_faces.resize(j["faces"]["data"].size() / 3, 3);
    m_normals.resize(j["normals"]["data"].size() / 3, 3);

    Expects(j["vertices"]["data"].size() % 3 == 0);
    Expects(j["faces"]["data"].size() % 3 == 0);
    Expects(j["normals"]["data"].size() % 3 == 0);

    // Use Eigen::Map to fill the matrix
    Eigen::Map<const Eigen::MatrixXd> vMap(j["vertices"]["data"].get<std::vector<double>>().data(),
                                           j["vertices"]["data"].size() / 3,
                                           3);
    Eigen::Map<const Eigen::MatrixXi> fMap(j["faces"]["data"].get<std::vector<int>>().data(),
                                           j["faces"]["data"].size() / 3,
                                           3);
    Eigen::Map<const Eigen::MatrixXd> nMap(j["normals"]["data"].get<std::vector<double>>().data(),
                                           j["normals"]["data"].size() / 3,
                                           3);

    m_vertices = vMap;
    m_faces = fMap;
    m_normals = nMap;
}
