#pragma once

#include "IGeometryModel.h"
#include "Model.h"
#include "MeshData.h"

#include "core/Property.h"

#include <Eigen/Dense>
#include <memory>
#include <string>
#include <TopoDS_Shape.hxx>

namespace Model
{

/**
 * @class GeometryModel
 * @brief Implementation of the IGeometryModel interface
 *
 * This class represents a geometry model with both B-Rep and mesh representations.
 * It supports various file formats, Boolean operations, and geometric queries.
 */
class GeometryModel: public Model, public IGeometryModel
{
public:
    using MatrixXd = Eigen::MatrixXd;
    using MatrixXi = Eigen::MatrixXi;
    using Vector3d = Eigen::Vector3d;
    using Point3d = Vector3d;

    /**
     * @brief Constructor
     * @param name Name of the geometry model
     */
    explicit GeometryModel(const std::string& name)
        : Model(name, ModelTypeEnum::Geometry)
    {
        // Register properties with SerializableObject
        registerProperty("shape", m_shape);
        registerProperty("mesh", m_mesh);
        
        // Initialize property change handlers
        setupPropertyHandlers();
    }

    /**
     * @brief Destructor
     */
    ~GeometryModel() override;

    // IModel implementation
    std::shared_ptr<IModel> clone() const override;

    // IGeometryModel implementation
    bool loadFromFile(const std::string& filename) override;
    bool saveToFile(const std::string& filename) override;
    void transform(const Matrix4d& matrix) override;
    bool getBoundingBox(Vector3d& min, Vector3d& max) const override;
    double getVolume() const override;
    double getSurfaceArea() const override;

    // Specific GeometryModel methods
    /**
     * @brief Get the underlying OpenCASCADE shape
     * @return Reference to the shape
     */
    const TopoDS_Shape& getShape() const
    {
        return m_shape.get();
    }

    /**
     * @brief Set the underlying OpenCASCADE shape
     * @param shape The new shape
     */
    void setShape(const TopoDS_Shape& shape)
    {
        m_shape.set(shape);
    }

    /**
     * @brief Get the mesh data
     * @return Reference to the mesh data
     */
    const MeshData& getMesh() const
    {
        return m_mesh.get();
    }

    /**
     * @brief Set the mesh data
     * @param mesh The mesh data
     */
    void setMesh(const MeshData& mesh)
    {
        m_mesh.set(mesh);
    }

    /**
     * @brief Set the mesh representation
     * @param vertices Matrix of vertices (Nx3)
     * @param faces Matrix of faces (Mx3 for triangles)
     * @param normals Matrix of normals (optional)
     */
    void setMesh(const MatrixXd& vertices, const MatrixXi& faces, const MatrixXd& normals = MatrixXd())
    {
        MeshData mesh(vertices, faces, normals);
        m_mesh.set(mesh);
    }

    /**
     * @brief Get the vertex matrix for mesh representation
     * @return Matrix of vertices (Nx3)
     */
    const MatrixXd& getVertices() const
    {
        return m_mesh.get().getVertices();
    }

    /**
     * @brief Get the face matrix for mesh representation
     * @return Matrix of faces (Mx3 for triangles)
     */
    const MatrixXi& getFaces() const
    {
        return m_mesh.get().getFaces();
    }

    /**
     * @brief Get the normal matrix for mesh representation
     * @return Matrix of normals
     */
    const MatrixXd& getNormals() const
    {
        return m_mesh.get().getNormals();
    }

    /**
     * @brief Check if the model has a mesh representation
     * @return True if mesh is available, false otherwise
     */
    bool hasMesh() const
    {
        return m_mesh.get().getVertices().rows() > 0 && m_mesh.get().getFaces().rows() > 0;
    }

    /**
     * @brief Get signal for shape changes
     * @return Reference to the shape changed signal
     */
    Core::Signal<const TopoDS_Shape&, const TopoDS_Shape&>& shapeChanged()
    {
        return m_shape.valueChanged;
    }

    /**
     * @brief Get signal for mesh changes
     * @return Reference to the mesh changed signal
     */
    Core::Signal<const MeshData&, const MeshData&>& meshChanged()
    {
        return m_mesh.valueChanged;
    }

    /**
     * @brief Perform a Boolean union operation
     * @param other The other geometry model
     * @return Result of the union operation
     */
    std::shared_ptr<GeometryModel> performUnion(const GeometryModel& other) const;

    /**
     * @brief Perform a Boolean difference operation
     * @param other The other geometry model
     * @return Result of the difference operation
     */
    std::shared_ptr<GeometryModel> performDifference(const GeometryModel& other) const;

    /**
     * @brief Perform a Boolean intersection operation
     * @param other The other geometry model
     * @return Result of the intersection operation
     */
    std::shared_ptr<GeometryModel> performIntersection(const GeometryModel& other) const;

private:
    // Setup property change handlers
    void setupPropertyHandlers();

    // File format handlers
    bool loadSTEP(const std::string& filename);
    bool loadSTL(const std::string& filename);
    bool loadOBJ(const std::string& filename);
    bool loadBREP(const std::string& filename);
    bool loadIGES(const std::string& filename);
    bool saveSTEP(const std::string& filename) const;
    bool saveSTL(const std::string& filename) const;
    bool saveOBJ(const std::string& filename) const;
    bool saveBREP(const std::string& filename) const;

    // Update mesh from shape
    void updateMeshFromShape();

    // Properties
    Core::Property<TopoDS_Shape> m_shape;
    Core::Property<MeshData> m_mesh;

    // Connection storage
};

}  // namespace Model