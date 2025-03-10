#include "MeshData.h"

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

bool MeshData::operator==(const MeshData& other) const {
    if(this == &other) {
        return true;
    }
    return m_vertices == other.m_vertices &&
           m_faces == other.m_faces &&
           m_normals == other.m_normals;
}
