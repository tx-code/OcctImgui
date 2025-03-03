/**
 * @file UnifiedModel.h
 * @brief Defines the UnifiedModel class which manages both CAD shapes and mesh data.
 * 
 * The UnifiedModel provides a unified interface for managing different types of
 * geometric data, including CAD shapes (TopoDS_Shape) and polygon meshes (Poly_Triangulation).
 */
#pragma once

#include "IModel.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include <TopoDS_Shape.hxx>
#include <Poly_Triangulation.hxx>
#include <Quantity_Color.hxx>
#include <gp_Trsf.hxx>

/**
 * @class UnifiedModel
 * @brief A model that manages both CAD shapes and mesh data with a unified interface.
 * 
 * This class implements the IModel interface and provides methods for adding, retrieving,
 * and manipulating both CAD shapes (TopoDS_Shape) and polygon meshes (Poly_Triangulation).
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
        MESH    ///< Polygon mesh (Poly_Triangulation)
    };
    
    /**
     * @brief Container for geometry data and associated properties
     */
    struct GeometryData {
        /** The geometry object, either a TopoDS_Shape or a Poly_Triangulation */
        std::variant<TopoDS_Shape, Handle(Poly_Triangulation)> geometry;
        
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
         * @param mesh The polygon mesh
         * @param color The color of the mesh (default: light gray)
         */
        GeometryData(const Handle(Poly_Triangulation)& mesh, const Quantity_Color& color = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB))
            : geometry(mesh), color(color), type(GeometryType::MESH) {}
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
    Handle(Poly_Triangulation) getMesh(const std::string& id) const;
    
    /**
     * @brief Adds a polygon mesh to the model
     * @param id The ID to assign to the mesh
     * @param mesh The polygon mesh to add
     */
    void addMesh(const std::string& id, const Handle(Poly_Triangulation)& mesh);
    
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