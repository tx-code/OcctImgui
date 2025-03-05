/**
 * @file UnifiedModel.h
 * @brief Defines the UnifiedModel class which manages both CAD shapes and mesh data.
 * 
 * The UnifiedModel provides a unified interface for managing different types of
 * geometric data, including CAD shapes (TopoDS_Shape) and polygon meshes (using libigl representation).
 */
#pragma once

#include "IModel.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include <TopoDS_Shape.hxx>
#include <Quantity_Color.hxx>
#include <gp_Trsf.hxx>
#include <Eigen/Dense>

/**
 * @class UnifiedModel
 * @brief A model that manages both CAD shapes and mesh data with a unified interface.
 * 
 * This class implements the IModel interface and provides methods for adding, retrieving,
 * and manipulating both CAD shapes (TopoDS_Shape) and polygon meshes (using libigl representation).
 * Each geometry is identified by a unique string ID and can have associated properties
 * such as color.
 */
class UnifiedModel : public IModel {
public:
    /**
     * @brief Enumeration of supported geometry types
     */
    enum class GeometryType {
        SHAPE,  ///< CAD model (TopoDS_Shape)
        MESH    ///< Polygon mesh (libigl representation)
    };
    
    /**
     * @brief Structure to represent a mesh using libigl's representation
     */
    struct MeshData {
        Eigen::MatrixXd vertices; ///< Vertex positions (n x 3 matrix)
        Eigen::MatrixXi faces;    ///< Face indices (m x 3 matrix for triangular mesh)
        Eigen::MatrixXd normals;  ///< Face normals (m x 3 matrix)
        
        /**
         * @brief Default constructor
         */
        MeshData() : vertices(0, 3), faces(0, 3), normals(0, 3) {}
        
        /**
         * @brief Constructor with vertices and faces
         */
        MeshData(const Eigen::MatrixXd& v, const Eigen::MatrixXi& f) 
            : vertices(v), faces(f), normals(Eigen::MatrixXd::Zero(f.rows(), 3)) {}
            
        /**
         * @brief Constructor with vertices, faces and normals
         */
        MeshData(const Eigen::MatrixXd& v, const Eigen::MatrixXi& f, const Eigen::MatrixXd& n)
            : vertices(v), faces(f), normals(n) {}
    };
    
    /**
     * @brief Container for geometry data and associated properties
     */
    struct GeometryData {
        /** The geometry object, either a TopoDS_Shape or a MeshData */
        std::variant<TopoDS_Shape, MeshData> geometry;
        
        /** The color of the geometry */
        Quantity_Color color;
        
        /** The type of the geometry */
        GeometryType type;
        
        /**
         * @brief Default constructor
         */
        GeometryData() : geometry(TopoDS_Shape()), color(Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB)), type(GeometryType::SHAPE) {}
        
        /**
         * @brief Constructor for CAD shapes
         * @param shape The CAD shape
         * @param color The color of the shape (default: light gray)
         */
        GeometryData(const TopoDS_Shape& shape, const Quantity_Color& color = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB))
            : geometry(shape), color(color), type(GeometryType::SHAPE) {}
        
        /**
         * @brief Constructor for polygon meshes
         * @param vertices The mesh vertices
         * @param faces The mesh faces
         * @param color The color of the mesh (default: light gray)
         */
        GeometryData(const Eigen::MatrixXd& vertices, const Eigen::MatrixXi& faces, 
                    const Quantity_Color& color = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB))
            : geometry(MeshData(vertices, faces)), color(color), type(GeometryType::MESH) {}
            
        /**
         * @brief Constructor for polygon meshes with pre-computed normals
         * @param vertices The mesh vertices
         * @param faces The mesh faces
         * @param normals The mesh face normals
         * @param color The color of the mesh (default: light gray)
         */
        GeometryData(const Eigen::MatrixXd& vertices, const Eigen::MatrixXi& faces, const Eigen::MatrixXd& normals,
                    const Quantity_Color& color = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB))
            : geometry(MeshData(vertices, faces, normals)), color(color), type(GeometryType::MESH) {}
    };
    
    /**
     * @brief Default constructor
     */
    UnifiedModel() = default;
    
    /**
     * @brief Virtual destructor
     */
    ~UnifiedModel() override = default;
    
    /**
     * @brief Gets the IDs of all entities in the model
     * @return Vector of entity IDs
     */
    std::vector<std::string> getAllEntityIds() const override;
    
    /**
     * @brief Removes an entity from the model
     * @param id The ID of the entity to remove
     */
    void removeEntity(const std::string& id) override;
    
    /**
     * @brief Gets a CAD shape by its ID
     * @param id The ID of the shape to retrieve
     * @return The CAD shape
     */
    TopoDS_Shape getShape(const std::string& id) const;
    
    /**
     * @brief Adds a CAD shape to the model
     * @param id The ID to assign to the shape
     * @param shape The CAD shape to add
     */
    void addShape(const std::string& id, const TopoDS_Shape& shape);
    
    /**
     * @brief Gets a polygon mesh by its ID
     * @param id The ID of the mesh to retrieve
     * @return The polygon mesh
     */
    const MeshData* getMesh(const std::string& id) const;
    
    /**
     * @brief Adds a polygon mesh to the model
     * @param id The ID to assign to the mesh
     * @param vertices The mesh vertices
     * @param faces The mesh faces
     */
    void addMesh(const std::string& id, const Eigen::MatrixXd& vertices, const Eigen::MatrixXi& faces);
    
    /**
     * @brief Adds a polygon mesh with pre-computed normals to the model
     * @param id The ID to assign to the mesh
     * @param vertices The mesh vertices
     * @param faces The mesh faces
     * @param normals The mesh face normals
     */
    void addMesh(const std::string& id, const Eigen::MatrixXd& vertices, const Eigen::MatrixXi& faces, const Eigen::MatrixXd& normals);
    
    /**
     * @brief Removes a geometry from the model
     * @param id The ID of the geometry to remove
     */
    void removeGeometry(const std::string& id);
    
    /**
     * @brief Gets the type of a geometry
     * @param id The ID of the geometry
     * @return The type of the geometry
     */
    GeometryType getGeometryType(const std::string& id) const;
    
    /**
     * @brief Gets the geometry data for an entity
     * @param id The ID of the entity
     * @return Pointer to the geometry data, or nullptr if not found
     */
    const GeometryData* getGeometryData(const std::string& id) const;
    
    /**
     * @brief Gets the IDs of all geometries of a specific type
     * @param type The type of geometries to retrieve
     * @return Vector of geometry IDs
     */
    std::vector<std::string> getGeometryIdsByType(GeometryType type) const;
    
    /**
     * @brief Sets the color of a geometry
     * @param id The ID of the geometry
     * @param color The color to set
     */
    void setColor(const std::string& id, const Quantity_Color& color);
    
    /**
     * @brief Gets the color of a geometry
     * @param id The ID of the geometry
     * @return The color of the geometry
     */
    Quantity_Color getColor(const std::string& id) const;
    
    /**
     * @brief Applies a transformation to a geometry
     * @param id The ID of the geometry to transform
     * @param transformation The transformation to apply
     */
    void transform(const std::string& id, const gp_Trsf& transformation);
    
private:
    /** Map of geometry IDs to geometry data */
    std::map<std::string, GeometryData> myGeometries;
}; 